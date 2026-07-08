//---------------------------------------------------------------------------
//! @file   Scene.h
//! @brief  シーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Object.h>
#include <System/Component/ComponentTransform.h>
#include <System/Component/ComponentCamera.h>
#include <System/Utils/HelperLib.h>
#include <System/Cereal.h>
#include <System/TypeInfo.h>
#include <WinMain.h>

#include <vector>
#include <memory>
#include <unordered_map>
#include <sigslot/include/sigslot/signal.hpp>

#include <iostream>
#include <sstream>
#include <string>

// ※シーン項目に追加する場合は更新が必要となります
constexpr int SCENE_VERSION = 1;

class Scene
{
public:
    class Base;
    using BasePtr    = std::shared_ptr<Base>;
    using BasePtrVec = std::vector<BasePtr>;
    using BasePtrMap = std::unordered_map<std::string, BasePtr>;

#if 0
	//---------------------------------------------------------------------------
	//! シーンステータス
	//---------------------------------------------------------------------------
	enum struct StatusBit : u64
	{
		Initialized,			//!< 初期化済み
		Serialized,				//!< シリアライズ済み.
		AliveInAnotherScene,	//!< 別シーン移行でも終了しない
		NoSerialize,			//!< シリアライズしない.
	};
#endif
    //---------------------------------------------------------------------------
    // シグナル
    //---------------------------------------------------------------------------
    using SignalsDefault = sigslot::signal<>;

    //---------------------------------------------------------------------------
    //! シーンベース
    //---------------------------------------------------------------------------
    class Base
    {
        friend class Scene;

    public:
        BP_CLASS_DECL(Scene::Base, u8"シーン基底");

        Base();
        virtual ~Base();

        const std::string_view GetName() const { return typeInfo()->className(); }

        const std::string GetGUIName() const
        {
            std::string name = "Scene : " + std::string(current_scene_->typeInfo()->className());
            return name;
        }

        //! @brief 現在のシーンの経過時間を取得する。
        //! @return シーン開始からの経過時間（秒）を float 型で返す。
        float GetSceneTime();

        //----------------------------------------------------------------------
        //! @name ユーザー処理
        //----------------------------------------------------------------------
        //@{

        virtual bool Init() { return true; }    //!< 初期化

        virtual void Update() {}    //!< 更新
        virtual void Draw() {}      //!< 描画
        virtual void Exit() {}      //!< 終了
        virtual void GUI() {}       //!< GUI表示

        virtual void PreUpdate() {}      //!< 更新前処理
        virtual void LateUpdate() {}     //!< 通常更新の後更新処理
        virtual void PrePhysics() {}     //!< 物理前(アクション後)処理
        virtual void PostPhysics() {}    //!< 物理後処理
        virtual void PostUpdate() {}     //!< 更新後(物理後)処理

        virtual void PreDraw() {}     //!< 描画前処理
        virtual void LateDraw() {}    //!< 遅い描画の処理
        virtual void PostDraw() {}    //!< 描画後の処理

        virtual void InitSerialize() {}

        //@}

        //----------------------------------------------------------------------
        //! @name オブジェクト登録/削除 処理
        //----------------------------------------------------------------------
        //@{

        //! @brief オブジェクト仮登録
        //! @param obj オブジェクト
        //! @param update 更新プライオリティ
        //! @param draw 描画プライオリティ
        void PreRegister(ObjectPtr obj, ProcPriority update = ProcPriority::NORMAL, ProcPriority draw = ProcPriority::NORMAL);

        template <class T>
        void PreRegister(T* obj, ProcPriority update, ProcPriority draw);

        void Register(ObjectPtr obj, ProcPriority update = ProcPriority::NORMAL, ProcPriority draw = ProcPriority::NORMAL);
        void RegisterForLoad(ObjectPtr obj);
        void Unregister(ObjectPtr obj);
        void UnregisterAll();

        //@}
        //----------------------------------------------------------------------
        //! @name 処理優先変更 処理
        //----------------------------------------------------------------------
        //@{

        //! 優先を設定変更します
        void SetPriority(ObjectPtr obj, ProcTiming timing, ProcPriority priority);

        //! 優先を設定変更します
        void SetPriority(ComponentPtr component, ProcTiming timing, ProcPriority priority);

        //@}
        //----------------------------------------------------------------------
        //! @name シーン取得or作成/ 解放 処理
        //----------------------------------------------------------------------
        //@{

        //! シーンを作成または取得します
        //! @param T 取得するシーンクラス
        //! @return シーン
        template <class T>
        static std::shared_ptr<T> GetScene();

        //! シーンを消去します
        //! @param name シーン名
        template <class T>
        static void ReleaseScene();

        //! 確保しているものに同じシーンタイプがいないかチェックする
        static bool IsSceneExist(const BasePtr& scene);

        //! シーン確保 (GetSceneを使ってない場合の対処)
        static void SetScene(const BasePtr& scene);

        static void ReleaseScene(const BasePtr& scene);
        //@}
        //----------------------------------------------------------------------
        //! @name オブジェクト作成/取得 処理
        //----------------------------------------------------------------------
        //@{

        //! 存在するオブジェクトの取得
        //! @tparam [in] class T 取得するオブジェクトタイプ
        template <class T>
        std::shared_ptr<T> GetObjectPtr(const std::string_view name = "");

        //! 存在する複数オブジェクトの取得
        //! @tparam [in] class T 取得するオブジェクトタイプ
        template <class T>
        std::vector<std::shared_ptr<T>> GetObjectsPtr(const std::string_view name = "");

        template <class T>
        std::shared_ptr<T> GetObjectPtrWithCreate(const std::string_view name = "");

        //! 複数オブジェクト配列の取得
        const ObjectPtrVec GetObjectPtrVec() { return objects_; }

        //@}
        //----------------------------------------------------------------------
        //! @name シーン状態にかかわる 処理
        //----------------------------------------------------------------------
        //@{
        enum struct StatusBit : u64
        {
            Initialized,            //!< 初期化済み
            Serialized,             //!< シリアライズ済み.
            AliveInAnotherScene,    //!< 別シーン移行でも終了しない
            NoSerialize,            //!< シリアライズしない.
            IsLoading,              //!< ローディングシリアライズ必要
        };

        void SetStatus(StatusBit b, bool on) { on ? status_.on(b) : status_.off(b); }

        bool GetStatus(StatusBit b) { return status_.is(b); }

        //! 別のシーンで生存するように設定する
        void AliveInAnotherScene(bool alive = true) { SetStatus(StatusBit::AliveInAnotherScene, alive); }

        //! 別のシーンで生存するように設定されているか?
        bool IsAliveInAnotherScene() { return GetStatus(StatusBit::AliveInAnotherScene); }

        //@}
        //----------------------------------------------------------------------
        //! @name シグナル
        //----------------------------------------------------------------------
        //@{

        SignalsDefault& GetSignals(ProcTiming timing) { return signals_[static_cast<int>(timing)]; }

        //@}
        //--------------------------------------------------------------------
        //! @name ユーザーシグナル
        //--------------------------------------------------------------------
        //! Shadowタイミングシグナル取得
        SignalsDefault& GetSignalsShadow() { return signals_[static_cast<int>(ProcTiming::Shadow)]; }

        //! GBufferタイミングシグナル取得
        SignalsDefault& GetSignalsGBuffer() { return signals_[static_cast<int>(ProcTiming::GBuffer)]; }

        //! Gbufferタイミングシグナル取得
        [[deprecated("Gbufferは旧呼び出しのため、GetSignalsGBuffer()を使用してください")]]
        SignalsDefault& GetSignalsGbuffer()
        {
            return signals_[static_cast<int>(ProcTiming::GBuffer)];
        }

        //! Lightタイミングシグナル取得
        SignalsDefault& GetSignalsLight() { return signals_[static_cast<int>(ProcTiming::Light)]; }

        //! Lightタイミングシグナル取得
        SignalsDefault& GetSignalsHDR() { return signals_[static_cast<int>(ProcTiming::HDR)]; }

        //----------------------------------------------------------------------
        //! @name Cereal セーブロード
        //----------------------------------------------------------------------
        //@{

        virtual bool Save(std::string_view filename = "");

        virtual bool Load(std::string_view filename = "");
        //@}

    private:
        struct ComponentSignalGateState
        {
            bool update_blocked_ = true;
            bool draw_blocked_   = true;
        };

        struct ObjectSignalGateState
        {
            bool                                                       initialized_           = false;
            bool                                                       object_update_blocked_ = true;
            bool                                                       object_draw_blocked_   = true;
            std::unordered_map<::Component*, ComponentSignalGateState> component_states_;
        };

        //! オブジェクトの指定処理を指定優先で処理するように登録する
        void setProc(ObjectPtr obj, SlotProc& slot);

        //! オブジェクトの指定処理を削除する
        void resetProc(ObjectPtr obj, SlotProc& slot);

        //! @brief コンポーネントの指定処理を指定優先で処理するように登録する
        void setProc(ComponentPtr component, SlotProc& slot);

        //! @brief コンポーネントの指定処理を削除する
        void resetProc(ComponentPtr component, SlotProc& slot);

        void applyObjectSignalState(ObjectPtr obj, bool is_pause, bool force_sync);
        void cleanupSignalStateCache();

        ObjectPtrVec                                         pre_objects_;    //!< シーンに存在させるオブジェクト(仮登録)
        ObjectPtrVec                                         objects_;        //!< シーンに存在するオブジェクト
        Status<StatusBit>                                    status_;         //!< 状態
        std::unordered_map<::Object*, ObjectSignalGateState> signal_gate_states_;

        int version_ = SCENE_VERSION;

        // プロセスタイミングによるシグナル (実行処理)
        std::array<SignalsDefault, static_cast<int>(ProcTiming::NUM)> signals_;

        int loading_wait_ = 5;
    };

    //----------------------------------------------------------------
    //! @name シーン操作関係
    //----------------------------------------------------------------
    //@{

    //! シーンを作成または取得します
    //! @param [in] class T 取得するシーンクラス
    //! @param [in] name シーン名
    //! @return シーン
    template <class T>
    static std::shared_ptr<T> GetScene()
    {
        // 存在する場合はシーンを返す
        // あまりよろしくはないがシーンを作成し名前を取得する
        T           forName;
        std::string name = forName.typeInfo()->className();
        if(scenes_.count(name) > 0) {
            return std::dynamic_pointer_cast<T>(scenes_[name]);
        }

        // 存在しない場合は作成します
        auto scene    = std::make_shared<T>();
        scenes_[name] = scene;
        return scene;
    }

    //! シーンを消去します
    //! @param [in] name シーン名
    template <class T>
    static void ReleaseScene()
    {
        Base::ReleaseScene<T>();
    }

    static void ReleaseScene(const ::Scene::BasePtr& scene) { ::Scene::Base::ReleaseScene(scene); }

    //! シーン切り替え
    //! @param scene 次に再生するシーン
    //! @details 内部的には、SetNextScene() -> ChangeNextScene() が呼ばれています
    static void Change(BasePtr scene);

    //! 次のシーンをセットする
    static void SetNextScene(BasePtr scene);

    static void CheckLeak();

    //! 次のシーンに切り替える
    static void ChangeNextScene();

    //  現在アクティブなシーンを取得します
    static Scene::Base* GetCurrentScene();

    template <typename T>
    static T* GetCurrentScene()
    {
        return dynamic_cast<T*>(GetCurrentScene());
    }

    //@}
    //----------------------------------------------------------------
    //! @name オブジェクト操作 関係
    //----------------------------------------------------------------
    //@{

    struct Object
    {
        template <class T>
        static std::shared_ptr<T>    //
        Create(const std::string_view name         = u8"object",
               bool                   no_transform = false,
               ProcPriority           update       = ProcPriority::NORMAL,
               ProcPriority           draw         = ProcPriority::NORMAL)
        {
            if(current_scene_) {
                auto tmp = std::make_shared<T>();
                current_scene_->PreRegister(tmp, update, draw);

                // デフォルトではComponentTransformは最初から用意する
                if(!no_transform)
                    tmp->AddComponent<ComponentTransform>();

                tmp->SetName(name.data());
                // Object::Init()は作成したときに行うように変更
                bool ret = tmp->Init();
                if(ret) {
                    assert("継承先のInit()にて__super::Init()を入れてください." && tmp->GetStatus(::Object::StatusBit::Initialized));
                }
                return tmp;
            }
            assert(!"Scene上で作成しなければなりません.");
            return std::shared_ptr<T>();
        }

        //! オブジェクトの作成
        //! Objectをシーン上に発生させる
        //! @param no_transform ComponentTransformを作らない (true = 作らない)
        //! @param update 処理優先
        //! @param draw 描画優先
        template <class T>
        static std::shared_ptr<T> CreateDelayInitialize(const std::string_view name         = u8"object",
                                                        bool                   no_transform = false,
                                                        ProcPriority           update       = ProcPriority::NORMAL,
                                                        ProcPriority           draw         = ProcPriority::NORMAL)
        {
            if(current_scene_) {
                auto tmp = std::make_shared<T>();
                current_scene_->PreRegister(tmp, update, draw);

                tmp->SetName(name.data());

                // デフォルトではComponentTransformは最初から用意する
                if(!no_transform)
                    tmp->AddComponent<ComponentTransform>();

                return tmp;
            }
            assert(!"Scene上で作成しなければなりません.");
            return std::shared_ptr<T>();
        }

        template <class T>
        static void Release()
        {
            auto obj = current_scene_->GetObjectPtr<T>();
            current_scene_->Unregister(obj);
        }

        static void Release(std::string_view name = "");

        static void Release(ObjectPtr obj);

        template <class T>
        static void Release(std::vector<std::shared_ptr<T>> objs)
        {
            for(auto& obj : objs) Scene::Object::Release(obj);
        }

        //! @brief オブジェクトサーチ&取得
        //! @tparam T 取得したいオブジェクトタイプ
        //! @param name 取得したいオブジェクトの名前
        //! @return オブジェクト
        template <class T>
        static std::shared_ptr<T> Get(std::string_view name = "")
        {
            //! @todo nullptrの時、存在しないので、ここでLOGでエラーを出しておく
            return current_scene_->GetObjectPtr<T>(name);
        }

        //! @brief オブジェクトサーチ&取得
        //! @tparam T 取得したいオブジェクトタイプ
        //! @details 同じタイプのオブジェクトが複数あると先に見つかったものを返します
        //! @return オブジェクト
        template <class T>
        static std::vector<std::shared_ptr<T>> GetArray()
        {
            return current_scene_->GetObjectsPtr<T>();
        }

        template <class T>
        static std::shared_ptr<T> GetOrCreate(std::string_view name = "")
        {
            return current_scene_->GetObjectPtrWithCreate<T>(name);
        }
    };

    //! オブジェクトの作成
    //! Objectをシーン上に発生させる
    //! @param no_transform ComponentTransformを作らない (true = 作らない)
    //! @param update 処理優先
    //! @param draw 描画優先
    template <class T>
    [[deprecated("削除予定関数です　Scene::Object::Create() を利用してください")]]
    static std::shared_ptr<T>    //
    CreateObjectPtr(const std::string_view name         = u8"object",
                    bool                   no_transform = false,
                    ProcPriority           update       = ProcPriority::NORMAL,
                    ProcPriority           draw         = ProcPriority::NORMAL)
    {
        return Object::Create<T>(name, no_transform, update, draw);
    }

    //! オブジェクトの作成
    //! Objectをシーン上に発生させる
    //! @param no_transform ComponentTransformを作らない (true = 作らない)
    //! @param update 処理優先
    //! @param draw 描画優先
    template <class T>
    [[deprecated("削除予定関数です　Scene::Object::CreateDelayInitialize() を利用してください")]]
    static std::shared_ptr<T> CreateObjectDelayInitialize(const std::string_view name         = u8"object",
                                                          bool                   no_transform = false,
                                                          ProcPriority           update       = ProcPriority::NORMAL,
                                                          ProcPriority           draw         = ProcPriority::NORMAL)
    {
        return Object::CreateDelayInitialize<T>(name, no_transform, update, draw);
    }

    template <class T>
    [[deprecated("削除予定関数です　Scene::Object::Release() を利用してください")]]
    static void ReleaseObject()
    {
        auto obj = current_scene_->GetObjectPtr<T>();
        current_scene_->Unregister(obj);
    }

    [[deprecated("削除予定関数です　Scene::Object::Release() を利用してください")]]
    static void ReleaseObject(std::string_view name = "");

    [[deprecated("削除予定関数です　Scene::Object::Release() を利用してください")]]
    static void ReleaseObject(ObjectPtr obj);

    //@}
    //----------------------------------------------------------------
    //! @name シーン処理 関係
    //----------------------------------------------------------------
    //@{

    //! 初期化処理
    static void Init();

    //! @brief シーン更新前処理
    //! @detail 基本的にはUpdate集団より先にで行いたいものをこの層で処理します
    static void PreUpdate();

    //! シーン更新
    //! @param [in] delta 更新インターバル(秒指定)
    static void Update();

    //! @brief Physics前処理
    //! @detail OwnerObject移動によるコリジョンコンポーネント前処理
    static void PrePhysics();

    //! @brief Physics後処理
    //! @detail OwnerObject移動によるコリジョンコンポーネント移動後処理
    static void PostPhysics();

    //! @brief シーン更新後の処理
    //! @detail Physicsおよびすべての当たり判定後に処理します
    static void PostUpdate();

    //! シーン描画
    static void Draw();

    //! シーン終了
    static void Exit();

    //! @brief シーンGUI
    //! @detail ※この処理はUpdate後に行う必要があります
    static void GUI();

    //@}
    //----------------------------------------------------------------
    //! @name シーン状態 関係
    //----------------------------------------------------------------
    //@{

    //! ポーズ中かをチェック
    //! @retval true ポーズ中
    //! @retval false ポーズしていない
    static bool IsPause();

    //! シーン内時間の取得
    //! @return シーンが始まってからの時間
    static float GetTime();

    //! 存在するシーン数
    static size_t GetSceneCount() { return scenes_.size(); }

    //@}
    //----------------------------------------------------------------
    //! @name シーン内オブジェクト取得 関係
    //----------------------------------------------------------------
    //@{

    //! @brief オブジェクトサーチ&取得
    //! @tparam T 取得したいオブジェクトタイプ
    //! @param name 取得したいオブジェクトの名前
    //! @return オブジェクト
    template <class T>
    static std::shared_ptr<T> GetObjectPtr(std::string_view name = "")
    {
        //! @todo nullptrの時、存在しないので、ここでLOGでエラーを出しておく
        return current_scene_->GetObjectPtr<T>(name);
    }

    //! @brief オブジェクトサーチ&取得
    //! @tparam T 取得したいオブジェクトタイプ
    //! @details 同じタイプのオブジェクトが複数あると先に見つかったものを返します
    //! @return オブジェクト
    template <class T>
    static std::vector<std::shared_ptr<T>> GetObjectsPtr()
    {
        return current_scene_->GetObjectsPtr<T>();
    }

    template <class T>
    static std::shared_ptr<T> GetObjectPtrWithCreate(std::string_view name = "")
    {
        return current_scene_->GetObjectPtrWithCreate<T>(name);
    }

    //! @brief ComponentCollisionの当たり判定を行う
    static void CheckComponentCollisions();

    //! セレクトしているオブジェクトかをチェックする
    static bool SelectObjectWindow(const ObjectPtr& object);

    static ObjectPtr SelectObjectPtr();

    //! @brief
    //! GUIオブジェクトのEditor設定であるためObjectでなくこちらで設定する
    static void SetGUIObjectDetailSize();
    //@}
    //----------------------------------------------------------------------
    //! @name Proc
    //----------------------------------------------------------------------
    //@{

public:
    static SignalsDefault& GetSignals(ProcTiming timing) { return current_scene_->GetSignals(timing); }

    //@}

    static const int SET_GUI_ATTACH_LEFT   = -10000;
    static const int SET_GUI_ATTACH_RIGHT  = +10000;
    static const int SET_GUI_ATTACH_TOP    = -10000;
    static const int SET_GUI_ATTACH_BOTTOM = +10000;
    static void      SetGUIWindow(int posx, int posy, int width, int height);

    //! @brief オブジェクト選択
    //! @param x mouseポインタX
    //! @param y mouseポインタY
    //! @return ObjectPtr (ない場合はnullptr)
    static ObjectPtr PickObject(int x, int y);

    //! @brief カレントカメラの取得
    //! @return カレントカメラ
    static ComponentCameraWeakPtr GetCurrentCamera();

    //! @brief オブジェクトの名前でのカレントカメラの設定
    //! @brief name 名前
    //! @return カレントカメラ
    static ComponentCameraWeakPtr SetCurrentCamera(std::string_view name);

    enum struct EditorStatusBit : u64
    {
        EditorPlacement,           //!< エディタ配置
        EditorPlacement_Always,    //!< エディタ配置(常に引っ付く)
    };

    static void SetEditorStatus(EditorStatusBit b, bool on) { editor_status_.set(b, on); }

    static bool GetEditorStatus(EditorStatusBit b) { return editor_status_.is(b); }

    static Status<EditorStatusBit>& SceneStatus() { return editor_status_; }

    //----------------------------------------------------------------------
    //! @name Cereal セーブロード
    //----------------------------------------------------------------------
    //@{

    static void SaveEditor();

    static void LoadEditor();

    static bool Save(std::string_view filename = "");

    static bool Load(std::string_view filename = "");

    //@}

private:
    static Status<EditorStatusBit> editor_status_;    //!< 状態
    static float2                  inspector_size;
    static float2                  object_detail_size;

private:
    //! @brief 関数シリアライズ (InitSerializeの呼び出し)
    static void functionSerialize(ObjectPtr obj);

    // シリアライズされてないものがないかチェックします
    static void checkSerialized(ObjectPtr obj);
    static void checkSerialized(ComponentPtr comp);

    static void checkNextAlive();

    static BasePtr current_scene_;    //!< 現在のシーン
    static BasePtr next_scene_;       //!< 変更シーン

    static BasePtrMap scenes_;    //!< 存在する全シーン
};

//! @brief オブジェクト取得
//! @tparam T 取得したいオブジェクトクラス
//! @return 取得オブジェクト
template <class T>
std::shared_ptr<T> Scene::Base::GetObjectPtr(const std::string_view name)
{
    if(name.empty()) {
        for(auto& obj : objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            auto cast = std::dynamic_pointer_cast<T>(obj);

            if(cast)
                return cast;
        }
    }
    else {
        for(auto& obj : objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            if(name.compare(obj->GetNameDefault()) == 0) {
                auto cast = std::dynamic_pointer_cast<T>(obj);

                if(cast)
                    return cast;
            }
        }
        for(auto& obj : objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            if(name.compare(obj->GetName()) == 0) {
                auto cast = std::dynamic_pointer_cast<T>(obj);

                if(cast)
                    return cast;
            }
        }

        // 作成前Objectも検査する
        for(auto& obj : pre_objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            if(name.compare(obj->GetNameDefault()) == 0) {
                auto cast = std::dynamic_pointer_cast<T>(obj);

                if(cast)
                    return cast;
            }
        }
        for(auto& obj : pre_objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            if(name.compare(obj->GetName()) == 0) {
                auto cast = std::dynamic_pointer_cast<T>(obj);

                if(cast)
                    return cast;
            }
        }
    }

    return nullptr;
}

//! 複数オブジェクトの取得
//! @param <class T> 取得したいオブジェクトクラス
//! @return 複数の指定オブジェクト
template <class T>
std::vector<std::shared_ptr<T>> Scene::Base::GetObjectsPtr(const std::string_view name)
{
    std::vector<std::shared_ptr<T>> objects;

    if(name == "") {
        for(auto& obj : objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            auto cast = std::dynamic_pointer_cast<T>(obj);
            if(cast)
                objects.push_back(cast);
        }
        for(auto& obj : pre_objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            auto cast = std::dynamic_pointer_cast<T>(obj);
            if(cast)
                objects.push_back(cast);
        }
    }
    else {
        for(auto& obj : objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            if(obj->GetNameDefault() == name) {
                auto cast = std::dynamic_pointer_cast<T>(obj);
                if(cast)
                    objects.push_back(cast);
            }
        }
        for(auto& obj : pre_objects_) {
            if(!obj->GetStatus(::Object::StatusBit::Alive))
                continue;

            if(obj->GetNameDefault() == name) {
                auto cast = std::dynamic_pointer_cast<T>(obj);
                if(cast)
                    objects.push_back(cast);
            }
        }
    }
    return objects;
}

template <class T>
std::shared_ptr<T> Scene::Base::GetObjectPtrWithCreate(const std::string_view name)
{
    std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(Scene::GetCurrentScene()->GetObjectPtr<T>(name));
    if(ptr == nullptr) {
        ptr = std::dynamic_pointer_cast<T>(Scene::Object::Create<T>()->SetName(std::string(name)));
    }
    return ptr;
}

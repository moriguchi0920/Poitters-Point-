#include "Scene.h"
#include "ProcTiming.h"

#include <System/Object.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentHitInfo.h>
#include <System/Debug/DebugCamera.h>
#include <System/SystemMain.h>	  // ResetDeltaTime

#include <System/Component/ComponentEffect.h>	 //!< リソース解放追加

#include <algorithm>
#include <unordered_set>

//=============================================================
// シーン ローカル変数
//=============================================================
namespace
{
#ifdef _DEBUG
	bool scene_edit = true;	   //!< エディター状態
#else
	bool scene_edit = false;	//!< エディター状態
#endif
#ifdef _DEBUG
	bool scene_fps = true;	  //!< FPS状態
#else
	bool scene_fps = false;	   //!< FPS状態
#endif
	bool  scene_pause	= false;	//!< ポーズ中
	bool  scene_step	= false;	//!< 1フレームスキップ
	float scene_time	= 0.0f;		//!< タイマー
	float scene_overlap = 0.0f;		//!< シーン切り替えオーバーラップ

	bool scene_change_next = false;	   //!< 次のシーンへ移行する

	int					  select_object_index = 0;	  //!< GUIでセレクトされているオブジェクト
	std::weak_ptr<Object> selectObject;

	constexpr const char* const cap_item = "Object";
	std::string					sel_item = "";

	std::string debug_scene_name;

	//auto BindObjectUpdate( ProcTiming proc, ObjectPtr obj )
	auto BindObject(ProcTiming proc, ObjectPtr obj)
	{
		using ObjectType = void (Object::*)();

		ObjectType func_table[] = {
			&Object::PreUpdate,
			&Object::Update,
			&Object::LateUpdate,
			&Object::PrePhysics,
			&Object::PostPhysics,
			&Object::PostUpdate,
			&Object::PreDraw,
			&Object::Draw,
			&Object::LateDraw,
			&Object::PostDraw,
		};

		assert(static_cast<u32>(proc) < static_cast<u32>(ProcTiming::NUM));

		return std::bind(func_table[static_cast<int>(proc)], obj.get());

		//assert( !"処理がBindできません" );
		//return std::bind( &Object::Update, obj.get(), std::placeholders::_1 );
	}

	//auto BindComponentUpdate( ProcTiming proc, ComponentPtr cmp )
	auto BindComponent(ProcTiming proc, ComponentPtr cmp)
	{
		using ComponentType = void (Component::*)();

		ComponentType func_table[] = {
			&Component::PreUpdate,
			&Component::Update,
			&Component::LateUpdate,
			&Component::PrePhysics,
			&Component::PostPhysics,
			&Component::PostUpdate,
			&Component::PreDraw,
			&Component::Draw,
			&Component::LateDraw,
			&Component::PostDraw,
		};

		assert(static_cast<u32>(proc) < static_cast<u32>(ProcTiming::NUM));

		return std::bind(func_table[static_cast<int>(proc)], cmp.get());
	}

	ObjectWeakPtrVec leak_objs;

}	 // namespace

Scene::BasePtr				   Scene::current_scene_ = nullptr;	   //!< 現在のシーン
Scene::BasePtr				   Scene::next_scene_	 = nullptr;	   //!< 変更シーン
Scene::BasePtrMap			   Scene::scenes_		 = {};		   //!< 存在する全シーン
Status<Scene::EditorStatusBit> Scene::editor_status_;			   //!< シーン状態
float2						   Scene::inspector_size{300, 300};
float2						   Scene::object_detail_size{300, 452};

//=============================================================
// シーンクラス処理
//=============================================================

//! 初期化処理
Scene::Base::Base()
{
}

//! 終了処理
Scene::Base::~Base()
{
	UnregisterAll();
}

float Scene::Base::GetSceneTime()
{
	return scene_time;
}

//! @brief 優先を設定変更します
//! @param obj 優先を変更したいオブジェクト
//! @param timing 固定処理
//! @param priority 設定する優先
void Scene::Base::SetPriority(ObjectPtr obj, ProcTiming timing, ProcPriority priority)
{
	assert("Objectがまだ仮登録状態です。 Scene::Object::CreateDelayInitialize() で作成を試みてください。" &&
		   !obj->GetStatus(::Object::StatusBit::TempRegister));

	// 以前いるプライオリティから削除し、
	// 設定したい優先に設定する
	auto& proc	   = obj->GetProc(GetProcTimingName(timing), timing);
	proc.timing_   = timing;
	proc.priority_ = priority;
	proc.proc_	   = BindObject(timing, obj);
	resetProc(obj, proc);
	setProc(obj, proc);
}

//!	@brief 優先を設定変更します
//!	@param component 優先を変更したいコンポーネント
//!	@param timing 固定処理
//!	@param priority 設定する優先
void Scene::Base::SetPriority(ComponentPtr component, ProcTiming timing, ProcPriority priority)
{
	assert(timing <= ProcTiming::PostDraw || "Shadowよりタイミングが後のものは名前による設定が必要です");

	constexpr int prio_component_offset = 10;
	// 以前いるプライオリティから削除し、
	// 設定したい優先に設定する
	auto& proc	   = component->GetProc(GetProcTimingName(timing), timing);
	proc.timing_   = timing;
	proc.priority_ = static_cast<ProcPriority>(priority + prio_component_offset);
	proc.proc_	   = BindComponent(timing, component);
	resetProc(component, proc);
	setProc(component, proc);
}

void Scene::Base::PreRegister(ObjectPtr obj, ProcPriority update, ProcPriority draw)
{
	{
		auto itr = std::find(pre_objects_.begin(), pre_objects_.end(), obj);
		// すでに存在している?
		if(itr != pre_objects_.end())
			return;
	}
	{
		auto itr = std::find(objects_.begin(), objects_.end(), obj);
		// すでに存在している?
		if(itr != objects_.end())
			return;
	}

	// 処理を追加
	auto& proc_update	  = obj->GetProc(GetProcTimingName(ProcTiming::Update), ProcTiming::Update);
	proc_update.priority_ = update;
	auto& proc_draw		  = obj->GetProc(GetProcTimingName(ProcTiming::Draw), ProcTiming::Draw);
	proc_draw.priority_	  = draw;

	pre_objects_.push_back(obj);

	// 仮登録状態
	obj->SetStatus(::Object::StatusBit::TempRegister, true);
}

void Scene::Base::Register(ObjectPtr obj, ProcPriority update, ProcPriority draw)
{
	auto itr = std::find(objects_.begin(), objects_.end(), obj);
	if(itr != objects_.end())
	{
		// すでに存在している
		return;
	}

	// 処理を追加
	objects_.push_back(obj);

	auto lambdaProc = [this](ObjectPtr obj, ProcTiming timing, ProcPriority prio)
	{
		auto& proc	   = obj->GetProc(GetProcTimingName(timing), timing);
		proc.timing_   = timing;
		proc.priority_ = prio;
		proc.proc_	   = BindObject(timing, obj);
		proc.dirty_	   = false;
		setProc(obj, proc);
	};

	lambdaProc(obj, ProcTiming::PreUpdate, update);
	lambdaProc(obj, ProcTiming::Update, update);
	lambdaProc(obj, ProcTiming::LateUpdate, update);
	lambdaProc(obj, ProcTiming::PrePhysics, update);
	lambdaProc(obj, ProcTiming::PostPhysics, update);
	lambdaProc(obj, ProcTiming::PostUpdate, update);
	lambdaProc(obj, ProcTiming::PreDraw, draw);
	lambdaProc(obj, ProcTiming::Draw, draw);
	lambdaProc(obj, ProcTiming::LateDraw, draw);
	lambdaProc(obj, ProcTiming::PostDraw, draw);
}

void Scene::Base::RegisterForLoad(ObjectPtr obj)
{
	auto itr = std::find(objects_.begin(), objects_.end(), obj);
	if(itr != objects_.end())
	{
		// すでに存在している
		return;
	}

	// 処理を追加
	objects_.push_back(obj);

	obj->GetProc(GetProcTimingName(ProcTiming::PreUpdate), ProcTiming::PreUpdate).dirty_   = true;
	obj->GetProc(GetProcTimingName(ProcTiming::Update), ProcTiming::Update).dirty_		   = true;
	obj->GetProc(GetProcTimingName(ProcTiming::LateUpdate), ProcTiming::LateUpdate).dirty_ = true;

	obj->GetProc(GetProcTimingName(ProcTiming::PrePhysics), ProcTiming::PrePhysics).dirty_	 = true;
	obj->GetProc(GetProcTimingName(ProcTiming::PostPhysics), ProcTiming::PostPhysics).dirty_ = true;

	obj->GetProc(GetProcTimingName(ProcTiming::PostUpdate), ProcTiming::PostUpdate).dirty_ = true;
	obj->GetProc(GetProcTimingName(ProcTiming::PreDraw), ProcTiming::PreDraw).dirty_	   = true;
	obj->GetProc(GetProcTimingName(ProcTiming::Draw), ProcTiming::Draw).dirty_			   = true;
	obj->GetProc(GetProcTimingName(ProcTiming::LateDraw), ProcTiming::LateDraw).dirty_	   = true;

	obj->GetProc(GetProcTimingName(ProcTiming::PostDraw), ProcTiming::PostDraw).dirty_ = true;
}

void Scene::Base::Unregister(ObjectPtr obj)
{
	auto itr = std::find(objects_.begin(), objects_.end(), obj);
	signal_gate_states_.erase(obj.get());

	obj->RemoveAllProcesses();

	// リストから削除( 自動deleteされる )
	(*itr)->RemoveAllComponents();
	(*itr)->ModifyComponents();
	objects_.erase(itr);
}

void Scene::Base::UnregisterAll()
{
	// オブジェクトを消去 (自動delete)
	for(auto& obj : objects_)
	{
		obj->RemoveAllComponents();
		obj->ModifyComponents();

		obj->RemoveAllProcesses();
	}
	for(auto obj : objects_) leak_objs.push_back(obj);

	objects_.clear();
	signal_gate_states_.clear();

	// シグナルカット
	for(auto& s : signals_) s.disconnect_all();
}

//! 同じシーンタイプがいないかチェックする
bool Scene::Base::IsSceneExist(const BasePtr& scene)
{
	auto it = scenes_.find(scene->typeInfo()->className());

	if(it != scenes_.end())
	{
		// まだ生きている同じタイプのシーンがあり、それが自分ではない
		if(it->second != scene)
			return true;
	}

	return false;
}

void Scene::Base::SetScene(const BasePtr& scene)
{
	auto name = scene->typeInfo()->className();

	auto it = scenes_.find(name);
	if(it != scenes_.end())
	{
		scenes_[name] = scene;
	}
}

void Scene::Base::ReleaseScene(const BasePtr& scene)
{
	auto name = scene->typeInfo()->className();

	auto it = scenes_.find(name);
	if(it != scenes_.end())
	{
		scenes_.erase(name);
	}
}

//! @brief オブジェクトの指定処理を指定優先で処理するように登録する
//! @param obj オブジェクト
//! @param timing 指定処理
//! @param priority 指定優先
void Scene::Base::setProc(ObjectPtr obj, SlotProc& slot)
{
	auto& proc = obj->GetProc(slot.GetName(), slot.GetTiming());

	if(slot.func_ == nullptr)
	{
		proc.SetProc(slot.GetName(), slot.GetTiming(), slot.GetPriority(), slot.GetProc());
		proc.connect_ = current_scene_->GetSignals(slot.GetTiming()).connect(proc.proc_, (int)proc.priority_);
	}
	else
	{
		proc.SetAddProc(slot.GetAddProc(), slot.GetTiming(), slot.GetPriority());
		auto p		   = proc.GetAddProc().get();
		auto ptr	   = std::bind(&Callable::Exec, p);
		proc.GetProc() = ptr;
		proc.connect_  = current_scene_->GetSignals(slot.GetTiming()).connect(proc.proc_, (int)proc.priority_);
	}
}

//! @brief オブジェクトの指定処理を削除する
//! @param obj 削除するオブジェクト
//! @param timing 指定処理
void Scene::Base::resetProc(ObjectPtr, SlotProc& slot)
{
	// 以前いるconnectから削除
	if(slot.connect_.valid())
		slot.connect_.disconnect();
}

//! @brief コンポーネントの指定処理を指定優先で処理するように登録する
//! @param component コンポーネント
//! @param timing 指定処理
//! @param priority 指定優先
void Scene::Base::setProc(ComponentPtr component, SlotProc& slot)
{
	auto& proc = component->GetProc(slot.GetName(), slot.GetTiming());
	if(slot.GetProc() == nullptr)
	{
		slot.SetProc(slot.GetName(), slot.GetTiming(), slot.GetPriority(), BindComponent(slot.GetTiming(), component));
	}

	proc.SetProc(slot.GetName(), slot.GetTiming(), slot.GetPriority(), slot.GetProc());

	// 設定したい優先に設定する
	proc.connect_ = current_scene_->GetSignals(slot.GetTiming()).connect(proc.proc_, (int)proc.priority_);
}

//! @brief コンポーネントの指定処理を削除する
//! @param component 削除するオブジェクト
//! @param timing 指定処理
void Scene::Base::resetProc(ComponentPtr, SlotProc& slot)
{
	// 以前いるプライオリティから削除
	if(slot.connect_.valid())
		slot.connect_.disconnect();
}

//! @brief オブジェクトの状態に応じてシグナル状態を変更する
//! @param obj 対象オブジェクト
//! @param is_pause ポーズ中かどうか
//! @param force_sync 強制的にシグナルの状態をオブジェクトの状態に合わせるかどうか
void Scene::Base::applyObjectSignalState(ObjectPtr obj, bool is_pause, bool force_sync)
{
	auto* object_key = obj.get();
	auto& cache		 = signal_gate_states_[object_key];

	const bool object_update_blocked = obj->GetStatus(::Object::StatusBit::NoUpdate) || is_pause;
	const bool object_draw_blocked	 = obj->GetStatus(::Object::StatusBit::NoDraw);

	// オブジェクトの状態に応じてシグナル状態を変更
	if(force_sync || !cache.initialized_ || cache.object_update_blocked_ != object_update_blocked)
	{
		for(auto& sig : obj->proc_timings_)
		{
			if((int)sig.second.IsUpdate())
			{
				if(object_update_blocked)
					sig.second.connect_.block();
				else
					sig.second.connect_.unblock();
			}
		}
		cache.object_update_blocked_ = object_update_blocked;
	}

	// オブジェクトの状態に応じてシグナル状態を変更
	if(force_sync || !cache.initialized_ || cache.object_draw_blocked_ != object_draw_blocked)
	{
		for(auto& sig : obj->proc_timings_)
		{
			if((int)sig.second.IsDraw())
			{
				if(object_draw_blocked)
					sig.second.connect_.block();
				else
					sig.second.connect_.unblock();
			}
		}
		cache.object_draw_blocked_ = object_draw_blocked;
	}

	// コンポーネントの状態に応じてシグナル状態を変更
	auto& component_states = cache.component_states_;
	for(auto& component : obj->GetComponents())
	{
		auto* component_key = component.get();
		auto& comp_state	= component_states[component_key];

		// コンポーネントのUpdate/Draw状態を取得
		const bool component_update_blocked = object_update_blocked || !component->GetStatus(::Component::StatusBit::Enable) ||
											  (is_pause && !component->GetStatus(::Component::StatusBit::DisablePause));
		const bool component_draw_blocked = object_draw_blocked || !component->GetStatus(::Component::StatusBit::Enable);

		// コンポーネントの状態に応じてシグナル状態を変更
		if(force_sync || !cache.initialized_ || comp_state.update_blocked_ != component_update_blocked)
		{
			for(auto& sig : component->proc_timings_)
			{
				if((int)sig.second.IsUpdate())
				{
					if(component_update_blocked)
						sig.second.connect_.block();
					else
						sig.second.connect_.unblock();
				}
			}
			comp_state.update_blocked_ = component_update_blocked;
		}

		// コンポーネントの状態に応じてシグナル状態を変更
		if(force_sync || !cache.initialized_ || comp_state.draw_blocked_ != component_draw_blocked)
		{
			for(auto& sig : component->proc_timings_)
			{
				if((int)sig.second.IsDraw())
				{
					if(component_draw_blocked)
						sig.second.connect_.block();
					else
						sig.second.connect_.unblock();
				}
			}
			comp_state.draw_blocked_ = component_draw_blocked;
		}
	}

	// 存在しないコンポーネントの状態をキャッシュから削除
	if(component_states.size() != obj->GetComponents().size())
	{
		std::unordered_set<::Component*> alive_components;
		alive_components.reserve(obj->GetComponents().size());
		for(auto& component : obj->GetComponents())
		{
			alive_components.insert(component.get());
		}
		for(auto it = component_states.begin(); it != component_states.end();)
		{
			if(alive_components.find(it->first) == alive_components.end())
				it = component_states.erase(it);
			else
				++it;
		}
	}

	cache.initialized_ = true;
}

//! @brief
void Scene::Base::cleanupSignalStateCache()
{
	std::unordered_set<::Object*> alive_objects;
	alive_objects.reserve(objects_.size());
	for(auto& obj : objects_)
	{
		alive_objects.insert(obj.get());
	}

	for(auto it = signal_gate_states_.begin(); it != signal_gate_states_.end();)
	{
		if(alive_objects.find(it->first) == alive_objects.end())
			it = signal_gate_states_.erase(it);
		else
			++it;
	}
}

//----------------------------------------------------------------------
//! @name Cereal セーブロード
//----------------------------------------------------------------------
//@{

bool Scene::Base::Save(const std::string_view filename)
{
	// imguiで変更したstringはサイズを間違えていることがあるため
	// 一旦char*に変更したものをstring化する
	// 違う場合は、string + stringがうまく表示されなくなる
	std::string name = std::string(filename.data());
	if(filename.empty())
	{
		name = typeInfo()->className();
	}

	HelperLib::File::CreateFolder(".\\data\\_save\\");
	std::string fname = ".\\data\\_save\\" + name + ".txt";

	HelperLib::File::AddUniqueSuffixWidthFile(fname);

	std::ofstream file(fname);
	if(!file)
		return false;

	// 存在するオブジェクトをセーブする
	{
		ObjectPtrVec objects;
		ObjectPtrVec pre_objects;

		objects.reserve(objects_.size());
		pre_objects.reserve(pre_objects_.size());

		for(auto& obj : objects_)
			if(!obj->GetStatus(::Object::StatusBit::NoSerialize))
				objects.push_back(obj);
		for(auto& obj : pre_objects_)
			if(!obj->GetStatus(::Object::StatusBit::NoSerialize))
				pre_objects.push_back(obj);

		cereal::JSONOutputArchive o_archive(file);
		{
			o_archive(CEREAL_NVP(version_),
					  cereal::make_nvp("objects_", objects),
					  cereal::make_nvp("pre_objects_", pre_objects),
					  CEREAL_NVP(status_.get()),
					  cereal::make_nvp("time_", scene_time));
		}
	}
	file.close();

	return true;
}

bool Scene::Base::Load(const std::string_view filename)
{
	UnregisterAll();
	Scene::CheckLeak();

	std::string name = std::string(filename.data());
	if(filename.empty())
	{
		name = typeInfo()->className();
	}

	std::string path = ".\\data\\_save\\" + name + ".txt";

	// 正規のデータがあれば先に読み込みをおこなう
	std::ifstream file(path);

	if(!file.is_open())
		file.open(path);

	if(!file)
		return false;

	// バージョン付
	bool newer = HelperLib::File::IsFileNewerThan(path, "2025-07-04");

	// 存在するオブジェクトをロードする
	{
		cereal::JSONInputArchive i_archive(file);
		// バージョンチェック
		if(newer)
		{
			i_archive(CEREAL_NVP(version_), CEREAL_NVP(objects_), CEREAL_NVP(pre_objects_), CEREAL_NVP(status_.get()), cereal::make_nvp("time_", scene_time));
		}
		else
		{
			// 昔のものが time_で確保されているため、こちらで対応する
			i_archive(CEREAL_NVP(objects_), CEREAL_NVP(status_.get()), cereal::make_nvp("time_", scene_time));
		}
		// 処理のシリアライズは再度行う
		status_.off(StatusBit::Serialized);
	}
	return true;
}

//@}

void Scene::functionSerialize(ObjectPtr obj)
{
	// オブジェクト
	if(!obj->GetStatus(::Object::StatusBit::Serialized))
	{
		obj->InitSerialize();
		assert("継承先のInitSerialize()にて__super::InitSerialize()を入れてください." && obj->GetStatus(::Object::StatusBit::Serialized));

		// シリアライズされない処理を確認
		checkSerialized(obj);
	}

	// コンポーネントシグナル設定
	for(auto component : obj->GetComponents())
	{
		if(!component->GetStatus(Component::StatusBit::Initialized))
		{
			for(ProcTiming i = ProcTiming::PreUpdate; (int)i <= (int)ProcTiming::PostDraw; i = (ProcTiming)((int)i + 1))
			{
				auto& proc = component->GetProc(GetProcTimingName(i), i);
				current_scene_->setProc(component, proc);
				proc.ResetDirty();
			}
			component->Init();
			assert("継承先のInit()にて__super::Init()を入れてください." && component->GetStatus(Component::StatusBit::Initialized));
		}
		if(!component->GetStatus(Component::StatusBit::Serialized))
		{
			component->InitSerialize();
			assert("継承先のInitSerialize()にて__super::InitSerialize()を入れてください." && component->GetStatus(Component::StatusBit::Serialized));

			checkSerialized(component);
		}
	}
}

void Scene::checkSerialized(ObjectPtr obj)
{
	// 標準のUpdate以外のシグナルを復旧
	for(auto& sig : obj->proc_timings_)
	{
		auto& dat = sig.second;
		if(dat.GetName() == GetProcTimingName(dat.GetTiming()))
		{
			dat.SetProc(dat.GetName(), dat.GetTiming(), dat.GetPriority(), BindObject(dat.GetTiming(), obj));
		}
	}

	// 処理がnullptrのものがないかチェック

	for(auto itr = obj->proc_timings_.begin(); itr != obj->proc_timings_.end();)
	{
		auto&				   proc = *itr;
		[[maybe_unused]] auto& p	= proc.second;

		if(p.GetProc() == nullptr && p.GetAddProc() == nullptr)
		{
			assert(
				0 &&
				"SetProcで追加した処理がSerializeされません。処理をProcAddProc()で作成して登録するか、初期化する同じシーンを選択しInitSerialize()で初期化してください。ここは、「無視」することで進めますが、処理は復活しません");

			itr = obj->proc_timings_.erase(itr);
			continue;
		}
		itr++;
	}
}

void Scene::checkSerialized(ComponentPtr comp)
{
	// 標準のシグナルを復旧
	for(auto& sig : comp->proc_timings_)
	{
		auto& dat = sig.second;
		if(dat.GetName() == GetProcTimingName(dat.GetTiming()))
		{
			dat.SetProc(dat.GetName(), dat.GetTiming(), dat.GetPriority(), BindComponent(dat.GetTiming(), comp));
		}
	}

	// 処理がnullptrのものがないかチェック
	for(auto itr = comp->proc_timings_.begin(); itr != comp->proc_timings_.end();)
	{
		auto&				   proc = *itr;
		[[maybe_unused]] auto& p	= proc.second;

		if(p.GetProc() == nullptr && p.GetAddProc() == nullptr)
		{
			assert(
				0 &&
				"SetProcで追加した処理がSerializeされません。処理をProcAddProc()で作成して登録するか、初期化する同じシーンを選択しInitSerialize()で初期化してください。ここは、「無視」することで進めますが、処理は復活しません");

			itr = comp->proc_timings_.erase(itr);
			continue;
		}
		itr++;
	}
}

void Scene::checkNextAlive()
{
	if(current_scene_)
	{
		// シーン切り替えで前の状態を確保する?
		if(current_scene_->GetStatus(Base::StatusBit::AliveInAnotherScene))
		{
			Scene::Base::SetScene(current_scene_);
		}
		else
		{
			Scene::Base::ReleaseScene(current_scene_);
		}
	}
}

//=============================================================
// シーン処理
//=============================================================
void Scene::ReleaseObject(std::string_view name)
{
	if(current_scene_)
	{
		auto obj = Object::Get<::Object>(name);
		if(obj)
			obj->SetStatus(::Object::StatusBit::Alive, false);
	}
}

void Scene::ReleaseObject(ObjectPtr obj)
{
	if(current_scene_ && obj)
		obj->SetStatus(::Object::StatusBit::Alive, false);
}

void Scene::Object::Release(std::string_view name)
{
	if(current_scene_)
	{
		auto obj = Object::Get<::Object>(name);
		if(obj)
			obj->SetStatus(::Object::StatusBit::Alive, false);
	}
}

void Scene::Object::Release(ObjectPtr obj)
{
	if(current_scene_ && obj)
		obj->SetStatus(::Object::StatusBit::Alive, false);
}

//! 次のシーンをセットする
void Scene::SetNextScene(BasePtr scene)
{
	checkNextAlive();

	if(scene && Base::IsSceneExist(scene))
	{
		// 同じ名前のシーンを削除
		// TODO log/ensure処理
		assert(0 && "すでに同じタイプのシーンが用意されています。前のシーンは「無視」を押すことで削除して進めることができます");
		Base::ReleaseScene(scene);
	}

	if(current_scene_ == nullptr)
	{
		current_scene_ = scene;
	}
	else
	{
		next_scene_		  = scene;
		scene_change_next = true;
	}

	Init();

	::Object::ClearObjectNames();
}

void Scene::CheckLeak()
{
	bool found = false;
	for(auto scene : scenes_)
	{
		// 登録シーンの中にあるか?
		if(current_scene_ == scene.second)
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		// objectが解放できていない?
		ObjectWeakPtrVec objs = leak_objs;
		leak_objs.clear();
		for(auto obj : objs)
		{
			if(auto o = obj.lock())
			{
				// コンポーネントをとらえてシーン移動しようとしている
				bool leak = false;
				for(auto weak_cmp : o->leak_components_)
				{
					if(auto cmp = weak_cmp.lock())
						leak = true;
				}

				if(leak)
				{
					std::string str =
						"オブジェクト名: " + std::string(o->GetName()) +
						"\nオブジェクト内にてComponentPtrなどの変数で\nコンポーネントを確保したままになってないか確認してください.\n解決方法は、\n\n1. Objectの変数としない\n2. Exit()でnullptrにする\n3. weak_ptrに変更する\n\nいずれかを行うことで解決します.";
					MessageBox(GetMainWindowHandle(), str.c_str(), "Objectが正しく解放できませんでした.", MB_OK);
				}
				// リークとしてとらえておく
				leak_objs.push_back(obj);
			}
		}
	}
}

//! 次のシーンに切り替える
void Scene::ChangeNextScene()
{
	bool found = false;
	for(auto scene : scenes_)
	{
		// 登録シーンの中にあるか?
		if(current_scene_ == scene.second)
		{
			found = true;
			break;
		}
	}

	current_scene_ = next_scene_;
	if(!found)
	{
		// objectが解放できていない?
		ObjectWeakPtrVec objs = leak_objs;
		leak_objs.clear();
		for(auto obj : objs)
		{
			if(auto o = obj.lock())
			{
				// コンポーネントをとらえてシーン移動しようとしている
				bool leak = false;
				for(auto weak_cmp : o->leak_components_)
				{
					if(auto cmp = weak_cmp.lock())
						leak = true;
				}

				if(leak)
				{
					std::string str =
						"オブジェクト名: " + std::string(o->GetName()) +
						"\nオブジェクト内にてComponentPtrなどの変数で\nコンポーネントを確保したままになってないか確認してください.\n解決方法は、\n\n1. Objectの変数としない\n2. Exit()でnullptrにする\n3. weak_ptrに変更する\n\nいずれかを行うことで解決します.";
					MessageBox(GetMainWindowHandle(), str.c_str(), "Objectが正しく解放できませんでした.", MB_OK);
				}
				// リークとしてとらえておく
				leak_objs.push_back(obj);
			}
		}
	}

	// 終了の際は、確実にWindowを閉じます
	if(current_scene_ == nullptr)
	{
		DestroyWindow(GetMainWindowHandle());
	}

	next_scene_		  = nullptr;
	scene_change_next = false;

	scene_pause = false;	// ポーズ解除
}

//! 現在アクティブなシーンを取得します
Scene::Base* Scene::GetCurrentScene()
{
	return current_scene_.get();
}

//! 初期化処理
void Scene::Init()
{
	// ここではObjはInitしない
	// Worldが出来上がり、その後、
	// 個別にオブジェクトは作成されるため
	// Update内で、Initする形をとる

	debug_scene_name = current_scene_->typeInfo()->className();
	if(next_scene_)
		debug_scene_name = next_scene_->typeInfo()->className();
}

void Scene::Change(BasePtr scene)
{
	// すでに存在している?
	if(scene && Base::IsSceneExist(scene))
	{
		// とらえているシーンに切り替える
		SetNextScene(scenes_[scene->typeInfo()->className()]);
		return;
	}

	SetNextScene(scene);
}

void Scene::PreUpdate()
{
	if(IsKeyOn(KEY_INPUT_F1))
		scene_pause = !scene_pause;

	if(IsKeyOn(KEY_INPUT_F2))
		scene_step = true;

	if(next_scene_ != nullptr || scene_change_next)
	{
		// シーン切り替え
		current_scene_->Exit();

		// 継承し忘れの時のため
		Scene::Exit();

		// シーン切り替えの時にクリック情報などが新しいシーンに
		// 影響を及ぼすため、ここでトリガー情報をクリアする
		InputKeyUpdate();
		InputPadUpdate();
		InputMouseUpdate();

		//ChangeNextScene();
		scene_time = 0.0f;
	}

	if(current_scene_)
	{
		if(!current_scene_->GetStatus(Scene::Base::StatusBit::Initialized))
		{
			// シーンを初期化
			bool initialized = current_scene_->Init();
			current_scene_->SetStatus(Scene::Base::StatusBit::Initialized, initialized);

			// シーン初期化でブロッキングロードをしていた場合に時間経過が大きくなるためリセット
			ResetDeltaTime();
		}
		// シーン初期化できてない場合は Objectも初期化しない
		if(!current_scene_->GetStatus(Scene::Base::StatusBit::Initialized))
			return;

		// オブジェクト仮登録しているものを本登録に変更
		for(auto obj : current_scene_->pre_objects_)
		{
			auto& proc_update = obj->GetProc(GetProcTimingName(ProcTiming::Update), ProcTiming::Update);
			auto& proc_draw	  = obj->GetProc(GetProcTimingName(ProcTiming::Draw), ProcTiming::Draw);

			current_scene_->Register(obj, proc_update.priority_, proc_draw.priority_);

			obj->SetStatus(::Object::StatusBit::TempRegister, false);
		}
		// 仮登録のクリア
		current_scene_->pre_objects_.clear();

		if(!current_scene_->GetStatus(Scene::Base::StatusBit::Serialized))
		{
			current_scene_->InitSerialize();
			current_scene_->SetStatus(Scene::Base::StatusBit::Serialized, true);
		}

		current_scene_->cleanupSignalStateCache();

		// 処理優先とポーズのチェック
		for(auto& obj : current_scene_->GetObjectPtrVec())
		{
			obj->toFormalComponent();

			// ポーズ中
			bool is_pause = false;
			if(obj->GetStatus(::Object::StatusBit::IsPause) || ((scene_pause && !scene_step) && !obj->GetStatus(::Object::StatusBit::DisablePause)))
				is_pause = true;

			//if( !is_pause )
			{
				// Init前状態
				if(!obj->GetStatus(::Object::StatusBit::Initialized))
				{
					bool ret = obj->Init();
					if(!ret)
						continue;	 //!< 初期化未終了

					assert("継承先のInit()にて__super::Init()を入れてください." && obj->GetStatus(::Object::StatusBit::Initialized));
				}

				functionSerialize(obj);
			}
			bool force_signal_sync = false;
#if 1
			const bool is_loading	 = current_scene_->GetStatus(Scene::Base::StatusBit::IsLoading);
			auto	   relinkIfDirty = [&](auto&& owner, SlotProc& proc)
			{
				if(!proc.IsDirty())
					return;
				current_scene_->resetProc(owner, proc);
				current_scene_->setProc(owner, proc);
				proc.ResetDirty();
				force_signal_sync = true;
			};

			// dirty
			if(!is_loading)
			{
				for(auto& name : obj->dirty_object_list_)
				{
					auto& proc = obj->GetProc(name, ProcTiming::Update);
					relinkIfDirty(obj, proc);
				}
				obj->dirty_object_list_.clear();

				for(auto& comp : obj->components_)
				{
					// Componentのdirtyのチェック
					for(auto& name : comp->dirty_component_list_)
					{
						auto& proc = obj->GetProc(name, ProcTiming::Update);
						relinkIfDirty(comp, proc);
					}
					comp->dirty_component_list_.clear();
				}
			}
			else
			{
				for(auto& timing : obj->proc_timings_)
				{
					relinkIfDirty(obj, timing.second);
				}

				for(auto& comp : obj->components_)
				{
					// Componentのdirtyのチェック
					for(auto& timing : comp->proc_timings_)
					{
						relinkIfDirty(comp, timing.second);
					}
				}

				if(--current_scene_->loading_wait_ <= 0)
				{
					current_scene_->SetStatus(Scene::Base::StatusBit::IsLoading, false);
					current_scene_->loading_wait_ = 0;
				}
			}
#endif
			// シグナルのカット状態をオブジェクトの状態に合わせる
			current_scene_->applyObjectSignalState(obj, is_pause, force_signal_sync);
		}
		current_scene_->GetSignals(ProcTiming::PreUpdate)();
	}
}

//! 更新処理
void Scene::Update()
{
	f32 delta = GetDeltaTime();

	if(current_scene_)
	{
		if(!scene_pause || scene_step)
		{
			current_scene_->Update();
		}

		current_scene_->GetSignals(ProcTiming::Update)();

		if(!scene_pause || scene_step)
		{
			current_scene_->LateUpdate();
			scene_time += delta;
		}
		current_scene_->GetSignals(ProcTiming::LateUpdate)();
	}
}

void Scene::PrePhysics()
{
	if(current_scene_)
	{
		current_scene_->PrePhysics();

		current_scene_->GetSignals(ProcTiming::PrePhysics)();

		// Physics
		CheckComponentCollisions();
	}
}

void Scene::PostPhysics()
{
	if(current_scene_)
	{
		current_scene_->PostPhysics();

		current_scene_->GetSignals(ProcTiming::PostPhysics)();
	}
}

void Scene::PostUpdate()
{
	if(current_scene_)
	{
		current_scene_->PostUpdate();
		current_scene_->GetSignals(ProcTiming::PostUpdate)();
	}
}

void Scene::Draw()
{
	// シーンが無ければ何もしない
	if(!current_scene_)
		return;

	//----------------------------------------------------------
	// 影, GBuffer, 照明計算を先に実行する
	//----------------------------------------------------------
	current_scene_->GetSignals(ProcTiming::Shadow)();
	current_scene_->GetSignals(ProcTiming::GBuffer)();
	current_scene_->GetSignals(ProcTiming::Light)();

	//----------------------------------------------------------
	// シーンPreDrawの実行
	//----------------------------------------------------------
	current_scene_->PreDraw();
	current_scene_->GetSignals(ProcTiming::PreDraw)();

	//----------------------------------------------------------
	// シーンDrawの実行
	//----------------------------------------------------------
	current_scene_->Draw();

	current_scene_->GetSignals(ProcTiming::Draw)();

	current_scene_->LateDraw();
	current_scene_->GetSignals(ProcTiming::LateDraw)();

	current_scene_->PostDraw();
	current_scene_->GetSignals(ProcTiming::PostDraw)();

	//----------------------------------------------------------
	current_scene_->GetSignals(ProcTiming::HDR)();
	current_scene_->GetSignals(ProcTiming::Filter)();
	current_scene_->GetSignals(ProcTiming::UI)();

	scene_step = false;

	// 未使用のコンポーネントを削除
	for(auto obj : current_scene_->GetObjectPtrVec())
	{
		obj->ModifyComponents();
	}

	// 終了した場合( Exit()を呼んだ場合 )
	for(int i = (int)current_scene_->GetObjectPtrVec().size() - 1; i >= 0; --i)
	{
		auto obj = current_scene_->GetObjectPtrVec()[i];
		if(!obj->GetStatus(::Object::StatusBit::Alive))
		{
			if(!obj->GetStatus(::Object::StatusBit::Exited))
				obj->Exit();
		}
		if(obj->GetStatus(::Object::StatusBit::Exited))
		{
			current_scene_->Unregister(obj);
			continue;
		}
	}

	// PauseSystem::DrawPause();
}

void Scene::Exit()
{
	if(current_scene_)
	{
		for(auto& obj : current_scene_->GetObjectPtrVec())
		{
			if(!obj->GetStatus(::Object::StatusBit::Exited))
			{
				obj->Exit();
			}
		}
		current_scene_->Exit();
		current_scene_->UnregisterAll();
	}

	//TODO シーンがEffectを見ているため、Delegateなどに変更する必要あり
	ComponentEffect::ClearResource();
#if 0
    physx_lib->Exit();
#endif
	//	Change( nullptr );
	ChangeNextScene();
}

template <class T>
void Scene::Base::PreRegister(T* obj, ProcPriority update, ProcPriority draw)
{
	auto obj_ptr = std::shared_ptr<T>(obj);
	PreRegister(obj_ptr, update, draw);
}

void Scene::GUI()
{
	bool is_select_ok = !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)	 // Hover
						&& !ImGuizmo::IsOver()																					 // Guizom
						&& !IsShowMenu();																						 // ShowMenu

	// オブジェクトPICK
	if(IsMouseOn(MOUSE_INPUT_LEFT) && is_select_ok)
	{
		auto   obj	= PickObject(GetMouseX(), GetMouseY());
		auto   objs = current_scene_->GetObjectPtrVec();
		size_t max	= objs.size();

		for(int i = 0; i < max; i++)
		{
			if(obj == objs[i])
			{
				select_object_index = i;
				selectObject		= current_scene_->GetObjectPtrVec()[select_object_index];
			}
		}
	}

	// エディター状態でなければGUIは出ない
	if(!scene_edit)
		return;

	// シーンが無ければ何もしない
	if(!current_scene_)
		return;

	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));

	// GUIウインドウ設定
	SetGUIWindow(SET_GUI_ATTACH_RIGHT, 0, (int)inspector_size.x, (int)inspector_size.y);

	// シーンのGUIを表示
	std::string name = "Scene : " + std::string(current_scene_->typeInfo()->className());

	auto flags = ImGuiWindowFlags_None;
	if(GetEditorStatus(EditorStatusBit::EditorPlacement))
		flags = ImGuiWindowFlags_NoCollapse;

	ImGui::Begin(name.c_str(), (bool*)nullptr, flags);
	{
		ImGui::InputText(u8"名前", debug_scene_name.data(), 1024, ImGuiInputTextFlags_EnterReturnsTrue);

		if(ImGui::Button("Save"))
		{
			int result = MessageBox(NULL,
									TEXT("シーンを セーブ(Save) します？\n「はい」を選ぶと以前のデータは上書きされます。"),
									TEXT("Save Scene"),
									MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
			if(result == IDYES)
			{
				Save(debug_scene_name);
			}
		}
		ImGui::SameLine();
		if(ImGui::Button("Load"))
		{
			int result = MessageBox(NULL,
									TEXT("シーンを ロード(Load) します？\n「はい」を選ぶと作成中のシーンはなくなります"),
									TEXT("Load Scene"),
									MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
			if(result == IDYES)
			{
				Load(debug_scene_name.data());
				current_scene_->SetStatus(Scene::Base::StatusBit::IsLoading, true);
				current_scene_->loading_wait_ = 5;
			}
		}

		if(ImGui::TreeNode(u8"登録シーン"))
		{
			ImGui::Text(u8"シーン数 : %d", GetSceneCount());
			ImGui::Text(u8"TODO: 存在シーン列挙");
			ImGui::Text(u8"全Object数 : %d", ::Object::ExistObjectCount());

			if(leak_objs.size() > 0)
				ImGui::TextColored({1, 0, 0, 1}, u8"LeakObject数 : %d", leak_objs.size());
			ImGui::TreePop();
		}

		ImGui::DragFloat(u8"経過時間", &scene_time, 0.1f, 0, 0, "%.2f");
		ImGui::Text(u8"シーン内Object数 : %d", current_scene_->GetObjectPtrVec().size());

		//------------------------------------------
		// 登録されているObjectを列挙する
		//------------------------------------------
		auto&							  object	  = ::Object::Type;
		static ClassObjectType<::Object>* object_type = nullptr;

		if(ImGui::BeginCombo(cap_item, sel_item.data()))
		{
			// Object本体
			{
				// 表示文字列
				std::string cls = object.className();
				auto		w	= std::max(int(16 - cls.size()), 0);

				auto object_name = std::format("<{}> {:<{}}: {}", cls, "", w, object.descName());

				bool is_selected = (sel_item == object_name);
				if(ImGui::Selectable(object_name.data(), is_selected))
				{
					sel_item	= object_name;
					object_type = &object;
				}
				if(is_selected)
					ImGui::SetItemDefaultFocus();
			}

			for(auto* p = object.child(); p; p = p->siblings())
			{
				// 表示文字列
				std::string cls = p->className();
				auto		w	= std::max(int(16 - cls.size()), 0);

				auto object_name = std::format("<{}> {:<{}}: {}", cls, "", w, p->descName());

				bool is_selected = (sel_item == object_name);
				if(ImGui::Selectable(object_name.data(), is_selected))
				{
					sel_item	= object_name;
					object_type = (ClassObjectType<::Object>*)p;
				}
				if(is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if(ImGui::Button(u8"作成 (Scene::Object::Create)") && object_type != nullptr)
		{
			object_type->createObjectPtr();
		}
		if(auto obj = SelectObjectPtr())
		{
			//ImGui::SameLine();
			if(ImGui::Button(u8"削除 (Scene::Object::Release)"))
			{
				Scene::Object::Release(obj);
			}
		}

		std::vector<const char*> listbox;
		listbox.reserve(current_scene_->GetObjectPtrVec().size());
		for(auto& obj : current_scene_->GetObjectPtrVec())
		{
			listbox.emplace_back(obj->GetName().data());
		}

		int size = (int)current_scene_->GetObjectPtrVec().size();

		//  シーン オブジェクトインスペクタ
		auto xy = ImGui::GetWindowSize();
		ImGui::BeginChild(ImGui::GetID((void*)Scene::GUI), ImVec2(xy.x - 40, 150), ImGuiWindowFlags_NoTitleBar);
		{
			for(int i = 0; i < size; i++)
			{
				auto obj = current_scene_->GetObjectPtrVec()[i];
				if(!Scene::GetEditorStatus(EditorStatusBit::EditorPlacement))
				{
					ImGui::CheckboxFlags(std::to_string(i).c_str(), (int*)&obj->status_, 1 << (int)::Object::StatusBit::ShowGUI);
					ImGui::SameLine();
				}
				if(ImGui::Selectable(obj->GetName().data(), select_object_index == i))
				{
					select_object_index = i;
					selectObject		= current_scene_->GetObjectPtrVec()[select_object_index];
				}
			}
			for(int i = 0; i < leak_objs.size(); i++)
			{
				auto weak_obj = leak_objs[i];
				auto obj	  = weak_obj.lock();
				if(obj == nullptr)
					continue;

				if(!Scene::GetEditorStatus(EditorStatusBit::EditorPlacement))
				{
					ImGui::CheckboxFlags(std::to_string(i).c_str(), (int*)&obj->status_, 1 << (int)::Object::StatusBit::ShowGUI);
					ImGui::SameLine();
				}
				if(ImGui::Selectable(obj->GetName().data(), select_object_index == i + size, ImGuiSelectableFlags_::ImGuiSelectableFlags_Disabled))
				{
					select_object_index = i + size;
					selectObject		= leak_objs[select_object_index - size];
				}
			}
		}
		ImGui::EndChild();
		/*
		ImGui::ListBox( u8"シーン内オブジェクト", &select_object_index, listbox.data(), (int)listbox.size(), size );
		if( select_object_index != -1 )
		{
			selectObject = current_scene_->Object::GetArray()[ select_object_index ];
			auto obj	 = selectObject.lock();
			bool b		 = !obj->GetStatus( Object::StatusBit::ShowGUI );
			obj->SetStatus( Object::StatusBit::ShowGUI, b );
		}
		*/
		auto ims	   = ImGui::GetWindowSize();
		inspector_size = {ims.x, ims.y};

		ImGui::Dummy({0, 2});
		ImGui::Separator();
		// シーンのGUI描画を実行
		current_scene_->GUI();
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	size_t size = current_scene_->GetObjectPtrVec().size();
	for(int i = 0; i < size; i++)
	{
		auto obj = current_scene_->GetObjectPtrVec()[i];
		if(obj->GetStatus(::Object::StatusBit::ShowGUI))
		{
			// GUIウインドウ設定
			if(Scene::GetEditorStatus(Scene::EditorStatusBit::EditorPlacement))
			{
				if(i != select_object_index)
				{
					continue;
				}

				Scene::SetGUIWindow(Scene::SET_GUI_ATTACH_RIGHT, (int)inspector_size.y, (int)object_detail_size.x, (int)object_detail_size.y);
			}

			obj->SetStatus(::Object::StatusBit::CalledGUI, false);
			obj->GUI();
			assert("継承先のGUI()にて__super::GUI()を入れてください." && obj->GetStatus(::Object::StatusBit::CalledGUI));

			for(auto component : obj->GetComponents())
			{
				if(component->GetStatus(Component::StatusBit::ShowGUI))
					component->GUI();
#ifdef _DEBUG
//          obj->Checker();
#endif	  //_DEBUG
			}
		}
	}
}

bool Scene::IsPause()
{
	return scene_pause && !scene_step;
}

float Scene::GetTime()
{
	return scene_time;
}

//! @brief ComponentCollisionの当たり判定を行う
void Scene::CheckComponentCollisions()
{
	ObjectPtrVec objs;
	ObjectPtrVec last_objs;
	// オブジェの数を取得
	int obj_num = (int)current_scene_->GetObjectPtrVec().size();
	for(int obj_index = 0; obj_index < obj_num; obj_index++)
	{
		bool find = false;
		// 調べるオブジェクト
		auto obj  = current_scene_->GetObjectPtrVec()[obj_index];
		auto cols = obj->GetComponents<ComponentCollision>();
		for(auto col : cols)
		{
			if(col->GetCollisionType() == ComponentCollision::CollisionType::MODEL)
			{
				last_objs.push_back(obj);
				find = true;
				break;
			}
		}
		if(!find)
			objs.push_back(obj);
	}
	std::copy(last_objs.begin(), last_objs.end(), std::back_inserter(objs));

	for(int obj_index = 0; obj_index < obj_num; obj_index++)
	{
		// 調べるオブジェクト
		auto obj = objs[obj_index];

		// コンポーネントコリジョン群の取得
		auto cols = obj->GetComponents<ComponentCollision>();

		ComponentPtrVec cmps1;
		if(obj->GetStatus(::Object::StatusBit::OnHitAllComponent))
			cmps1 = obj->GetComponents<Component>();

		// コリジョンの検査
		for(auto col_1 : cols)
		{
			if(!col_1->GetStatus(Component::StatusBit::Enable))
				continue;

			// 相手オブジェクトの取得
			for(int other_index = obj_index + 1; other_index < obj_num; other_index++)
			{
				std::vector<ComponentCollision::HitInfo> hitInfos;

				// 相手オブジェクト
				auto obj2 = objs[other_index];

				auto			cols2 = obj2->GetComponents<ComponentCollision>();
				ComponentPtrVec cmps2;
				if(obj2->GetStatus(::Object::StatusBit::OnHitAllComponent))
					cmps2 = obj2->GetComponents<Component>();

				// 相手コリジョンの検査
				for(auto col_2 : cols2)
				{
					if(!col_1->IsGroupHit(col_2))
						continue;

					if(!col_2->GetStatus(Component::StatusBit::Enable))
						continue;

					// コリジョンどうしの当たりをチェックする
					ComponentCollision::HitInfo hitInfo;
					hitInfo = col_1->IsHit(col_2);

					if(hitInfo.hit_)
					{
						// 押し戻し量再計算
						float3 push{hitInfo.push_ * 0.5f};
						float3 other_push{-hitInfo.push_ * 0.5f};
						col_1->CalcPush(col_2, hitInfo.push_, &push, &other_push);

						// 相手か自分がオーバーラップする設定ならば押しあたりは発生しないようにする
						// オーバーラップする場合は当たりをすり抜ける
						if(col_1->IsOverlap(col_2->GetCollisionGroup()))
						{
							push	   = {0, 0, 0};
							other_push = {0, 0, 0};
						}

						// 相手もオーバーラップする場合は当たりをすり抜ける
						if(col_2->IsOverlap(col_1->GetCollisionGroup()))
						{
							push	   = {0, 0, 0};
							other_push = {0, 0, 0};
						}

						hitInfo.collision_	   = col_1;
						hitInfo.hit_collision_ = col_2;
						hitInfo.push_		   = push;
						col_1->OnHit(hitInfo);
						if(col_1->OnHitFunc)
							col_1->OnHitFunc(hitInfo);

						for(auto cmp : cmps1)
						{
							if(std::dynamic_pointer_cast<ComponentCollision>(cmp))
								continue;
							if(std::dynamic_pointer_cast<ComponentPhysics>(cmp))
								continue;

							Component::HitInfo hit_info{
								.hit_			= true,
								.collision_		= col_1,
								.hit_collision_ = col_2,
							};
							hit_info.hit_position_ = hitInfo.hit_position_;
							cmp->OnHitComponent(hit_info);
							if(cmp->OnHitComponentFunc)
								cmp->OnHitComponentFunc(hit_info);
						}

						hitInfo.collision_	   = col_2;
						hitInfo.hit_collision_ = col_1;
						hitInfo.push_		   = other_push;
						col_2->OnHit(hitInfo);
						if(col_2->OnHitFunc)
							col_2->OnHitFunc(hitInfo);

						for(auto cmp : cmps2)
						{
							if(std::dynamic_pointer_cast<ComponentCollision>(cmp))
								continue;
							if(std::dynamic_pointer_cast<ComponentPhysics>(cmp))
								continue;

							Component::HitInfo hit_info{
								.hit_			= true,
								.collision_		= col_2,
								.hit_collision_ = col_1,
							};
							hit_info.hit_position_ = hitInfo.hit_position_;
							cmp->OnHitComponent(hit_info);
							if(cmp->OnHitComponentFunc)
								cmp->OnHitComponentFunc(hit_info);
						}
					}
				}
			}
		}
	}
}

//! セレクトしているオブジェクトかをチェックする
bool Scene::SelectObjectWindow(const ObjectPtr& object)
{
	auto obj = selectObject.lock();
	if(obj != nullptr)
	{
		auto sel_name = obj->GetName();
		auto my_name  = object->GetName();
		if(object->GetName() == obj->GetName())
		{
			selectObject.reset();
			return true;
		}
	}
	return false;
}

ObjectPtr Scene::SelectObjectPtr()
{
	if(current_scene_ == nullptr)
		return nullptr;

	int size = (int)current_scene_->GetObjectPtrVec().size();

	if(size <= select_object_index)
		return nullptr;

	auto Object = current_scene_->GetObjectPtrVec()[select_object_index];
	return Object;
}

void Scene::SetGUIObjectDetailSize()
{
	auto ims		   = ImGui::GetWindowSize();
	object_detail_size = {ims.x, ims.y};
}

void Scene::SetGUIWindow(int posx, int posy, int width, int height)
{
	if(Scene::GetEditorStatus(Scene::EditorStatusBit::EditorPlacement))
	{
		auto mode = ImGuiCond_Once;
		if(Scene::GetEditorStatus(Scene::EditorStatusBit::EditorPlacement_Always))
			mode = ImGuiCond_Always;

		int game_width = 0, game_height = 0;
		int game_posx = 0, game_posy = 0;
		GetWindowSize(&game_width, &game_height);
		GetWindowPosition(&game_posx, &game_posy);

		if(posx == SET_GUI_ATTACH_LEFT)
		{
			posx = game_posx - width;
		}
		else if(posx == SET_GUI_ATTACH_RIGHT)
		{
			posx = game_posx + game_width + 8;
		}
		else
		{
			posx += game_posx;
		}

		if(posy == SET_GUI_ATTACH_TOP)
		{
			posy = game_posy - height;
		}
		else if(posy == SET_GUI_ATTACH_BOTTOM)
		{
			posy = game_posy + game_height;
		}
		else
		{
			posy += game_posy;
		}

		ImGui::SetNextWindowPos(ImVec2((float)posx, (float)posy), mode);
		ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), mode);
	}
}

ObjectPtr Scene::PickObject(int x, int y)
{
	auto wcam = GetCurrentCamera();
	auto cam  = wcam.lock();
	if(cam == nullptr)
		return nullptr;

	ComponentCamera::CameraRay ray = cam->MousePositionRay(x, y);

	auto vec = normalize(ray.end - ray.start) * 5.0f;

	DrawCapsule3D(cast(ray.start + vec), cast(ray.end), 0.05f, 30, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);

	float	  min	  = FLT_MAX;
	ObjectPtr min_obj = nullptr;

	for(auto& obj : current_scene_->GetObjectPtrVec())
	{
		auto mdl = obj->GetComponent<ComponentModel>();
		if(mdl)
		{
			// パラメータをいれると当たりがなくなる現象が発生 (授業中にも確認できたもの)
			MV1SetupCollInfo(mdl->GetModel());	  //, -1, 8, 8, 8);
			MV1_COLL_RESULT_POLY result = MV1CollCheck_Line(mdl->GetModel(), -1, cast(ray.start), cast(ray.end));
			if(result.HitFlag)
			{
				float near_len = length(cast(result.HitPosition) - cam->GetPosition());
				if(near_len < min)
				{
					min_obj = obj;
					min		= near_len;
				}
			}
		}
	}
	return min_obj;
}

ComponentCameraWeakPtr Scene::GetCurrentCamera()
{
	// デバック中であればデバックカメラを返す
	if(DebugCamera::IsUse())
		return DebugCamera::GetCamera();

	// 通常のカレントカメラを返す
	return ComponentCamera::GetCurrentCamera();
}

ComponentCameraWeakPtr Scene::SetCurrentCamera(std::string_view name)
{
	if(auto obj = Object::Get<::Object>(name))
	{
		if(auto cam = obj->GetComponent<ComponentCamera>())
			cam->SetCurrentCamera();
	}

	// デバック中であればデバックカメラを返す
	if(DebugCamera::IsUse())
		return DebugCamera::GetCamera();

	// 通常のカレントカメラを返す
	return ComponentCamera::GetCurrentCamera();
}

//----------------------------------------------------------------------
//! @name Cereal セーブロード
//----------------------------------------------------------------------
//@{

void Scene::SaveEditor()
{
	HelperLib::File::CreateFolder(".\\data\\_save\\");
	std::string	  name = ".\\data\\_save\\Editor.txt";
	std::ofstream file(name);
	if(!file)
		return;

	// 存在するオブジェクトをセーブする
	{
		cereal::JSONOutputArchive o_archive(file);
		{
			o_archive(CEREAL_NVP(editor_status_.get()));
			o_archive(CEREAL_NVP(inspector_size));
			o_archive(CEREAL_NVP(object_detail_size));
		}
	}
	file.close();
}

void Scene::LoadEditor()
{
	std::string	  name = ".\\data\\_save\\Editor.txt";
	std::ifstream file(name);
	if(!file)
	{
		editor_status_.set(Scene::EditorStatusBit::EditorPlacement, true);
		editor_status_.set(Scene::EditorStatusBit::EditorPlacement_Always, true);
		return;
	}

	// 存在するオブジェクトをセーブする
	std::stringstream ss;
	{
		cereal::JSONInputArchive i_archive(file);
		{
			i_archive(CEREAL_NVP(editor_status_.get()));
			i_archive(CEREAL_NVP(inspector_size));
			i_archive(CEREAL_NVP(object_detail_size));
		}
	}
}

bool Scene::Save(std::string_view filename)
{
	return current_scene_->Save(filename);
}

bool Scene::Load(std::string_view filename)
{
	return current_scene_->Load(filename);
}

//----------------------------------------------------------------------------
// Doxygen Sceneマニュアル
//----------------------------------------------------------------------------
//! @page section_scene Sceneについて
//!	@li スマートポインタにて管理 (オブジェクト管理が楽)
//!	@li シーン切り替えが容易にできる
//!	@li GUIにて現状把握できる
//!
//!	@li 【簡易Scene使用方法】①シーンを用意する
//!	@code
//!	class SceneSample : public Scene::Base
//!	{
//!	public:
//!		std::string Name() override {   //< シーンに名前をつける必要があります
//!			return "Sample";            //< 別のシーンに同じ名前はを使用しないでください。
//!		}                               //< SceneXXXXのXXXXを名前にするとよいと思います。
//!
//!		bool Init() override
//!		{
//!			// Objectの初期化例
//!			auto obj = Scene::Object::Create<Object>();
//!			obj->AddComponent<ModelComponent>("Model.mv1");
//!			obj->AddComponent<CameraComponent>();
//!		}
//!		void Update(float delta) override
//!		{
//!			obj->Update(delta);
//!
//!			// シーンが始まって10秒以上たったらSceneNextへ
//!			if (Scene::GetTime() > 10.0f)
//!			   Scene::Change( Scene::GetScene<SceneNext>() );
//!		}
//!		void Draw() override
//!		{
//!			obj->Draw();
//!		}
//!		void Exit() override
//!		{
//!		}
//!		void GUI() override
//!		{
//!
//!		}
//!	};
//!	@endcode
//!
//!	@li 【簡易Scene使用方法】②シーンを切り替える (初期はGameMainで切り替えます)
//!	@code
//!	// 例: シーンを「SceneSample」に変更します
//!	  Scene::Change( Scene::GetScene<SceneSample>() );
//!
//!	@endcode

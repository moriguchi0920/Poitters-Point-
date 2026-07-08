#include "SceneTestProc.h"
#include <System/Component/ComponentModel.h>

bool SceneTestProc::Init()
{
    auto obj = Scene::Object::Create<Object>()->SetName("ObjProc");
    auto mdl = obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1")->SetScaleAxisXYZ({0.05f});

    return true;
}

//---------------------------------------------------------------
// どのシーン移行してもセーブ可能なプロセス関数
// lamdaのようにキャプチャが使用できないため自らも取得する必要がある
// clang-format off

// Drawなど全般に利用できるプロセス
ProcAddProc(
	DrawTest, (){ printfDx( "DrawTest!\n" ); } 
);

// Updateに利用できるプロセスの記述方法
ProcAddProc(
	UpdateTest, () {	
		auto obj = Scene::Object::Get<Object>( "ObjProc" ); 
		obj->AddRotationAxisXYZ( { 0, 1, 0 } ); 
		printfDx( "UpdateTest!\n" ); 
	} 
);
// clang-format on
//---------------------------------------------------------------

// 標準以外の処理はセーブ・ロードができないため、
// ラムダ式を追加する場合は、InitSerialize()を利用する必要がある
// ※シーンが同じオブジェクト以外はシリアライズできません。
void SceneTestProc::InitSerialize()
{
    // シーンにあるオブジェクトを取得
    auto obj = Scene::Object::Get<Object>();

    // Drawという名前で、Drawのタイミングにて処理追加
    obj->SetProc("Draw", []() { printfDx("Draw!\n"); });

    // セーブ可能関数を使ったプロセス追加
    obj->SetAddProc(DrawTest);
    obj->SetAddProc(UpdateTest);

    // Updateという名前で、Updateのタイミングにて処理追加 (引数をfloatにするとUpdateに追加)
    obj->SetProc("Update", []() { printfDx("Update!\n"); });

    // ABCという名前で、PreDrawのタイミングに処理追加
    obj->SetProc("ABC", []() { printfDx("PreDraw!\n"); }, ProcTiming::PreDraw);

    // Testという名前で、Updateタイミングに追加 (優先付き)
    obj->SetProc("Test", []() { printfDx("Test!\n"); }, ProcTiming::Update, ProcPriority::NORMAL);

    // ライトのタイミングでプロセスを発生 (今回追加した Lightタイミング )
    // ※ Light/HDR/GBuffer/Shadow は、現在適当なタイミングで用意しています
    // Scene::Draw内の　プロセスシグナルの実行　にて実行
    // ※ 調整が必要となります
    obj->SetProc("CDE", []() { printfDx("Light\n"); }, ProcTiming::Light, ProcPriority::NORMAL);

    // コンポーネントに処理をつける
    if(auto mdl = obj->GetComponent<ComponentModel>())
        mdl->SetProc("CompUpdate", []() { printfDx("CompUpdate!\n"); });

    // 注意点は、lamdaに投げるオブジェクトはweak_ptrにする必要がある。
    // 使用しない場合、最後までCaptureし続け、DxLib_End内でエラーとなる
    std::weak_ptr<Object> wkobj = obj;

    // Updateを違う処理に変更します
    obj->SetProc("Update", [wkobj]() {
        printfDx("Update!\n");
        if(auto obj = wkobj.lock())
            obj->AddRotationAxisXYZ({0, 1, 0});
    });

    // デフォルトのPreUpdate()の優先を変更することもできます (obj::PreUpdate()の処理優先を変更しています)
    Scene::GetCurrentScene()->SetPriority(obj, ProcTiming::PreUpdate, ProcPriority::LOWEST);
}

void SceneTestProc::Update()
{
    // カメラの設定
    SetCameraPositionAndTarget_UpVecY({0.f, 6.f, -35.f}, {0.f, 1.f, 0.f});
    SetupCamera_Perspective(60.0f * DegToRad);

    // スペースを押したら次に行く
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        auto obj = Scene::Object::Get<Object>();

        // 動作追加
        obj->SetProc("Update2", []() { printfDx("Update2! (Update Priority::Normal)\n"); });

        // 現在 Update(引数あり)と無しとでは名前共有ができていません。
        // 今後共有予定です。(LightタイミングのCDEは現在消しません)
        obj->SetProc("Update3", []() { printfDx("Update3! (Update Priority::Normal)\n"); }, ProcTiming::Update, ProcPriority::NORMAL);

        obj->SetProc("Update4", []() { printfDx("Update4 (Update Priority::HIGHT)!\n"); }, ProcTiming::Update, ProcPriority::HIGH);

        // ABCのプロセスを終了します (PreDrawのものを終了)
        obj->ResetProc("ABC");

        // Updateを消します
        obj->ResetProc("Update");

        // コンポーネントの処理を削除する
        if(auto mdl = obj->GetComponent<ComponentModel>()) {
            mdl->ResetProc("CompUpdate");
        }
    }
    // 連続で変更
    if(IsKey(KEY_INPUT_RETURN)) {
        auto obj = Scene::Object::Get<Object>();

        ProcPriority up3 = ProcPriority::NORMAL;
        ProcPriority up4 = ProcPriority::NORMAL;

        static int count = 0;
        count++;
        if(count % 2 == 0) {
            up3 = ProcPriority::HIGH;
            up4 = ProcPriority::LOW;
        }
        else {
            up3 = ProcPriority::LOW;
            up4 = ProcPriority::HIGH;
        }

        obj->SetProc("Update3", [up3]() { printfDx("Update3! (Update %d)\n", (int)up3); }, ProcTiming::Update, up3);

        obj->SetProc("Update4", [up4]() { printfDx("Update4 (Update %d)!\n", (int)up4); }, ProcTiming::Update, up4);
    }
}

void SceneTestProc::Draw()
{
    // とりあえずTitleという文字を表示しておく
    DrawFormatString(100, 50, GetColor(255, 255, 255), "Title");
}

void SceneTestProc::Exit()
{
    // タイトル終了時に行いたいことは今はない
}

void SceneTestProc::GUI()
{
}

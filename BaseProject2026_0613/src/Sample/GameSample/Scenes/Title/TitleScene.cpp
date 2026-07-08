#include "TitleScene.h"
#include "../Game/GameScene.h"

namespace Sample::GameSample {

bool TitleScene::Init()
{
    return true;
}

void TitleScene::Update()
{
    if(IsKeyOn(KEY_INPUT_RETURN))
        Scene::Change(Scene::GetScene<GameScene>());
}

void TitleScene::Draw()
{
    // とりあえずTitleという文字を表示しておく
    DrawFormatString(100, 50, GetColor(255, 255, 255), "Title");
}

void TitleScene::Exit()
{
    // タイトル終了時に行いたいことは今はない
}

void TitleScene::GUI()
{
}

}    // namespace Sample::GameSample

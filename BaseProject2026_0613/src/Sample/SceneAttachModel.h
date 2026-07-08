#pragma once

#include <System/Scene.h>

//! @brief シーン
class SceneAttachModel : public Scene::Base
{
public:
    // 継承している形をここでも書いておく(プログラム的に使用する)
    BP_CLASS_DECL(SceneAttachModel, u8"AttachModel 使用サンプル");

    //! @brief シーン初期化関数を継承します
    //! @return シーン初期化が終わったらtrueを返します
    bool Init() override;

    //! @brief シーン更新関数。ディスプレイリフレッシュレートに合わせて実行されます
    //! @param delta 1秒をベースとした1フレームの数値
    //! @detial deltaは、リフレッシュレートが違うと速度が変わってしまう部分を吸収するためにある
    void Update() override;

    //! @brief 終了処理
    void Exit() override;

private:
#if 0
	struct Material
	{
		std::shared_ptr<Texture> albedo_;
		std::shared_ptr<Texture> normal_;
		std::shared_ptr<Texture> roughness_;
		std::shared_ptr<Texture> metalness_;
	};

	std::vector<Material> materials_;
#endif
};

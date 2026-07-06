//---------------------------------------------------------------------------
//!	@file	GuiMenu.cpp
//! @brief	メニューGUI
//---------------------------------------------------------------------------
#include <System/GuiMenu.h>
#include <System/Debug/DebugCamera.h>

namespace gui
{

	//---------------------------------------------------------------------------
	// GUI表示
	//---------------------------------------------------------------------------
	void GuiMenuScene::GUI()
	{
		struct Group
		{
			std::string_view			 filter_name_;	  //!< フィルター文字列
			std::string					 menu_name_;	  //!< メニュー表示名
			std::vector<const TypeInfo*> type_infos_;	  //!< グループに属する型リスト
		};

		static Group groups[] = {
			{ "Tutorial",	 u8"Tutorial / チュートリアル"},
			{"SceneTest", u8"SceneTest / 使用方法サンプル"},
		};

		static std::vector<const TypeInfo*> scene_types;

		//----------------------------------------------------------
		// 初回初期化
		//----------------------------------------------------------
		if(scene_types.empty())
		{
			//------------------------------------------
			// 登録されているシーンを列挙する
			//------------------------------------------
			auto& scene_base = Scene::Base::Type;

			// SceneBaseを継承する型情報リストを取得
			for(auto* type_info = scene_base.child(); type_info; type_info = type_info->siblings())
			{
				scene_types.emplace_back(type_info);
			}

			// クラス名のアルファベット順にソート
			{
				auto predicate = [](const TypeInfo* a, const TypeInfo* b)
				{ return strcmp(a->className(), b->className()) < 0; };

				std::sort(scene_types.begin(), scene_types.end(), predicate);
			}

			//------------------------------------------------------
			// フィルターに該当するクラスを振り分け
			//------------------------------------------------------
			for(u32 i = 0; i < scene_types.size(); ++i)
			{
				auto& type_info = scene_types[i];

				for(auto& group : groups)
				{
					// フィルター名に一致しなければスキップ
					if(std::strncmp(group.filter_name_.data(), type_info->className(), group.filter_name_.size()))
						continue;

					// グループに登録してメインのリストから抜く
					group.type_infos_.emplace_back(std::move(type_info));
					scene_types.erase(scene_types.begin() + i);	   // リストからは削除
					--i;										   // もう一度同じ場所を処理する
					break;
				}
			}
		}

		//----------------------------------------------------------
		// クラスをメニューに登録する関数
		//----------------------------------------------------------
		auto enum_classes = [](std::vector<const TypeInfo*> type_infos)
		{
			auto& types = type_infos;

			// リスト中の最大文字数を計算
			size_t max_class_name_length = 0;
			for(auto& type_info : types)
			{
				max_class_name_length = std::max(max_class_name_length, std::strlen(type_info->className()));
			}

			// メニュー登録
			for(auto& type_info : types)
			{
				// 表示文字列
				std::string menu_name = type_info->className();

				// 余白空白を追加
				for(u32 i = 0; i < max_class_name_length - std::strlen(type_info->className()); ++i)
				{
					menu_name += " ";
				}

				// 説明文を追加
				menu_name += " - ";
				menu_name += type_info->descName();

				// 現在のシーンなら選択状態にする
				bool selected = false;
				if(auto* current_scene = Scene::GetCurrentScene())
				{
					selected = (current_scene->typeInfo() == type_info);
				}

				if(ImGui::MenuItem(menu_name.c_str(), nullptr, selected))
				{
					if(!selected)
					{	 // 現在のシーン以外が新たに選択された場合
						auto* scene = reinterpret_cast<Scene::Base*>(type_info->createInstance());
						// シーンジャンプ
						if(scene)
						{
							// デバッグカメラはOFFにしてからシーン切り替えを行います
							DebugCamera::Use(false);
							Scene::Change(std::shared_ptr<Scene::Base>(scene));
						}
					}
				}
			}
		};

		if(ImGui::BeginMenu(u8"シーン選択"))
		{
			//------------------------------------------------------
			// グループを先に表示
			//------------------------------------------------------
			for(auto& group : groups)
			{
				if(!ImGui::BeginMenu(group.menu_name_.c_str()))
					continue;

				enum_classes(group.type_infos_);

				ImGui::EndMenu();
			}

			//------------------------------------------------------
			// メニュー登録
			//------------------------------------------------------
			enum_classes(scene_types);

			ImGui::EndMenu();
		}
		if(ImGui::Button(u8"シーンリセット"))
		{
			auto inst = reinterpret_cast<Scene::Base*>(Scene::GetCurrentScene()->typeInfo()->createInstance());
			Scene::GetCurrentScene()->Exit();
			Scene::Change(std::shared_ptr<Scene::Base>(inst));
		}
	}

}	 // namespace gui

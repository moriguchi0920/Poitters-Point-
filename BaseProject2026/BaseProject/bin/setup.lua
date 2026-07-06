--============================================================================
-- オプション設定の追加
--============================================================================
newoption {
   trigger     = "output-path",
   description = "Specifies the output path of the project file."
}

--------------------------------------------------------------------
-- プロジェクトフォルダ出力先のパスを取得
--------------------------------------------------------------------
function build_path(name)
	-- デフォルトはルートフォルダ(一階層上)
	local build_root_path = path.join("..", name);	-- デフォルト値 "../.build"
	return build_root_path
end

--============================================================================
-- ソリューションファイル設定
--============================================================================
function config_solution(name)
	filter {}

	workspace(name)
    configurations { "Debug", "Release", "ReleaseLTCG" }
    platforms { "x64" }
    location (".")			-- slnファイルの生成場所


	filter "platforms:x64"
	    system "Windows"
	    architecture "x64"

	filter {}	-- 元に戻す
end

--============================================================================
-- プロジェクト共通設定
--============================================================================
function config_project(name, kind_type)
	filter {}
	project(name)

    kind (kind_type)
    language "C++"
    uuid (os.uuid(name))

    location  (path.join(build_path(".build"), name))												-- slnファイルの生成場所
    targetdir (path.join(build_path(".build"), name, "exe", "%{cfg.platform}","%{cfg.buildcfg}"))	-- exeの生成場所
    objdir    (path.join(build_path(".build"), name, "obj"))										-- objファイルの生成場所

	entrypoint "WinMainCRTStartup"

	-- 各種オプション設定
	cppdialect        "C++latest"	-- C++最新準拠
	rtti              "On"			-- ランタイム方情報を有効にする	
	exceptionhandling "On"			-- 例外を有効にする
	warnings          "Extra"		-- 最大警告レベル
    floatingpoint     "Strict"		-- fastmath
	characterset      "MBCS"		-- マルチバイト文字セット
	intrinsics        "On"			-- 組み込み関数を使用する
	symbols           "FastLink"	-- 高速リンク
	staticruntime     "On"			-- ランタイムにDLLを使用しない
	multiprocessorcompile "On"		-- 複数プロセッサによるコンパイル

	vectorextensions  "sse2"		-- SSE2サポート
--	vectorextensions  "avx"			-- AVXサポート
    systemversion     "latest"		-- WindowsSDK最新版を使用
	
	-- リンカーの追加オプション
	linkoptions {
		"/IGNORE:4099"
	}
  
	-- プリプロセッサ #define
    defines {
	--	"_CRT_SECURE_NO_WARNINGS",
		"WIN64",
		"WIN32",
		"_WIN32",
		"_WINDOWS"
	}
	filter { "kind:StaticLib" }
		defines { "_LIB" }
	filter { "kind:SharedLib" }
		defines { "_DLL" }
	filter { "kind:WindowedApp" }
	filter { "kind:ConsoleApp" }
		defines { "_CONSOLE" }
	filter {}

	-- 特定の警告の無効化
	disablewarnings {
		"4324",		-- warning C4324: アラインメント指定子のために構造体がパッドされました
		"4201",		-- warning C4201: 非標準の拡張機能が使用されています: 無名の構造体または共用体です。
		"5054",		-- warning C5054: 演算子 '|': 異なる型の列挙間では非推奨です (Cerealで使用されています)
	}

	--============================================================================
	-- デバッグ(Debug) ビルド
	--============================================================================
	filter "configurations:Debug"
	    defines {
			"_DEBUG",
			"DEBUG",
			"_WINDOWS"
		}

	    optimize             "Off"
		omitframepointer     "Off"
		inlining             "Explicit"
		functionlevellinking "On"
		editandcontinue      "Off"
		runtimechecks        "Default"
		
		buildoptions {
			"/Zo",	-- 最適化されたデバッグ機能の強化
			"/Zc:char8_t-", -- char8_tは使用しない
		}
		
	--============================================================================
	-- リリース(Release) ビルド
	--============================================================================
	filter "configurations:Release*"
		runtimechecks        "Off"

	    defines {
			"NDEBUG",
		}
		buildoptions {
			"/GT",	-- ファイバー保護の最適化
		}
		
	-- Release
	filter "configurations:Release"
	    optimize             "Speed"
		omitframepointer     "Off"
		inlining             "Explicit"
		functionlevellinking "Off"

	    defines {
			"PROFILE",
		}
		buildoptions {
			"/Zo",	-- 最適化されたデバッグ機能の強化
			"/Zc:char8_t-", -- char8_tは使用しない
		}

	-- ReleaseLTCG
	filter "configurations:ReleaseLTCG"
	    optimize             "Full"
		omitframepointer     "On"
		inlining             "Auto"
		functionlevellinking "On"
		linktimeoptimization "On"

	    defines {
			"LTCG",
		}
		buildoptions {
			"/Zc:char8_t-", -- char8_tは使用しない
		}

	filter {}	-- 元に戻す
end

--============================================================================
-- 外部のプロジェクトファイル
--============================================================================
function external_project(name, path)
	externalproject (name)
	location (path)
	uuid(os.uuid(name))
   kind "StaticLib"
   language "C++"
end

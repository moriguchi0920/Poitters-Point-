//---------------------------------------------------------------------------
//! @file   IniFileLib.h
//! @brief  Iniファイルライブラリ
//---------------------------------------------------------------------------
#pragma once

#include <System/Utils/HelperLib.h>
#include <string>

bool CheckIsNum(const std::string& check_string);

bool CheckIsNumForMultVar(const std::vector<std::string>& check_vars);

class IniFileLib
{
public:
    IniFileLib() {}

    virtual ~IniFileLib() {}

    //----------------------------------------------------------------------------
    //! @brief Iniファイル初期設定
    //! @param name ファイル名
    //----------------------------------------------------------------------------
    IniFileLib(const std::string& name) { File(name); }

    //! Iniファイルロード

    //----------------------------------------------------------------------------
    //! @brief Iniファイル名の設定
    //! @param name ファイル名
    //! @return ファイル名の存在
    //----------------------------------------------------------------------------
    bool File(const std::string& name)
    {
        file_ = "data/" + name;
        return HelperLib::File::CheckFileExistence(file_);
    }

    //! 整数値取得

    //----------------------------------------------------------------------------
    //! @brief 整数を取得します
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    const int GetInt(const std::string& section, const std::string& key, int def = 0) const;

    //----------------------------------------------------------------------------
    //! @brief 浮動小数点数取得
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    const float GetFloat(const std::string& section, const std::string& key, float def = 0.0f) const;

    //----------------------------------------------------------------------------
    //! @brief 浮動小数点数取得(2要素)
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    const float2 GetFloat2(const std::string& section, const std::string& key, float2 def = {0.0f, 0.0f}) const;

    //----------------------------------------------------------------------------
    //! @brief 浮動小数点数取得(3要素)
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    const float3 GetFloat3(const std::string& section, const std::string& key, float3 def = {0, 0, 0}) const;

    //----------------------------------------------------------------------------
    //! @brief 浮動小数点数取得(4要素)
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    const float4 GetFloat4(const std::string& section, const std::string& key, float4 def = {0, 0, 0, 0}) const;

    //----------------------------------------------------------------------------
    //! @brief 文字列の取得
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 文字列
    //----------------------------------------------------------------------------
    const std::string GetString(const std::string& section, const std::string& key, std::string def = "") const;

    //----------------------------------------------------------------------------
    //! @brief 文字列の取得
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 文字列
    //----------------------------------------------------------------------------
    const std::vector<std::string> GetStrings(const std::string& section, const std::string& key) const;

    //----------------------------------------------------------------------------
    //! @brief 文字列の取得
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 文字列
    //----------------------------------------------------------------------------
    const bool GetBool(const std::string& section, const std::string& key, bool def = false) const;

private:
    //----------------------------------------------------------------------------
    //! @brief 文字列をbuffer_に読み込む
    //! @param section セッション
    //! @param key キー
    //! @param def デフォルト
    //----------------------------------------------------------------------------
    void ReadBuffer(LPCTSTR section, LPCTSTR key, LPCTSTR def) const;

    std::string                   file_{};
    mutable std::array<char, 256> buffer_{};
};

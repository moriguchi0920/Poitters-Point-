//---------------------------------------------------------------------------
//! @file   IniFileLib.h
//! @brief  Iniファイルライブラリ
//---------------------------------------------------------------------------
#pragma once

#include "IniFileLib.h"

//! 要素が数値であるか

//----------------------------------------------------------------------------
//! @brief stringの要素が全て数値であるかどうかチェックします
//! @param check_string チェックするstring型のデータ
//! @return 全て数値であるかどうか
//----------------------------------------------------------------------------

bool CheckIsNum(const std::string& check_string)
{
	// 全ての要素の文字コードが'0'以上'9'以下であるかどうかを返す
	return std::all_of(check_string.begin(), check_string.end(), [&check_string](auto c)
					   { return (c >= '0' && c <= '9'); });
}

//----------------------------------------------------------------------------
//! @brief stringの要素が全てFloatで表現できる数値であるかどうかチェックします
//! @param check_string チェックするstring型のデータ
//! @return 全て数値であるかどうか
//----------------------------------------------------------------------------
bool CheckIsFloat(const std::string& check_string)
{
	try
	{
		size_t pos;
		float  f = std::stof(check_string, &pos);
		f;
		return pos == check_string.size();	  // 全て変換できたか確認
	}
	catch(...)
	{
		return false;
	}
}

//! 要素(複数)が数値であるか
//----------------------------------------------------------------------------
//! @brief 各stringの要素が全て数値であるかどうかチェックします
//! @param check_vars チェックするstring型のデータが入ったvector配列
//! @return 全て数値であるかどうか
//----------------------------------------------------------------------------
bool CheckIsNumForMultVar(const std::vector<std::string>& check_vars)
{
	return std::all_of(check_vars.begin(), check_vars.end(), [&check_vars](auto s)
					   { return (CheckIsNum(s)); });
}

//----------------------------------------------------------------------------
//! @brief 各stringの要素が全て数値であるかどうかチェックします
//! @param check_vars チェックするstring型のデータが入ったvector配列
//! @return 全て数値であるかどうか
//----------------------------------------------------------------------------
bool CheckIsFloatForMultVar(const std::vector<std::string>& check_vars)
{
	return std::all_of(check_vars.begin(), check_vars.end(), [&check_vars](auto s)
					   { return (CheckIsFloat(s)); });
}

//! 整数値取得

//----------------------------------------------------------------------------
//! @brief 整数を取得します
//! @param section セクション名
//! @param key キー名
//! @param def デフォルト数値(ないとき選ばれる)
//! @return 数値
//----------------------------------------------------------------------------

const int IniFileLib::GetInt(const std::string& section, const std::string& key, int def) const
{
	ReadBuffer(section.c_str(), key.c_str(), "None");
	auto str = std::string(buffer_.data());

	if(str == "None")
		return def;

	str = HelperLib::String::Trim(str, "{}; ");

	if(!CheckIsNum(str))
		return def;

	return stoi(str);
}

//----------------------------------------------------------------------------
//! @brief 浮動小数点数取得
//! @param section セクション名
//! @param key キー名
//! @param def デフォルト数値(ないとき選ばれる)
//! @return 数値
//----------------------------------------------------------------------------

const float IniFileLib::GetFloat(const std::string& section, const std::string& key, float def) const
{
	ReadBuffer(section.c_str(), key.c_str(), "None");
	auto str = std::string(buffer_.data());

	if(str == "None")
		return def;

	str = HelperLib::String::Trim(str, "; ");

	if(!CheckIsNum(str))
		return def;

	return stof(str);
}

//----------------------------------------------------------------------------
//! @brief 浮動小数点数取得(2要素)
//! @param section セクション名
//! @param key キー名
//! @param def デフォルト数値(ないとき選ばれる)
//! @return 数値
//----------------------------------------------------------------------------

const float2 IniFileLib::GetFloat2(const std::string& section, const std::string& key, float2 def) const
{
	ReadBuffer(section.c_str(), key.c_str(), "None");
	auto str = std::string(buffer_.data());

	if(str == "None")
		return def;

	str		= HelperLib::String::Trim(str, "{}; ");
	auto v2 = HelperLib::String::Split(str, ',');

	if(!CheckIsFloatForMultVar(v2))
		return def;

	return float2(stof(v2[0]), stof(v2[1]));
}

//----------------------------------------------------------------------------
//! @brief 浮動小数点数取得(3要素)
//! @param section セクション名
//! @param key キー名
//! @param def デフォルト数値(ないとき選ばれる)
//! @return 数値
//----------------------------------------------------------------------------

const float3 IniFileLib::GetFloat3(const std::string& section, const std::string& key, float3 def) const
{
	ReadBuffer(section.c_str(), key.c_str(), "None");
	auto str = std::string(buffer_.data());

	if(str == "None")
		return def;

	str		= HelperLib::String::Trim(str, "{}; ");
	auto v2 = HelperLib::String::Split(str, ',');

	if(!CheckIsFloatForMultVar(v2))
		return def;

	return float3(stof(v2[0]), stof(v2[1]), stof(v2[2]));
}

//----------------------------------------------------------------------------
//! @brief 浮動小数点数取得(4要素)
//! @param section セクション名
//! @param key キー名
//! @param def デフォルト数値(ないとき選ばれる)
//! @return 数値
//----------------------------------------------------------------------------

const float4 IniFileLib::GetFloat4(const std::string& section, const std::string& key, float4 def) const
{
	ReadBuffer(section.c_str(), key.c_str(), "None");
	auto str = std::string(buffer_.data());

	if(str == "None")
		return def;

	str		= HelperLib::String::Trim(str, "{}; ");
	auto v2 = HelperLib::String::Split(str, ',');

	if(!CheckIsFloatForMultVar(v2))
		return def;

	return float4(stof(v2[0]), stof(v2[1]), stof(v2[2]), stof(v2[3]));
}

//----------------------------------------------------------------------------
//! @brief 文字列の取得
//! @param section セクション名
//! @param key キー名
//! @param def デフォルト数値(ないとき選ばれる)
//! @return 文字列
//----------------------------------------------------------------------------

const std::string IniFileLib::GetString(const std::string& section, const std::string& key, std::string def) const
{
	ReadBuffer(section.c_str(), key.c_str(), "None");
	auto str = std::string(buffer_.data());

	if(str == "None")
		return def;

	str = HelperLib::String::Trim(str, "\";");

	return str;
}

//----------------------------------------------------------------------------
//! @brief 文字列の取得
//! @param section セクション名
//! @param key キー名
//! @param def デフォルト数値(ないとき選ばれる)
//! @return 文字列
//----------------------------------------------------------------------------

const std::vector<std::string> IniFileLib::GetStrings(const std::string& section, const std::string& key) const
{
	ReadBuffer(section.c_str(), key.c_str(), "None");
	const auto str = std::string(buffer_.data());

	if(str == "None")
		return std::vector<std::string>();

	return HelperLib::String::Split(str, ',');
}

//----------------------------------------------------------------------------
//! @brief 文字列の取得
//! @param section セクション名
//! @param key キー名
//! @param def デフォルト数値(ないとき選ばれる)
//! @return 文字列
//----------------------------------------------------------------------------

const bool IniFileLib::GetBool(const std::string& section, const std::string& key, bool def) const
{
	ReadBuffer(section.c_str(), key.c_str(), "None");
	auto str = std::string(buffer_.data());

	if(str == "None")
		return def;

	str = HelperLib::String::Trim(str, "\"; ");
	if(str == "0")
		return false;
	if(str == "1")
		return true;
	if(str == "False")
		return false;
	if(str == "True")
		return true;
	if(str == "false")
		return false;
	if(str == "true")
		return true;

	// 異なる文字などの場合はTrueとする
	return true;
}

//----------------------------------------------------------------------------
//! @brief 文字列をbuffer_に読み込む
//! @param section セッション
//! @param key キー
//! @param def デフォルト
//----------------------------------------------------------------------------

void IniFileLib::ReadBuffer(LPCTSTR section, LPCTSTR key, LPCTSTR def) const
{
	GetPrivateProfileStringA(section, key, def, buffer_.data(), (DWORD)buffer_.size(), file_.c_str());
}

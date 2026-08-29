//----------------------------------------------------------------------------
//!	@file	ps_model.fx
//!	@brief	MV1モデルピクセルシェーダー
//----------------------------------------------------------------------------
#include "dxlib_ps.h"

// 頂点シェーダーの出力
struct VS_OUTPUT_MODEL
{
	float4	position_       : SV_Position;      //!< 座標       (スクリーン空間)
    float4  curr_position_  : CURR_POSITION;    //!< 現在の座標 (スクリーン空間)
	float3	world_position_ : WORLD_POSITION;   //!< ワールド座標
	float3	normal_         : NORMAL0;          //!< 法線
	float4	diffuse_        : COLOR0;           //!< Diffuseカラー
	float2	uv0_            : TEXCOORD0;        //!< テクスチャ座標
    float4  prev_position_  : PREV_POSITION;    //!< 1フレーム前の座標 (スクリーン空間) ※末尾に追加されているため注意
};

typedef	VS_OUTPUT_MODEL	PS_INPUT_MODEL;

//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT_MODEL input)
{
	PS_OUTPUT	output;

	float2	uv = input.uv0_;
	float3	N = normalize(input.normal_);	// 法線

	//------------------------------------------------------------
	// 法線マップ
	//------------------------------------------------------------
	N = Normalmap(N, input.world_position_, uv);

	//------------------------------------------------------------
	// テクスチャカラーを読み込み
	//------------------------------------------------------------
	float4	textureColor = DiffuseTexture.Sample(DiffuseSampler, uv);

    // アルファテスト
	if(textureColor.a < 0.5) discard;

	//これ元のやつ
	//output.color0_ = textureColor; // * input.diffuse_;
	
	//  丸太（テクスチャが無くて色情報を持っているモデル）だけを特定する判定 
	// 1. テクスチャが白（テクスチャなし）
	// 2. input.diffuse_ のアルファ（透明度）が 0 より大きい（色が設定されている）
    bool isNoTexture = (textureColor.r >= 0.99f && textureColor.g >= 0.99f && textureColor.b >= 0.99f);
    bool hasVertexColor = (input.diffuse_.a > 0.0f && (input.diffuse_.r > 0.0f || input.diffuse_.g > 0.0f || input.diffuse_.b > 0.0f));

    if (isNoTexture && hasVertexColor)
    {
		// 【丸太限定】モデル自身の色（input.diffuse_）を使って描画
        output.color0_ = input.diffuse_;
    }
    else
    {
		// 【その他のテクスチャありモデル】通常通りテクスチャの色を描画（真っ黒にならない！）
        output.color0_ = textureColor;
    }

	// 出力パラメータを返す
	return output;
}

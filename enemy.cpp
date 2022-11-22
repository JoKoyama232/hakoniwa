//=============================================================================
//
// エネミーモデル処理 [enemy.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"

#include "input.h"
#include "debugproc.h"
#include "enemy.h"
#include "shadow.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/enemy.obj"		// 読み込むモデル名
#define MODEL_GUN			"data/MODEL/gun.obj"
#define MODEL_SHEILD		"data/MODEL/shield.obj"
#define MODEL_FUNNEL		"data/MODEL/FinFunnel_s.obj"
#define MODEL_ARIEL			"data/MODEL/Aerial.obj"
#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define ENEMY_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define ENEMY_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる

enum {
	aerial = 0,
	funnel,
	shield,
	gun,
};
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// エネミー

int g_Enemy_load = 0;

static INTERPOLATION_DATAPOINT g_moveList[] = {
	//座標								回転率							拡大率							時間
	{ XMFLOAT3( 0.0f,   7.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		30 },
	{ XMFLOAT3(100.0f,  7.0f, 100.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		30 },
	{ XMFLOAT3(50.0f,  7.0f, 100.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		30 },
};

static INTERPOLATION_DATAPOINT* g_MoveListMngr[] =
{
	g_moveList,
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		switch (i) {
		case 0:
			LoadModel(MODEL_ARIEL, &g_Enemy[i].model);
			break;
		case 1:
			LoadModel(MODEL_FUNNEL, &g_Enemy[i].model);
			break;
		case 2:
			LoadModel(MODEL_SHEILD, &g_Enemy[i].model);
			break;
		case 3:
			LoadModel(MODEL_GUN, &g_Enemy[i].model);
			break;
		default:
			LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
			break;

		}
		
		g_Enemy[i].load = true;

		g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, 50.0f, 20.0f);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Enemy[i].spd  = 0.0f;			// 移動スピードクリア
		g_Enemy[i].size = ENEMY_SIZE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);
		
		g_Enemy[0].time = 0.0f;		// 線形補間用のタイマーをクリア
		g_Enemy[0].tblNo = 0;		// 再生するアニメデータの先頭アドレスをセット
		g_Enemy[0].tblMax = sizeof(g_moveList) / sizeof(INTERPOLATION_DATAPOINT);	// 再生するアニメデータのレコード数をセット

		g_Enemy[i].use = true;		// true:生きてる

	}

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = false;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	// エネミーを動かす場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == true)		// このエネミーが使われている？
		{								// Yes

			switch (i) {
			case aerial:
				g_Enemy[i].pos = XMFLOAT3(100.0f, -100.0f, 10.0f);
				g_Enemy[i].rot = XMFLOAT3(-3.14f/4, +3.14f / 4, 0.0f);
				g_Enemy[i].scl = XMFLOAT3(6.0f, 6.0f, 6.0f);
				break;
			case funnel:
				g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, 50.0f, 20.0f);
				g_Enemy[i].rot = XMFLOAT3(3.14f/2, 0.0f, 3.14f);
				g_Enemy[i].scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
				break;
			case shield:
				g_Enemy[i].pos = XMFLOAT3(220, 25.0f, 50.0f);
				g_Enemy[i].rot = XMFLOAT3(-3.14f / 4, +3.14f / 3, 0.0f);
				g_Enemy[i].scl = XMFLOAT3(5.0f, 3.0f, 3.0f);
				break;
			case gun:
				g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, 50.0f, 20.0f);
				g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
				g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
				break;
			default:
				g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, 50.0f, 20.0f);
				g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
				g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
				break;

			}

	
		}
	}




#ifdef _DEBUG

	if (GetKeyboardTrigger(DIK_P))
	{
		// モデルの色を変更できるよ！半透明にもできるよ。
		for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Enemy[0].model, j, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));
		}
	}

	if (GetKeyboardTrigger(DIK_L))
	{
		// モデルの色を元に戻している
		for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Enemy[0].model, j, g_Enemy[0].diffuse[j]);
		}
	}
#endif


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == false) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_Enemy[i].model);
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}

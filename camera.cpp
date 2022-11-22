//=============================================================================
//
// カメラ処理 [camera.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "enemy.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	POS_X_CAM			(0.0f)			// カメラの初期位置(X座標)
#define	POS_Y_CAM			(50.0f)			// カメラの初期位置(Y座標)
#define	POS_Z_CAM			(-140.0f)		// カメラの初期位置(Z座標)

//#define	POS_X_CAM		(0.0f)			// カメラの初期位置(X座標)
//#define	POS_Y_CAM		(200.0f)		// カメラの初期位置(Y座標)
//#define	POS_Z_CAM		(-400.0f)		// カメラの初期位置(Z座標)


#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(10000.0f)										// ビュー平面のFarZ値

#define	VALUE_MOVE_CAMERA	(2.0f)										// カメラの移動量
#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)								// カメラの回転量

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static CAMERA			g_Camera;		// カメラデータ

static int				g_ViewPortType = TYPE_FULL_SCREEN;

static INTERPOLATION_DATAPOINT g_moveList[] = {
	//座標								回転率							拡大率							時間
	{ XMFLOAT3(650.0f,   200.0f, -650.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		500},
	{ XMFLOAT3(-600.0f,   7.0f, -600.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		500 },
	{ XMFLOAT3(0.0f,   7.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		1 },
};

static INTERPOLATION_DATAPOINT* g_MoveListMngr[] =
{
	g_moveList,
};

//=============================================================================
// 初期化処理
//=============================================================================
void InitCamera(void)
{
	g_Camera.pos = { POS_X_CAM, POS_Y_CAM, POS_Z_CAM };
	g_Camera.at  = { 0.0f, 0.0f, 0.0f };
	g_Camera.up  = { 0.0f, 1.0f, 0.0f };
	g_Camera.rot = { 0.0f, 0.0f, 0.0f };

	// 視点と注視点の距離を計算
	float vx, vz;
	vx = g_Camera.pos.x - g_Camera.at.x;
	vz = g_Camera.pos.z - g_Camera.at.z;
	g_Camera.len = sqrtf(vx * vx + vz * vz);

	g_Camera.time = 0.0f;		// 線形補間用のタイマーをクリア
	g_Camera.tblNo = 0;		// 再生するアニメデータの先頭アドレスをセット
	g_Camera.tblMax = sizeof(g_moveList) / sizeof(INTERPOLATION_DATAPOINT);	// 再生するアニメデータのレコード数をセット
	// ビューポートタイプの初期化
	g_ViewPortType = TYPE_FULL_SCREEN;
}


//=============================================================================
// カメラの終了処理
//=============================================================================
void UninitCamera(void)
{

}


//=============================================================================
// カメラの更新処理
//=============================================================================
void UpdateCamera(void)
{

	int nowNo = (int)g_Camera.time;			// 整数分であるテーブル番号を取り出している
	int maxNo = g_Camera.tblMax;				// 登録テーブル数を数えている
	int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
	INTERPOLATION_DATAPOINT* tbl = g_MoveListMngr[g_Camera.tblNo];	// 行動テーブルのアドレスを取得

	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換

	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している

	float nowTime = g_Camera.time - nowNo;	// 時間部分である少数を取り出している

	Pos *= nowTime;								// 現在の移動量を計算している
	Rot *= nowTime;								// 現在の回転量を計算している
	Scl *= nowTime;								// 現在の拡大率を計算している

	// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
	XMStoreFloat3(&g_Camera.pos, nowPos + Pos);

	// 計算して求めた回転量を現在の移動テーブルに足している
	XMStoreFloat3(&g_Camera.rot, nowRot + Rot);

	// frameを使て時間経過処理をする
	g_Camera.time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
	if ((int)g_Camera.time >= maxNo)			// 登録テーブル最後まで移動したか？
	{
		g_Camera.time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
	}
}


//=============================================================================
// カメラの更新
//=============================================================================
void SetCamera(void) 
{
	ENEMY* enemy = GetEnemy();
	// ビューマトリックス設定
	XMMATRIX mtxView;
	int look = g_Camera.time + 1 % g_Camera.tblMax;
	XMFLOAT3 lookahead = g_moveList[look].pos;
	switch (look) {
	case 1:
		
		mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Camera.pos), XMLoadFloat3(&enemy[0].pos), XMLoadFloat3(&g_Camera.up));
		break;
	default:
		mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Camera.pos), XMLoadFloat3(&lookahead), XMLoadFloat3(&g_Camera.up));
		break;
	}
	
	
	SetViewMatrix(&mtxView);
	XMStoreFloat4x4(&g_Camera.mtxView, mtxView);

	XMMATRIX mtxInvView;
	mtxInvView = XMMatrixInverse(nullptr, mtxView);
	XMStoreFloat4x4(&g_Camera.mtxInvView, mtxInvView);


	// プロジェクションマトリックス設定
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

	SetProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&g_Camera.mtxProjection, mtxProjection);

	SetShaderCamera(g_Camera.pos);
}


//=============================================================================
// カメラの取得
//=============================================================================
CAMERA *GetCamera(void) 
{
	return &g_Camera;
}

//=============================================================================
// ビューポートの設定
//=============================================================================
void SetViewPort(int type)
{
	ID3D11DeviceContext *g_ImmediateContext = GetDeviceContext();
	D3D11_VIEWPORT vp;

	g_ViewPortType = type;

	// ビューポート設定
	switch (g_ViewPortType)
	{
	case TYPE_FULL_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_LEFT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_RIGHT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = (FLOAT)SCREEN_WIDTH / 2;
		vp.TopLeftY = 0;
		break;

	case TYPE_UP_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_DOWN_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = (FLOAT)SCREEN_HEIGHT / 2;
		break;


	}
	g_ImmediateContext->RSSetViewports(1, &vp);

}


int GetViewPortType(void)
{
	return g_ViewPortType;
}



// カメラの視点と注視点をセット
void SetCameraAT(XMFLOAT3 pos)
{
	// カメラの注視点をプレイヤーの座標にしてみる
	g_Camera.at = pos;

	// カメラの視点をカメラのY軸回転に対応させている
	g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
	g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;

}


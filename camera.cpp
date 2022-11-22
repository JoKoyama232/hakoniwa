//=============================================================================
//
// �J�������� [camera.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "enemy.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	POS_X_CAM			(0.0f)			// �J�����̏����ʒu(X���W)
#define	POS_Y_CAM			(50.0f)			// �J�����̏����ʒu(Y���W)
#define	POS_Z_CAM			(-140.0f)		// �J�����̏����ʒu(Z���W)

//#define	POS_X_CAM		(0.0f)			// �J�����̏����ʒu(X���W)
//#define	POS_Y_CAM		(200.0f)		// �J�����̏����ʒu(Y���W)
//#define	POS_Z_CAM		(-400.0f)		// �J�����̏����ʒu(Z���W)


#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// �r���[���ʂ̎���p
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// �r���[���ʂ̃A�X�y�N�g��	
#define	VIEW_NEAR_Z		(10.0f)											// �r���[���ʂ�NearZ�l
#define	VIEW_FAR_Z		(10000.0f)										// �r���[���ʂ�FarZ�l

#define	VALUE_MOVE_CAMERA	(2.0f)										// �J�����̈ړ���
#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)								// �J�����̉�]��

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static CAMERA			g_Camera;		// �J�����f�[�^

static int				g_ViewPortType = TYPE_FULL_SCREEN;

static INTERPOLATION_DATAPOINT g_moveList[] = {
	//���W								��]��							�g�嗦							����
	{ XMFLOAT3(650.0f,   200.0f, -650.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		500},
	{ XMFLOAT3(-600.0f,   7.0f, -600.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		500 },
	{ XMFLOAT3(0.0f,   7.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		1 },
};

static INTERPOLATION_DATAPOINT* g_MoveListMngr[] =
{
	g_moveList,
};

//=============================================================================
// ����������
//=============================================================================
void InitCamera(void)
{
	g_Camera.pos = { POS_X_CAM, POS_Y_CAM, POS_Z_CAM };
	g_Camera.at  = { 0.0f, 0.0f, 0.0f };
	g_Camera.up  = { 0.0f, 1.0f, 0.0f };
	g_Camera.rot = { 0.0f, 0.0f, 0.0f };

	// ���_�ƒ����_�̋������v�Z
	float vx, vz;
	vx = g_Camera.pos.x - g_Camera.at.x;
	vz = g_Camera.pos.z - g_Camera.at.z;
	g_Camera.len = sqrtf(vx * vx + vz * vz);

	g_Camera.time = 0.0f;		// ���`��ԗp�̃^�C�}�[���N���A
	g_Camera.tblNo = 0;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Camera.tblMax = sizeof(g_moveList) / sizeof(INTERPOLATION_DATAPOINT);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	// �r���[�|�[�g�^�C�v�̏�����
	g_ViewPortType = TYPE_FULL_SCREEN;
}


//=============================================================================
// �J�����̏I������
//=============================================================================
void UninitCamera(void)
{

}


//=============================================================================
// �J�����̍X�V����
//=============================================================================
void UpdateCamera(void)
{

	int nowNo = (int)g_Camera.time;			// �������ł���e�[�u���ԍ������o���Ă���
	int maxNo = g_Camera.tblMax;				// �o�^�e�[�u�����𐔂��Ă���
	int nextNo = (nowNo + 1) % maxNo;			// �ړ���e�[�u���̔ԍ������߂Ă���
	INTERPOLATION_DATAPOINT* tbl = g_MoveListMngr[g_Camera.tblNo];	// �s���e�[�u���̃A�h���X���擾

	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTOR�֕ϊ�
	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTOR�֕ϊ�
	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTOR�֕ϊ�

	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ�ړ��ʂ��v�Z���Ă���
	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ��]�ʂ��v�Z���Ă���
	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ�g�嗦���v�Z���Ă���

	float nowTime = g_Camera.time - nowNo;	// ���ԕ����ł��鏭�������o���Ă���

	Pos *= nowTime;								// ���݂̈ړ��ʂ��v�Z���Ă���
	Rot *= nowTime;								// ���݂̉�]�ʂ��v�Z���Ă���
	Scl *= nowTime;								// ���݂̊g�嗦���v�Z���Ă���

	// �v�Z���ċ��߂��ړ��ʂ����݂̈ړ��e�[�u��XYZ�ɑ����Ă��遁�\�����W�����߂Ă���
	XMStoreFloat3(&g_Camera.pos, nowPos + Pos);

	// �v�Z���ċ��߂���]�ʂ����݂̈ړ��e�[�u���ɑ����Ă���
	XMStoreFloat3(&g_Camera.rot, nowRot + Rot);

	// frame���g�Ď��Ԍo�ߏ���������
	g_Camera.time += 1.0f / tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
	if ((int)g_Camera.time >= maxNo)			// �o�^�e�[�u���Ō�܂ňړ��������H
	{
		g_Camera.time -= maxNo;				// �O�ԖڂɃ��Z�b�g�������������������p���ł���
	}
}


//=============================================================================
// �J�����̍X�V
//=============================================================================
void SetCamera(void) 
{
	ENEMY* enemy = GetEnemy();
	// �r���[�}�g���b�N�X�ݒ�
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


	// �v���W�F�N�V�����}�g���b�N�X�ݒ�
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

	SetProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&g_Camera.mtxProjection, mtxProjection);

	SetShaderCamera(g_Camera.pos);
}


//=============================================================================
// �J�����̎擾
//=============================================================================
CAMERA *GetCamera(void) 
{
	return &g_Camera;
}

//=============================================================================
// �r���[�|�[�g�̐ݒ�
//=============================================================================
void SetViewPort(int type)
{
	ID3D11DeviceContext *g_ImmediateContext = GetDeviceContext();
	D3D11_VIEWPORT vp;

	g_ViewPortType = type;

	// �r���[�|�[�g�ݒ�
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



// �J�����̎��_�ƒ����_���Z�b�g
void SetCameraAT(XMFLOAT3 pos)
{
	// �J�����̒����_���v���C���[�̍��W�ɂ��Ă݂�
	g_Camera.at = pos;

	// �J�����̎��_���J������Y����]�ɑΉ������Ă���
	g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
	g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;

}


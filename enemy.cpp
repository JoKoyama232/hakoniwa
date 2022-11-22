//=============================================================================
//
// �G�l�~�[���f������ [enemy.cpp]
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
// �}�N����`
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/enemy.obj"		// �ǂݍ��ރ��f����
#define MODEL_GUN			"data/MODEL/gun.obj"
#define MODEL_SHEILD		"data/MODEL/shield.obj"
#define MODEL_FUNNEL		"data/MODEL/FinFunnel_s.obj"
#define MODEL_ARIEL			"data/MODEL/Aerial.obj"
#define	VALUE_MOVE			(5.0f)						// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// ��]��

#define ENEMY_SHADOW_SIZE	(0.4f)						// �e�̑傫��
#define ENEMY_OFFSET_Y		(7.0f)						// �G�l�~�[�̑��������킹��

enum {
	aerial = 0,
	funnel,
	shield,
	gun,
};
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// �G�l�~�[

int g_Enemy_load = 0;

static INTERPOLATION_DATAPOINT g_moveList[] = {
	//���W								��]��							�g�嗦							����
	{ XMFLOAT3( 0.0f,   7.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		30 },
	{ XMFLOAT3(100.0f,  7.0f, 100.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		30 },
	{ XMFLOAT3(50.0f,  7.0f, 100.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		30 },
};

static INTERPOLATION_DATAPOINT* g_MoveListMngr[] =
{
	g_moveList,
};


//=============================================================================
// ����������
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

		g_Enemy[i].spd  = 0.0f;			// �ړ��X�s�[�h�N���A
		g_Enemy[i].size = ENEMY_SIZE;	// �����蔻��̑傫��

		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);
		
		g_Enemy[0].time = 0.0f;		// ���`��ԗp�̃^�C�}�[���N���A
		g_Enemy[0].tblNo = 0;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Enemy[0].tblMax = sizeof(g_moveList) / sizeof(INTERPOLATION_DATAPOINT);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

		g_Enemy[i].use = true;		// true:�����Ă�

	}

	return S_OK;
}

//=============================================================================
// �I������
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
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
	// �G�l�~�[�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == true)		// ���̃G�l�~�[���g���Ă���H
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
		// ���f���̐F��ύX�ł����I�������ɂ��ł����B
		for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Enemy[0].model, j, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));
		}
	}

	if (GetKeyboardTrigger(DIK_L))
	{
		// ���f���̐F�����ɖ߂��Ă���
		for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Enemy[0].model, j, g_Enemy[0].diffuse[j]);
		}
	}
#endif


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == false) continue;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// ���f���`��
		DrawModel(&g_Enemy[i].model);
	}

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// �G�l�~�[�̎擾
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}

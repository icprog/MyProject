// PciDll.cpp : �w�q DLL ����l�Ʊ`���C
//

#include "stdafx.h"
#include "PciDll.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	�`�N!
//
//		�p�G�o�� DLL �O�ʺA�a�� MFC DLL �s���A����q�o�� DLL �ץX������|�I�s
//		MFC �������禡�A�������b�禡�@�}�Y�[�W AFX_MANAGE_STATE �����C
//
//		�Ҧp:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// ���B�����`�禡�D��
//		}
//
//		�o�ӥ����@�w�n�X�{�b�C�@�Ө禡���A�~����I�s MFC �������C
//		�o�N���ۥ�������ܬ��禡�����Ĥ@�ӳ��z���A�Ʀܥ����b
//		���󪫥��ܼƫŧi���e���A�]�����̪��غc�禡�i��|���͹�
//		MFC DLL �������I�s�C
//
//		�p�ݸԲӸ�T�A�аѾ\ MFC �޳N���� 33 �M 58�C
//

// CPciDllApp

BEGIN_MESSAGE_MAP(CPciDllApp, CWinApp)
END_MESSAGE_MAP()


// CPciDllApp �غc

CPciDllApp::CPciDllApp()
{
	// TODO: �b���[�J�غc�{���X�A
	// �N�Ҧ����n����l�]�w�[�J InitInstance ��
}


// �Ȧ����@�� CPciDllApp ����

CPciDllApp theApp;


// CPciDllApp ��l�]�w

BOOL CPciDllApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

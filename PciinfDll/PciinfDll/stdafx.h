// stdafx.h : �i�b�����Y�ɤ��]�t�зǪ��t�� Include �ɡA
// �άO�g�`�ϥΫo�ܤ��ܧ󪺱M�ױM�� Include �ɮ�
//

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// �q Windows ���Y�ư����`�ϥΪ�����
#endif

// �p�G�z�������u����������x�A�Эק�U�C�w�q�C
// �Ѧ� MSDN ���o���P���x�����Ȫ��̷s��T�C
#ifndef WINVER				// ���\�ϥ� Windows 95 �P Windows NT 4 (�t) �H�᪩�����S�w�\��C
#define WINVER 0x0400		// �N���ܧ󬰰w�� Windows 98 �M Windows 2000 (�t) �H�᪩���A���ȡC
#endif

#ifndef _WIN32_WINNT		// ���\�ϥ� Windows NT 4 (�t) �H�᪩�����S�w�\��C
#define _WIN32_WINNT 0x0400	// �N���ܧ󬰰w�� Windows 2000 (�t) �H�᪩�����A��ȡC
#endif						

#ifndef _WIN32_WINDOWS		// ���\�ϥ� Windows 98 (�t) �H�᪩�����S�w�\��C
#define _WIN32_WINDOWS 0x0410 // �N���ܧ󬰰w�� Windows Me (�t) �H�᪩���A���ȡC
#endif

#ifndef _WIN32_IE			// ���\�ϥ� IE 4.0 (�t) �H�᪩�����S�w�\��C
#define _WIN32_IE 0x0400	// �N���ܧ󬰰w�� IE 5.0 (�t) �H�᪩���A���ȡC
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ���T�w�q������ CString �غc�禡

#include <afxwin.h>         // MFC �֤߻P�зǤ���
#include <afxext.h>         // MFC �X�R�\��

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE ���O
#include <afxodlgs.h>       // MFC OLE ��ܤ�����O
#include <afxdisp.h>        // MFC Automation ���O
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC ��Ʈw���O
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO ��Ʈw���O
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC �䴩�� Internet Explorer 4 �q�α��
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC �䴩�� Windows �q�α��
#endif // _AFX_NO_AFXCMN_SUPPORT

// PciDll.h : PciDll DLL ���D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// �D�n�Ÿ�


// CPciDllApp
// �o�����O����@�аѾ\ PciDll.cpp
//

class CPciDllApp : public CWinApp
{
public:
	CPciDllApp();

// �мg
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

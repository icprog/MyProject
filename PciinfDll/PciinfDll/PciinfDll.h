// PciinfDll.h : PciinfDll DLL ���D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// �D�n�Ÿ�


// CPciinfDllApp
// �o�����O����@�аѾ\ PciinfDll.cpp
//

class CPciinfDllApp : public CWinApp
{
public:
	CPciinfDllApp();

// �мg
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

// PciinfDll.h : PciinfDll DLL 的主要標頭檔
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// 主要符號


// CPciinfDllApp
// 這個類別的實作請參閱 PciinfDll.cpp
//

class CPciinfDllApp : public CWinApp
{
public:
	CPciinfDllApp();

// 覆寫
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

// PciDll.h : PciDll DLL 的主要標頭檔
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// 主要符號


// CPciDllApp
// 這個類別的實作請參閱 PciDll.cpp
//

class CPciDllApp : public CWinApp
{
public:
	CPciDllApp();

// 覆寫
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

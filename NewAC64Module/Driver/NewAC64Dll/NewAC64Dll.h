// NewAC64Dll.h : main header file for the NewAC64Dll DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CNewAC64DllApp
// See NewAC64Dll.cpp for the implementation of this class
//

class CNewAC64DllApp : public CWinApp
{
public:
	CNewAC64DllApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

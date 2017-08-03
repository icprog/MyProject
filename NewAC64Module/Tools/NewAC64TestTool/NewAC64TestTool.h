// NewAC64TestTool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CNewAC64TestToolApp:
// See NewAC64TestTool.cpp for the implementation of this class
//

class CNewAC64TestToolApp : public CWinApp
{
public:
	CNewAC64TestToolApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CNewAC64TestToolApp theApp;
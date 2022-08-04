
// ObjectClassifierGUI.h : main header file for the ObjectClassifierGUI application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CObjectClassifierGUIApp:
// See ObjectClassifierGUI.cpp for the implementation of this class
//

class CObjectClassifierGUIApp : public CWinAppEx
{
public:
	CObjectClassifierGUIApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CObjectClassifierGUIApp theApp;

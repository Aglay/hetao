
// minihetao.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMiniHetaoApp:
// �йش����ʵ�֣������ minihetao.cpp
//

class CMiniHetaoApp : public CWinAppEx
{
public:
	CMiniHetaoApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
public:
	int m_argc;
	LPWSTR *m_argv;
};

extern CMiniHetaoApp theApp;

DWORD WINAPI minihetao( LPVOID p );

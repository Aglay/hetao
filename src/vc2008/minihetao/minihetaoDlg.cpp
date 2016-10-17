
// minihetaoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "minihetao.h"
#include "minihetaoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_WAITFOR_WORKERTHREAD	1
#define TIMER_INIT_DIALOG		2

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMiniHetaoDlg �Ի���




CMiniHetaoDlg::CMiniHetaoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMiniHetaoDlg::IDD, pParent)
	, m_dwThreadId(0)
	, m_strStaticRemark(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strWWWRoot = "" ;
	m_hRunningThread = NULL ;
	m_strStaticRemark.Format(	"\n"
					"minihetao v%s\n"
					"\n"
					"Copyright by calvin 2016\n"
					"\n"
					"Email : <calvinwilliams@163.com>\n"
					"\n" , __HETAO_VERSION );
}

void CMiniHetaoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_WWWROOT, m_strWWWRoot);
	DDX_Control(pDX, IDC_BUTTON_RUNNING, m_ctlRunningButton);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_ctlStopButton);
	DDX_Control(pDX, IDC_BUTTON_REGISTE, m_ctlRegisteFolderPopupmenu);
	DDX_Control(pDX, IDC_BUTTON_UNREGISTE, m_ctlUnregisteFolderPopupmenu);
	DDX_Control(pDX, IDC_EDIT_WWWROOT, m_ctlWWWRoot);
	DDX_Text(pDX, IDC_STATIC_REMARK, m_strStaticRemark);
}

BEGIN_MESSAGE_MAP(CMiniHetaoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CMiniHetaoDlg::OnBnClickedButtonExit)
	ON_MESSAGE( WM_CLICK_POPUPMENU , &CMiniHetaoDlg::OnClickPopupMenu )
	ON_COMMAND( WM_SHOW_WINDOW , &CMiniHetaoDlg::OnShowWindow )
	ON_COMMAND( IDM_ABOUTBOX , &CMiniHetaoDlg::OnShowAboutBox )
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_HIDE, &CMiniHetaoDlg::OnBnClickedButtonHide)
	ON_BN_CLICKED(IDC_BUTTON_SELECTDIRECTORY, &CMiniHetaoDlg::OnBnClickedButtonSelectDirectory)
	ON_BN_CLICKED(IDC_BUTTON_RUNNING, &CMiniHetaoDlg::OnBnClickedButtonRunning)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMiniHetaoDlg::OnBnClickedButtonStop)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_REGISTE, &CMiniHetaoDlg::OnBnClickedButtonRegiste)
	ON_BN_CLICKED(IDC_BUTTON_UNREGISTE, &CMiniHetaoDlg::OnBnClickedButtonUnregiste)
END_MESSAGE_MAP()

// CMiniHetaoDlg ��Ϣ�������

BOOL CMiniHetaoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	
	HKEY		regkey ;
	long		lret ;
	
	if( theApp.m_argc >= 2 )
	{
		if( _wcsicmp( theApp.m_argv[1] , L"--registe-folder-popupmenu" ) == 0 )
		{
			OnBnClickedButtonRegiste();
			theApp.m_pMainWnd->SendMessage(WM_DESTROY);
			theApp.m_pMainWnd->SendMessage(WM_CLOSE);
		}
		else if( _wcsicmp( theApp.m_argv[1] , L"--unregiste-folder-popupmenu" ) == 0 )
		{
			OnBnClickedButtonUnregiste();
			theApp.m_pMainWnd->SendMessage(WM_DESTROY);
			theApp.m_pMainWnd->SendMessage(WM_CLOSE);
		}
		else
		{
			m_strWWWRoot = theApp.m_argv[1] ;
			theApp.m_pMainWnd->SendMessage( WM_COMMAND , IDC_BUTTON_RUNNING );
			SetTimer( TIMER_INIT_DIALOG , 3000 , NULL );
		}
	}
	else
	{
		TCHAR	path[ MAX_PATH ] ;
		
		memset( path , 0x00 , sizeof(path) );
		GetCurrentDirectory( sizeof(path) , path );
		m_strWWWRoot = path ;
	}

	/* �����Ƿ���ע��Ŀ¼�Ҽ��˵������ʧЧһЩ��ť */
	lret = RegOpenKeyEx( HKEY_CLASSES_ROOT , "Folder\\shell\\minihetao" , 0 , KEY_READ , & regkey ) ;
	if( lret != ERROR_SUCCESS )
	{
		m_ctlRegisteFolderPopupmenu.EnableWindow( TRUE );
		m_ctlUnregisteFolderPopupmenu.EnableWindow( FALSE );
	}
	else
	{
		m_ctlRegisteFolderPopupmenu.EnableWindow( FALSE );
		m_ctlUnregisteFolderPopupmenu.EnableWindow( TRUE );
	}
	RegCloseKey( regkey );
	
	// ��������ͼ��
	m_nid.cbSize = sizeof(NOTIFYICONDATA) ;
	m_nid.hWnd = this->m_hWnd ;
	m_nid.uID = IDR_MAINFRAME ;
	m_nid.hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME)) ;
	m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP ;
	m_nid.uCallbackMessage = WM_CLICK_POPUPMENU ;
	strcpy_s( m_nid.szTip , "Hello" ) ;
	Shell_NotifyIcon( NIM_ADD , & m_nid );
	
	// ���¶Ի���
	UpdateData( FALSE );

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMiniHetaoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMiniHetaoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMiniHetaoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMiniHetaoDlg::OnShowWindow()
{
	this->ShowWindow( SW_SHOW );
	
	return;
}

void CMiniHetaoDlg::OnShowAboutBox()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
	
	return;
}

void CMiniHetaoDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
	Shell_NotifyIcon( NIM_DELETE , & m_nid );
}

int CALLBACK BrowseCallbackProc( HWND hwnd , UINT uMsg , LPARAM lp , LPARAM pData )
{
	CString		strWWWRoot = CString((LPCTSTR)pData) ;
	TCHAR		sg_szDir[ MAX_PATH ] ;
	
	switch(uMsg) 
	{
		case BFFM_INITIALIZED: 
			memset( sg_szDir , 0x00 , sizeof(sg_szDir) );
			strncpy( sg_szDir , (LPSTR)(LPCSTR)strWWWRoot , sizeof(sg_szDir)-1 );
			::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)sg_szDir );
			break;
		case BFFM_SELCHANGED: 
			memset( sg_szDir , 0x00 , sizeof(sg_szDir) );
			if (SHGetPathFromIDList((LPITEMIDLIST) lp ,sg_szDir))
			{
				::SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)sg_szDir);
			}
			break;
	}
	
	return 0;
}

void CMiniHetaoDlg::OnBnClickedButtonSelectDirectory()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	BROWSEINFO	bi ;
	char		buffer[ MAX_PATH ] ;
	LPITEMIDLIST	piil ;
	char		path[ MAX_PATH ] ;
	
	UpdateData( TRUE );
	
	memset( & bi , 0x00 , sizeof(BROWSEINFO) );
	bi.hwndOwner = this->m_hWnd ;
	bi.pidlRoot = NULL ;
	memset( buffer , 0x00 , sizeof(buffer) );
	bi.pszDisplayName = buffer ;
	bi.lpszTitle = "Select wwwroot for your website" ;
	bi.ulFlags = BIF_BROWSEFORCOMPUTER && BIF_RETURNONLYFSDIRS ;
	bi.lpfn = BrowseCallbackProc ;
	bi.lParam = (LPARAM)(LPCTSTR)m_strWWWRoot ;
	bi.iImage = 0 ;
	piil = SHBrowseForFolder( & bi ) ;
	if( piil )
	{
		memset( path , 0x00 , sizeof(path) );
		SHGetPathFromIDList( piil , path );
		m_strWWWRoot.Format( "%s" , path );
		
		UpdateData( FALSE );
	}

	return;
}

extern "C"
{
extern signed char	g_worker_exit_flag ;
}

void CMiniHetaoDlg::OnBnClickedButtonRunning()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	g_worker_exit_flag = 0 ;
	
	m_hRunningThread = CreateThread( NULL , 0 , minihetao , (LPVOID)(LPCTSTR)m_strWWWRoot , 0 , & m_dwThreadId ) ;
	if( m_hRunningThread == NULL )
	{
		AfxMessageBox( "CreateThread failed" );
		return;
	}
	else
	{
		m_ctlWWWRoot.EnableWindow( FALSE );
		m_ctlRunningButton.EnableWindow( FALSE );
		m_ctlStopButton.EnableWindow( TRUE );
		SetTimer( TIMER_WAITFOR_WORKERTHREAD , 1000 , NULL );
	}
	
	return;
}

void CMiniHetaoDlg::OnBnClickedButtonStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if( m_hRunningThread )
	{
		g_worker_exit_flag = 1 ;
	}
	
	return ;
}

void CMiniHetaoDlg::OnBnClickedButtonHide()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	this->ShowWindow( SW_HIDE );
}

void CMiniHetaoDlg::OnBnClickedButtonExit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	theApp.m_pMainWnd->SendMessage(WM_DESTROY);
	theApp.m_pMainWnd->SendMessage(WM_CLOSE);
}

LRESULT CMiniHetaoDlg::OnClickPopupMenu( WPARAM wParam , LPARAM lParam )
{
	POINT		point ;
	CMenu		menu , *p_menu = NULL ;

	if( wParam != IDR_MAINFRAME )
		return 1;
	
	switch( lParam )
	{
		case WM_RBUTTONUP:
			::GetCursorPos( & point );
			menu.LoadMenu( IDR_MENU_POPUP );
			p_menu = menu.GetSubMenu(0) ;
			p_menu->TrackPopupMenu( TPM_LEFTALIGN , point.x , point.y , this );
			SetForegroundWindow();
			break;
		case WM_LBUTTONDBLCLK:
			this->ShowWindow( SW_SHOW );
			break;
	}

	return 0;
}

void CMiniHetaoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	switch( nIDEvent )
	{
		case TIMER_INIT_DIALOG :
			theApp.m_pMainWnd->SendMessage( WM_COMMAND , IDC_BUTTON_HIDE );
			KillTimer( TIMER_INIT_DIALOG );
			break;
		case TIMER_WAITFOR_WORKERTHREAD :
			if( m_hRunningThread )
			{
				DWORD	dwret ;
				
				dwret = WaitForSingleObject( m_hRunningThread , 0 ) ;
				if( dwret == WAIT_OBJECT_0 )
				{
					m_ctlWWWRoot.EnableWindow( TRUE );
					m_ctlRunningButton.EnableWindow( TRUE );
					m_ctlStopButton.EnableWindow( FALSE );
					m_hRunningThread = NULL ;
					KillTimer( TIMER_WAITFOR_WORKERTHREAD );
				}
			}
			break;
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CMiniHetaoDlg::OnBnClickedButtonRegiste()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
	HKEY		regkey , regkey2 ;
	char		szModule[ MAX_PATH ] ;
	char		szRegValue[ MAX_PATH * 2 ] ;
	long		lret ;
	
	lret = RegCreateKeyEx( HKEY_CLASSES_ROOT , "Folder\\shell\\minihetao" , 0 , NULL , 0 , KEY_READ | KEY_WRITE , NULL , & regkey , NULL ) ;
	if( lret != ERROR_SUCCESS )
	{
		AfxMessageBox( "RegCreateKeyEx failed" );
		return;
	}
	
	lret = RegCreateKeyEx( HKEY_CLASSES_ROOT , "Folder\\shell\\minihetao\\command" , 0 , NULL , 0 , KEY_READ | KEY_WRITE , NULL , & regkey2 , NULL ) ;
	if( lret != ERROR_SUCCESS )
	{
		AfxMessageBox( "RegCreateKeyEx failed" );
		RegCloseKey( regkey );
		return;
	}
	
	memset( szModule , 0x00 , sizeof(szModule) );
	GetModuleFileName( NULL , szModule , sizeof(szModule) );
	memset( szRegValue , 0x00 , sizeof(szRegValue) );
	_snprintf( szRegValue , sizeof(szRegValue)-1 , "\"%s\" \"%%1\"" , szModule );
	lret = RegSetValueEx( regkey2 , NULL , 0 , REG_SZ , (BYTE*)szRegValue , strlen(szRegValue) ) ;
	if( lret != ERROR_SUCCESS )
	{
		AfxMessageBox( "RegSetValueEx failed" );
		RegCloseKey( regkey2 );
		RegCloseKey( regkey );
		return;
	}
	
	RegCloseKey( regkey2 );
	RegCloseKey( regkey );

	m_ctlRegisteFolderPopupmenu.EnableWindow( FALSE );
	m_ctlUnregisteFolderPopupmenu.EnableWindow( TRUE );
	
	return;
}

void CMiniHetaoDlg::OnBnClickedButtonUnregiste()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	long		lret ;
	
	lret = RegDeleteKey( HKEY_CLASSES_ROOT , "Folder\\shell\\minihetao\\command" ) ;
	if( lret != ERROR_SUCCESS )
	{
		AfxMessageBox( "RegDeleteKey failed" );
	}
	
	lret = RegDeleteKey( HKEY_CLASSES_ROOT , "Folder\\shell\\minihetao" ) ;
	if( lret != ERROR_SUCCESS )
	{
		AfxMessageBox( "RegDeleteKey failed" );
	}
	
	m_ctlRegisteFolderPopupmenu.EnableWindow( TRUE );
	m_ctlUnregisteFolderPopupmenu.EnableWindow( FALSE );
	
	return;
}


// minihetaoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "minihetao.h"
#include "minihetaoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
{
	char	*p = NULL ;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	p = getenv("ProgramFiles") ;
	if( p )
		m_strWWWRoot.Format( "%s\\hetao\\www" , p );
	else
		m_strWWWRoot = "" ;

	m_hRunningThread = NULL ;
}

void CMiniHetaoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_WWWROOT, m_strWWWRoot);
	DDX_Control(pDX, IDC_BUTTON_RUNNING, m_ctlRunningButton);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_ctlStopButton);
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
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CMiniHetaoDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_RUNNING, &CMiniHetaoDlg::OnBnClickedButtonRunning)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMiniHetaoDlg::OnBnClickedButtonStop)
	ON_WM_TIMER()
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
	// ��������ͼ��
	m_nid.cbSize = sizeof(NOTIFYICONDATA) ;
	m_nid.hWnd = this->m_hWnd ;
	m_nid.uID = IDR_MAINFRAME ;
	m_nid.hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME)) ;
	m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP ;
	m_nid.uCallbackMessage = WM_CLICK_POPUPMENU ;
	strcpy_s( m_nid.szTip , "Hello" ) ;
	Shell_NotifyIcon( NIM_ADD , & m_nid );

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
		m_ctlRunningButton.EnableWindow( FALSE );
		m_ctlStopButton.EnableWindow( TRUE );
		SetTimer( 1 , 1000 , NULL );
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

void CMiniHetaoDlg::OnBnClickedButtonAbout()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
	
	return;
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

	if( m_hRunningThread )
	{
		DWORD	dwret ;
		
		dwret = WaitForSingleObject( m_hRunningThread , 0 ) ;
		if( dwret == WAIT_OBJECT_0 )
		{
			m_ctlRunningButton.EnableWindow( TRUE );
			m_ctlStopButton.EnableWindow( FALSE );
			m_hRunningThread = NULL ;
			KillTimer( 1 );
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

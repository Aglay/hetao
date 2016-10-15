
// minihetao.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "minihetao.h"
#include "minihetaoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMiniHetaoApp

BEGIN_MESSAGE_MAP(CMiniHetaoApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMiniHetaoApp ����

CMiniHetaoApp::CMiniHetaoApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CMiniHetaoApp ����

CMiniHetaoApp theApp;


// CMiniHetaoApp ��ʼ��

BOOL CMiniHetaoApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	// ��ʾ�Ի���
	CMiniHetaoDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

extern "C"
{
#include "../../hetao_in.h"
}

#define PREFIX_DSCLOG_hetao_conf	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_hetao_conf
#include "../../IDL_hetao_conf.dsc.LOG.c"

#if ( defined _WIN32 )
static	WSADATA		wsd;
#endif

DWORD WINAPI minihetao( LPVOID p )
{
	char			*wwwroot = (char*)p ;
	hetao_conf		*p_conf = NULL ;
	struct HetaoEnv		*p_env = NULL ;
	
	int			nret = 0 ;
	
	/* ���ñ�׼����޻��� */
	setbuf( stdout , NULL );
	
	/* ������������� */
	srand( (unsigned)time(NULL) );
	
	/* �����ļ�����0 */
	UMASK(0);
	
	if( p )
	{
		if( WSAStartup( MAKEWORD(2,2) , & wsd ) != 0 )
			return 1;
		/* ���뻷���ṹ�ڴ� */
		p_env = (struct HetaoEnv *)malloc( sizeof(struct HetaoEnv) ) ;
		if( p_env == NULL )
		{
			return 1;
		}
		memset( p_env , 0x00 , sizeof(struct HetaoEnv) );
		g_p_env = p_env ;
		p_env->argv = NULL ;
		
		/* �������ýṹ�ڴ� */
		p_conf = (hetao_conf *)malloc( sizeof(hetao_conf) ) ;
		if( p_conf == NULL )
		{
			return 1;
		}
		memset( p_conf , 0x00 , sizeof(hetao_conf) );
		
		/* ��������HTTP״̬�롢����Ϊȱʡ */
		ResetAllHttpStatus();
		
		/* ����ȱʡ����־ */
		if( getenv( HETAO_LOG_PATHFILENAME ) == NULL )
			SetLogFile( "#" );
		else
			SetLogFile( getenv(HETAO_LOG_PATHFILENAME) );
		SetLogLevel( LOGLEVEL_ERROR );
		SETPID
		SETTID
		UPDATEDATETIMECACHEFIRST
		
		/* ����ȱʡ���� */
		SetDefaultConfig( p_conf );
		
		if( IsDirectory( wwwroot ) != 1 )
		{
			return 1;
		}
		
		strcpy( p_conf->listen[0].ip , "" );
		p_conf->listen[0].port = 80 ;
		p_conf->_listen_count = 1 ;
		
		strcpy( p_conf->listen[0].website[0].domain , "" );
		strcpy( p_conf->listen[0].website[0].wwwroot , wwwroot );
		strcpy( p_conf->listen[0].website[0].index , "/index.html,/index.htm" );
		strcpy( p_conf->listen[0].website[0].access_log , "" );
		p_conf->listen[0]._website_count = 1 ;
		
		/* ׷��ȱʡ���� */
		AppendDefaultConfig( p_conf );
		
		/* ת������ */
		nret = ConvertConfig( p_conf , p_env ) ;
		if( nret )
		{
			free( p_conf );
			free( p_env );
			return -nret;
		}
		
		/* ������������־ */
		CloseLogFile();
		
		SetLogFile( p_env->error_log );
		SetLogLevel( p_env->log_level );
		SETPID
		SETTID
		UPDATEDATETIMECACHEFIRST
		InfoLog( __FILE__ , __LINE__ , "--- hetao v%s build %s %s ---" , __HETAO_VERSION , __DATE__ , __TIME__ );
		SetHttpCloseExec( g_file_fd );
		
		/* ������õ���־ */
		DSCLOG_hetao_conf( p_conf );
		
		/* ��ʼ������������ */
		nret = InitEnvirment( p_env , p_conf ) ;
		free( p_conf );
		if( nret )
		{
			return -nret;
		}
		
		return -WorkerProcess( p_env );
	}
	
	return 0;
}

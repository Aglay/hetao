/*
 * hetao - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "hetao_in.h"

#define PREFIX_DSCLOG_hetao_conf	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_hetao_conf
#include "IDL_hetao_conf.dsc.LOG.c"

#if ( defined _WIN32 )
static	WSADATA		wsd;
#endif

static void usage()
{
	printf( "hetao v%s build %s %s\n" , __HETAO_VERSION , __DATE__ , __TIME__ );
	printf( "USAGE : hetao hetao.conf\n" );
#if ( defined _WIN32 )
	printf( "        hetao hetao.conf [--install-service | --uninstall-service ]\n" );
#endif
	return;
}

int main( int argc , char *argv[] )
{
	hetao_conf		*p_conf = NULL ;
	struct HetaoEnv		*p_env = NULL ;
	
	int			nret = 0 ;
	
	/* ���ñ�׼����޻��� */
	setbuf( stdout , NULL );
	
	/* ������������� */
	srand( (unsigned)time(NULL) );
	
	/* �����ļ�����0 */
	UMASK(0);
	
	if( argc == 1 + 1 || ( argc == 1 + 2 && ( STRCMP( argv[2] , == , "--no-daemon" ) || STRCMP( argv[2] , == , "--service" ) || STRCMP( argv[2] , == , "--child" ) ) ) )
	{
#if ( defined __linux ) || ( defined __unix )
#elif ( defined _WIN32 )
#else
		printf( "��֧�ֵĲ���ϵͳ\n" );
		return 1;
#endif

#if ( defined _WIN32 )
		if( WSAStartup( MAKEWORD(2,2) , & wsd ) != 0 )
			return 1;
#endif
		/* ���뻷���ṹ�ڴ� */
		p_env = (struct HetaoEnv *)malloc( sizeof(struct HetaoEnv) ) ;
		if( p_env == NULL )
		{
			if( GETENV( HETAO_LISTEN_SOCKFDS ) == NULL )
				printf( "alloc failed[%d] , errno[%d]\n" , nret , ERRNO );
			return 1;
		}
		memset( p_env , 0x00 , sizeof(struct HetaoEnv) );
		g_p_env = p_env ;
		p_env->argv = argv ;
		
		/* �������ýṹ�ڴ� */
		p_conf = (hetao_conf *)malloc( sizeof(hetao_conf) ) ;
		if( p_conf == NULL )
		{
			if( GETENV( HETAO_LISTEN_SOCKFDS ) == NULL )
				printf( "alloc failed[%d] , errno[%d]\n" , nret , ERRNO );
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
		
		/* װ������ */
		strncpy( p_env->config_pathfilename , argv[1] , sizeof(p_env->config_pathfilename)-1 );
		nret = LoadConfig( p_env->config_pathfilename , p_conf , p_env ) ;
		if( nret )
		{
			if( GETENV( HETAO_LISTEN_SOCKFDS ) == NULL )
				printf( "LoadConfig failed[%d]\n" , nret );
			free( p_conf );
			free( p_env );
			return -nret;
		}
		
		/* ׷��ȱʡ���� */
		AppendDefaultConfig( p_conf );
		
		/* ת������ */
		nret = ConvertConfig( p_conf , p_env ) ;
		if( nret )
		{
			if( GETENV( HETAO_LISTEN_SOCKFDS ) == NULL )
				printf( "ConvertConfig failed[%d]\n" , nret );
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
			if( GETENV( HETAO_LISTEN_SOCKFDS ) == NULL )
				printf( "InitEnvirment failed[%d]\n" , nret );
			return -nret;
		}
		
#if ( defined __linux ) || ( defined __unix )
		if( STRCMP( argv[2] , == , "--no-daemon" ) )
		{
			return -MonitorProcess( (void*)p_env );
		}
		else
		{
			return -BindDaemonServer( & MonitorProcess , p_env );
		}
#elif ( defined _WIN32 )
		if( argc == 1 + 2 && STRCMP( argv[2] , == , "--service" ) )
		{
			return -RunService();
		}
		else if( argc == 1 + 2 && STRCMP( argv[2] , == , "--child" ) )
		{
			return -WorkerProcess( p_env );
		}
		else
		{
			return -MonitorProcess( p_env );
		}
#endif
	}
#if ( defined _WIN32 )
	else if( argc == 1 + 2 )
	{
		if( STRCMP( argv[2] , == , "--install-service" ) )
		{
			nret = InstallService( argv[1] ) ;
			if( nret )
			{
				printf( "��װWINDOWS����ʧ��[%d]errno[%d]\n" , nret , ERRNO );
				exit(1);
			}
			else
			{
				printf( "��װWINDOWS����ɹ�\n" );
				exit(0);
			}
		}
		else if( STRCMP( argv[2] , == , "--uninstall-service" ) )
		{
			nret = UninstallService() ;
			if( nret )
			{
				printf( "ж��WINDOWS����ʧ��[%d]errno[%d]\n" , nret , ERRNO );
				exit(1);
			}
			else
			{
				printf( "ж��WINDOWS����ɹ�\n" );
				exit(0);
			}
		}
		else
		{
			usage();
			exit(9);
		}
	}
#endif
	else
	{
		usage();
		exit(9);
	}
}


/*
 * hetao - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "hetao_in.h"

static void usage()
{
	printf( "hetaocheck v%s build %s %s\n" , __HETAO_VERSION , __DATE__ , __TIME__ );
	printf( "USAGE : hetaocheck hetao.conf\n" );
	return;
}

int main( int argc , char *argv[] )
{
	struct HetaoEnv		*p_env = NULL ;
	hetao_conf		*p_conf = NULL ;
	
	int			nret = 0 ;
	
	UMASK(0);
	
	if( argc == 1 + 1 )
	{
		/* ���뻷���ṹ�ڴ� */
		p_env = (struct HetaoEnv *)malloc( sizeof(struct HetaoEnv) ) ;
		if( p_env == NULL )
		{
			if( getenv( HETAO_LISTEN_SOCKFDS ) == NULL )
				printf( "alloc failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
		}
		memset( p_env , 0x00 , sizeof(struct HetaoEnv) );
		g_p_env = p_env ;
		p_env->argv = argv ;
		
		/* �������ýṹ�ڴ� */
		p_conf = (hetao_conf *)malloc( sizeof(hetao_conf) ) ;
		if( p_conf == NULL )
		{
			if( getenv( HETAO_LISTEN_SOCKFDS ) == NULL )
				printf( "alloc failed[%d] , errno[%d]\n" , nret , errno );
			free( p_env );
			return 1;
		}
		memset( p_conf , 0x00 , sizeof(hetao_conf) );
		
		/* ����ȱʡ����־ */
		SetLogFile( "#" );
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
			printf( "FAILED[%d]\n" , nret );
			free( p_conf );
			free( p_env );
			return 1;
		}
		
		/* ׷��ȱʡ���� */
		AppendDefaultConfig( p_conf );
		
		/* ת������ */
		nret = ConvertConfig( p_conf , p_env ) ;
		if( nret )
		{
			printf( "FAILED[%d]\n" , nret );
			free( p_conf );
			free( p_env );
			return 1;
		}
		
		printf( "OK\n" );
		
		/* �ͷŻ��� */
		free( p_conf );
		free( p_env );
		
		/* �ر�����־ */
		CloseLogFile();
		
		return -nret;
	}
	else
	{
		usage();
		exit(9);
	}
}


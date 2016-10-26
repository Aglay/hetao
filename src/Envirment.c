/*
 * hetao - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "hetao_in.h"

int InitEnvirment( struct HetaoEnv *p_env , hetao_conf *p_conf )
{
	struct NetAddress	*old_netaddr_array = NULL ;
	int			old_netaddr_array_count = 0 ;
	
	int			nret = 0 ;
	
	/* ���������ڴ������������ʹ�� */
#if ( defined __linux ) || ( defined __unix )
	p_env->process_info_shmid = shmget( IPC_PRIVATE , sizeof(struct ProcessInfo) * p_conf->worker_processes , IPC_CREAT | 0600 ) ;
	if( p_env->process_info_shmid == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "shmget failed , errno[%d]" , ERRNO );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "shmget ok[%ld]" , p_env->process_info_shmid );
	}
	
	p_env->process_info_array = shmat( p_env->process_info_shmid , NULL , 0 ) ;
	if( p_env->process_info_array == (void*)-1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "shmat failed , errno[%d]" , ERRNO );
		return -2;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "shmat[%ld] ok , [%ld]bytes" , p_env->process_info_shmid , sizeof(struct ProcessInfo) * p_conf->worker_processes );
	}
	memset( p_env->process_info_array , 0x00 , sizeof(struct ProcessInfo) * p_conf->worker_processes );
#elif ( defined _WIN32 )
	if( GETENV(HETAO_PROCESS_INFO) == NULL )
	{
		SECURITY_ATTRIBUTES	sa ;
		memset( & sa , 0x00 , sizeof(SECURITY_ATTRIBUTES) );
		sa.nLength = sizeof(SECURITY_ATTRIBUTES) ;
		sa.bInheritHandle = TRUE ;
		p_env->process_info_shmid = CreateFileMapping( INVALID_HANDLE_VALUE , & sa , PAGE_READWRITE , 0 , sizeof(struct ProcessInfo) , NULL ) ;	
		if( p_env->process_info_shmid == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "CreateFileMapping failed , errno[%d]" , ERRNO );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "CreateFileMapping ok[%ld]" , p_env->process_info_shmid );
		}
	}
	else
	{
		p_env->process_info_shmid = (HANDLE)( atoi(GETENV(HETAO_PROCESS_INFO)) ) ;
	}
	
	p_env->p_process_info = MapViewOfFile( p_env->process_info_shmid , FILE_MAP_ALL_ACCESS , 0 , 0 , 0 );
	if( p_env->p_process_info == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "MapViewOfFile failed , errno[%d]" , ERRNO );
		return -2;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "MapViewOfFile[%ld] ok , [%ld]bytes" , p_env->process_info_shmid , sizeof(struct ProcessInfo) * p_conf->worker_processes );
	}
	
	if( GETENV(HETAO_PROCESS_INFO) == NULL )
	{
		memset( p_env->p_process_info , 0x00 , sizeof(struct ProcessInfo) );
	}
#endif
	
	/* ������ע�������͹�ϣ�� */
	nret = InitMimeTypeHash( p_env , p_conf ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitMimeTypeHash failed[%d]" , nret );
		return -1;
	}
	
	/* ���������Ự�����׽ڵ� */
	memset( & (p_env->listen_session_list) , 0x00 , sizeof(struct ListenSession) );
	INIT_LIST_HEAD( & (p_env->listen_session_list.list) );
	p_env->listen_session_count = 0 ;
	
	/* ����������������ӻ��������л����һ��������Ϣ */
	nret = LoadOldListenSockets( & old_netaddr_array , & old_netaddr_array_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "LoadOldListenSockets failed[%d] , errno[%d]" , nret , ERRNO );
		return -1;
	}
	
	/* �������������Ự */
	nret = InitListenEnvirment( p_env , p_conf , old_netaddr_array , old_netaddr_array_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitListenEnvirment failed[%d] , errno[%d]" , nret , ERRNO );
		return -1;
	}
	
	/* ������һ��������Ϣ */
	CloseUnusedOldListeners( old_netaddr_array , old_netaddr_array_count );
	
	/* ������ҳ����Ự�����׽ڵ� */
	memset( & (p_env->htmlcache_session_list) , 0x00 , sizeof(struct HtmlCacheSession) );
	INIT_LIST_HEAD( & (p_env->htmlcache_session_list.list) );
	p_env->htmlcache_session_count = 0 ;
	
	/* ��������HTTPͨѶ�Ự�����׽ڵ㣬Ԥ����ڵ� */
	memset( & (p_env->http_session_array_list) , 0x00 , sizeof(struct HttpSessionArray) );
	INIT_LIST_HEAD( & (p_env->http_session_array_list.list) );

	memset( & (p_env->http_session_unused_list) , 0x00 , sizeof(struct HttpSession) );
	INIT_LIST_HEAD( & (p_env->http_session_unused_list.list) );
	p_env->http_session_unused_count = 0 ;
	
	nret = IncreaseHttpSessions( p_env , INIT_HTTP_SESSION_COUNT ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "IncreaseHttpSessions failed[%d] , errno[%d]" , nret , ERRNO );
		return nret;
	}
	
	/* ����IP���ƹ�ϣ�� */
	nret = InitIpLimitsHash( p_env ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitIpLimitsHash failed[%d]" , nret );
		return -1;
	}
	
	return 0;
}

void CleanEnvirment( struct HetaoEnv *p_env )
{
	struct list_head	*p_curr = NULL , *p_next = NULL ;
	
	int			i ;
	struct HttpSession	*p_http_session = NULL ;
	struct HttpSessionArray	*p_http_session_array = NULL ;
	struct ListenSession	*p_listen_session = NULL ;
	
	list_for_each_safe( p_curr , p_next , & (p_env->http_session_unused_list.list) )
	{
		p_http_session = container_of( p_curr , struct HttpSession , list ) ;
		DestroyHttpEnv( p_http_session->http );
		FreeHttpBuffer( p_http_session->http_buf );
		DestroyHttpEnv( p_http_session->forward_http );
	}

	list_for_each_safe( p_curr , p_next , & (p_env->http_session_array_list.list) )
	{
		p_http_session_array = container_of( p_curr , struct HttpSessionArray , list ) ;
		free( p_http_session_array->http_session_array );
		free( p_http_session_array );
	}
	
	for( i = 0 ; i < p_env->worker_processes ; i++ )
	{
#if ( defined __linux ) || ( defined __unix )
		DebugLog( __FILE__ , __LINE__ , "[%d]close epoll_fd #%d#" , i , p_env->process_info_array[i].epoll_fd );
		CLOSE( p_env->process_info_array[i].epoll_fd );
#endif
	}
	
	if( p_env->new_uri_re )
	{
		free( p_env->new_uri_re );
	}
	
	DebugLog( __FILE__ , __LINE__ , "delete all listen_session" );
	list_for_each_safe( p_curr , p_next , & (p_env->listen_session_list.list) )
	{
		p_listen_session = container_of( p_curr , struct ListenSession , list ) ;
		
		CleanVirtualHostHash( p_listen_session );
		
		list_del( p_curr );
		free( p_listen_session );
	}
	
#if ( defined __linux ) || ( defined __unix )
	DebugLog( __FILE__ , __LINE__ , "close htmlcache_inotify_fd #%d#" , p_env->htmlcache_inotify_fd );
	CLOSE( p_env->htmlcache_inotify_fd );
#endif
	
#if ( defined __linux ) || ( defined __unix )
	DebugLog( __FILE__ , __LINE__ , "shmdt and shmctl IPC_RMID" );
	shmdt( p_env->process_info_array );
	shmctl( p_env->process_info_shmid , IPC_RMID , NULL );
#elif ( defined _WIN32 )
	UnmapViewOfFile( p_env->p_process_info );
	CloseHandle( p_env->process_info_shmid );
#endif
	
	CleanMimeTypeHash( p_env );
	
	CleanIpLimitsHash( p_env );
	
	return;
}

int SaveListenSockets( struct HetaoEnv *p_env )
{
	unsigned int		env_value_size ;
	char			*env_value = NULL ;
	struct list_head	*p_curr = NULL ;
	struct ListenSession	*p_listen_session = NULL ;
	
	env_value_size = (1+p_env->listen_session_count)*(10+1) + 1 ;
	env_value = (char*)malloc( env_value_size ) ;
	if( env_value == NULL )
		return -1;
	memset( env_value , 0x00 , env_value_size );
	
	/* ��������Ϣ���� */
	SNPRINTF( env_value , env_value_size-1 , "%s=%d|" , HETAO_LISTEN_SOCKFDS , p_env->listen_session_count );
	
	list_for_each( p_curr , & (p_env->listen_session_list.list) )
	{
		p_listen_session = container_of( p_curr , struct ListenSession , list ) ;
		
		/* ÿһ��������Ϣ */
		SNPRINTF( env_value+strlen(env_value) , env_value_size-1-strlen(env_value) , "%d|" , p_listen_session->netaddr.sock );
	}
	
	/* д�뻷������ */
	InfoLog( __FILE__ , __LINE__ , "putenv[%s]" , env_value );
	PUTENV( env_value );
	
	free( env_value );
	
	return 0;
}

int LoadOldListenSockets( struct NetAddress **pp_old_netaddr_array , int *p_old_netaddr_array_count )
{
	char				*p_env_value = NULL ;
	char				*p_sockfd_count = NULL ;
	int				i ;
	struct NetAddress		*p_old_netaddr = NULL ;
	char				*p_sockfd = NULL ;
	SOCKLEN_T			addr_len = sizeof(struct sockaddr) ;
	
	int				nret = 0 ;
	
	p_env_value = GETENV( HETAO_LISTEN_SOCKFDS ) ;
	InfoLog( __FILE__ , __LINE__ , "getenv[%s][%s]" , HETAO_LISTEN_SOCKFDS , p_env_value );
	if( p_env_value )
	{
		p_env_value = STRDUP( p_env_value ) ;
		if( p_env_value == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "strdup failed , errno[%d]" , ERRNO );
			return -1;
		}
		
		/* �Ƚ���������Ϣ���� */
		p_sockfd_count = strtok( p_env_value , "|" ) ;
		if( p_sockfd_count )
		{
			(*p_old_netaddr_array_count) = atoi(p_sockfd_count) ;
			if( (*p_old_netaddr_array_count) > 0 )
			{
				(*pp_old_netaddr_array) = (struct NetAddress *)malloc( sizeof(struct NetAddress) * (*p_old_netaddr_array_count) ) ;
				if( (*pp_old_netaddr_array) == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "malloc failed , errno[%d]" , ERRNO );
					free( p_env_value );
					return -1;
				}
				memset( (*pp_old_netaddr_array) , 0x00 , sizeof(struct NetAddress) * (*p_old_netaddr_array_count) );
				
				/* ����ÿһ��������Ϣ */
				for( i = 0 , p_old_netaddr = (*pp_old_netaddr_array) ; i < (*p_old_netaddr_array_count) ; i++ , p_old_netaddr++ )
				{
					p_sockfd = strtok( NULL , "|" ) ;
					if( p_sockfd == NULL )
					{
						ErrorLog( __FILE__ , __LINE__ , "env[%s][%s] invalid" , HETAO_LISTEN_SOCKFDS , p_env_value );
						free( p_env_value );
						return -1;
					}
					
					p_old_netaddr->sock = atoi(p_sockfd) ;
					nret = getsockname( p_old_netaddr->sock , (struct sockaddr *) & (p_old_netaddr->addr) , & addr_len ) ;
					if( nret == -1 )
					{
						ErrorLog( __FILE__ , __LINE__ , "getsockname[%d] failed , errno[%d]" , p_old_netaddr->sock , ERRNO );
						free( p_env_value );
						return -1;
					}
					
					p_old_netaddr->ip[sizeof(p_old_netaddr->ip)-1] = '\0' ;
#if ( defined __linux ) || ( defined __unix )
					inet_ntop( AF_INET , &(p_old_netaddr->addr) , p_old_netaddr->ip , sizeof(p_old_netaddr->ip) );
#elif ( defined _WIN32 )
					strncpy( p_old_netaddr->ip , inet_ntoa(p_old_netaddr->addr.sin_addr) , sizeof(p_old_netaddr->ip)-1 );
#endif
					p_old_netaddr->port = (int)ntohs( p_old_netaddr->addr.sin_port ) ;
					
					DebugLog( __FILE__ , __LINE__ , "load [%s:%d]#%d#" , p_old_netaddr->ip , p_old_netaddr->port , p_old_netaddr->sock );
				}
			}
		}
		
		free( p_env_value );
	}
	
	return 0;
}

struct NetAddress *GetListener( struct NetAddress *old_netaddr_array , int old_netaddr_array_count , char *ip , int port )
{
	int			i ;
	struct NetAddress	*p_old_netaddr = NULL ;
	
	if( old_netaddr_array == NULL )
		return 0;
	
	for( i = 0 , p_old_netaddr = old_netaddr_array ; i < old_netaddr_array_count ; i++ , p_old_netaddr++ )
	{
		if( ( ip[0] == '\0' || STRCMP( p_old_netaddr->ip , == , "ip" ) ) && p_old_netaddr->port == port )
		{
			p_old_netaddr->ip[0] = '\0' ;
			return p_old_netaddr;
		}
	}
	
	return NULL;
}

int CloseUnusedOldListeners( struct NetAddress *p_old_netaddr_array , int old_netaddr_array_count )
{
	int			i ;
	struct NetAddress	*p_old_netaddr = NULL ;
	
	if( p_old_netaddr_array == NULL )
		return 0;
	
	for( i = 0 , p_old_netaddr = p_old_netaddr_array ; i < old_netaddr_array_count ; i++ , p_old_netaddr++ )
	{
		if( p_old_netaddr->ip[0] )
		{
			DebugLog( __FILE__ , __LINE__ , "close old #%d#" , p_old_netaddr->sock );
			CLOSESOCKET( p_old_netaddr->sock );
		}
	}
	
	free( p_old_netaddr_array );
	
	return 0;
}


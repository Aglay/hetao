/*
 * hetao - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "hetao_in.h"

int IncreaseHttpSessions( struct HetaoEnv *p_env , int http_session_incre_count )
{
	struct HttpSession	*p_http_session_array = NULL ;
	struct HttpSession	*p_http_session = NULL ;
	int			i ;
	
	/* �ж��Ƿ񵽴����HTTPͨѶ�Ự���� */
	if( p_env->http_session_used_count >= p_env->limits__max_http_session_count )
	{
		WarnLog( __FILE__ , __LINE__ , "http session count limits[%d]" , p_env->limits__max_http_session_count );
		return 1;
	}
	
	/* �������ӿ���HTTPͨѶ�Ự */
	if( p_env->http_session_used_count + http_session_incre_count > p_env->limits__max_http_session_count )
		http_session_incre_count = p_env->limits__max_http_session_count - p_env->http_session_used_count ;
	
	p_http_session_array = (struct HttpSession *)malloc( sizeof(struct HttpSession) * http_session_incre_count ) ;
	if( p_http_session_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "malloc failed , errno[%d]" , errno );
		return -1;
	}
	memset( p_http_session_array , 0x00 , sizeof(struct HttpSession) * http_session_incre_count );
	
	for( i = 0 , p_http_session = p_http_session_array ; i < http_session_incre_count ; i++ , p_http_session++ )
	{
		p_http_session->http = CreateHttpEnv() ;
		if( p_http_session->http == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "CreateHttpEnv failed , errno[%d]" , errno );
			return -1;
		}
		SetHttpTimeout( p_http_session->http , p_env->http_options__timeout );
		p_http_session->forward_http = CreateHttpEnv() ;
		if( p_http_session->forward_http == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "CreateHttpEnv failed , errno[%d]" , errno );
			return -1;
		}
		SetHttpTimeout( p_http_session->forward_http , p_env->http_options__timeout );
		
		list_add_tail( & (p_http_session->list) , & (p_env->http_session_unused_list.list) );
		DebugLog( __FILE__ , __LINE__ , "init http session[%p] http env[%p]" , p_http_session , p_http_session->http );
	}
	
	p_env->http_session_unused_count += http_session_incre_count ;
	
	return 0;
}

struct HttpSession *FetchHttpSessionUnused( struct HetaoEnv *p_env )
{
	struct HttpSession	*p_http_session = NULL ;
	
	int			nret = 0 ;
	
	if( p_env->http_session_unused_count == 0 )
	{
		/* �������HTTPͨѶ�Ự����Ϊ�� */
		nret = IncreaseHttpSessions( p_env , INCRE_HTTP_SESSION_COUNT ) ;
		if( nret )
			return NULL;
	}
	
	/* �ӿ���HTTPͨѶ�Ự�������Ƴ�һ���Ự��������֮ */
	p_http_session = list_first_entry( & (p_env->http_session_unused_list.list) , struct HttpSession , list ) ;
	list_del( & (p_http_session->list) );
	
	/* ���뵽����HTTPͨѶ�Ự���� */
	p_http_session->timeout_timestamp = GETSECONDSTAMP + p_env->http_options__timeout ;
	nret = AddHttpSessionTimeoutTreeNode( p_env , p_http_session ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AddTimeoutTreeNode failed , errno[%d]" , errno );
		list_add_tail( & (p_http_session->list) , & (p_env->http_session_unused_list.list) );
		return NULL;
	}
	
	/* ���������� */
	p_env->http_session_used_count++;
	p_env->http_session_unused_count--;
	DebugLog( __FILE__ , __LINE__ , "fetch http session[%p] http env[%p]" , p_http_session , p_http_session->http );
	
	return p_http_session;
}

void SetHttpSessionUnused( struct HetaoEnv *p_env , struct HttpSession *p_http_session )
{
	DebugLog( __FILE__ , __LINE__ , "reset http session[%p] http env[%p]" , p_http_session , p_http_session->http );
	
	/* ����HTTPͨѶ�Ự */
	epoll_ctl( p_env->p_this_process_info->epoll_fd , EPOLL_CTL_DEL , p_http_session->netaddr.sock , NULL );
	if( p_http_session->ssl )
	{
		SSL_shutdown( p_http_session->ssl ) ;
		SSL_free( p_http_session->ssl ) ;
		p_http_session->ssl = NULL ;
	}
	close( p_http_session->netaddr.sock );
	ResetHttpEnv( p_http_session->http );
	p_http_session->p_virtualhost = NULL ;
	
	if( p_http_session->p_forward_server )
	{
		if( p_http_session->forward_ssl )
		{
			SSL_shutdown( p_http_session->forward_ssl ) ;
			SSL_free( p_http_session->forward_ssl ) ;
			p_http_session->forward_ssl = NULL ;
		}
		SetHttpSessionUnused_05( p_env , p_http_session );
	}
	
	/* �ѵ�ǰ����HTTPͨѶ�Ự�Ƶ�����HTTPͨѶ�Ự������ */
	RemoveHttpSessionTimeoutTreeNode( p_env , p_http_session );
	list_add_tail( & (p_http_session->list) , & (p_env->http_session_unused_list.list) );
	
	p_env->http_session_used_count--;
	p_env->http_session_unused_count++;
	
	return;
}

void SetHttpSessionUnused_05( struct HetaoEnv *p_env , struct HttpSession *p_http_session )
{
	if( p_http_session->forward_flags )
	{
		SetHttpSessionUnused_02( p_env , p_http_session );
	}
	
	p_http_session->p_forward_server = NULL ;
	
	return;
}

void SetHttpSessionUnused_02( struct HetaoEnv *p_env , struct HttpSession *p_http_session )
{
	epoll_ctl( p_env->p_this_process_info->epoll_fd , EPOLL_CTL_DEL , p_http_session->forward_sock , NULL ) ;
	p_http_session->p_forward_server->connection_count--;
	close( p_http_session->forward_sock );
	ResetHttpEnv( p_http_session->forward_http );
	p_http_session->forward_flags = 0 ;
	
	return;
}

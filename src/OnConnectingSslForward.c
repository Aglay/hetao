/*
 * hetao - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "hetao_in.h"

int OnConnectingSslForward( struct HetaoEnv *p_env , struct HttpSession *p_http_session )
{
#if ( defined __linux ) || ( defined __unix )
	struct epoll_event	event ;
#elif ( defined _WIN32 )
	WSABUF			buf ;
	DWORD			dwFlags ;
#endif
	char			*request_base = NULL ;
	int			request_len ;
	struct HttpBuffer	*forward_b = NULL ;
	
	int			err ;
	
	int			nret = 0 ;
	
	nret = SSL_do_handshake( p_http_session->forward_ssl ) ;
	if( nret == 1 )
	{
		/* 复制HTTP请求 */
		request_base = GetHttpBufferBase( GetHttpRequestBuffer(p_http_session->http) , & request_len ) ;
		forward_b = GetHttpRequestBuffer( p_http_session->forward_http ) ;
		nret = MemcatHttpBuffer( forward_b , request_base , request_len ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "epoll_ctl failed , errno[%d]" , ERRNO );
			return 1;
		}
		
#if ( defined __linux ) || ( defined __unix )
		/* 注册epoll读事件 */
		memset( & event , 0x00 , sizeof(struct epoll_event) );
		event.events = EPOLLOUT | EPOLLERR ;
		event.data.ptr = p_http_session ;
		nret = epoll_ctl( p_env->p_this_process_info->epoll_fd , EPOLL_CTL_MOD , p_http_session->forward_netaddr.sock , & event ) ;
		if( nret == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "epoll_ctl #%d# add #%d# EPOLLOUT failed , errno[%d]" , p_env->p_this_process_info->epoll_fd , p_http_session->forward_netaddr.sock , ERRNO );
			return 1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "epoll_ctl #%d# mod #%d# EPOLLOUT" , p_env->p_this_process_info->epoll_fd , p_http_session->forward_netaddr.sock );
		}
#elif ( defined _WIN32 )
		p_http_session->flag = HTTPSESSION_FLAGS_SENDING ;
		
		/* 设置成堵塞模式 */
		SetHttpBlock( p_http_session->forward_netaddr.sock );
			
#if ( defined _WIN32 )
		/* 创建BIO */
		p_http_session->forward_in_bio = BIO_new(BIO_s_mem()) ;
		p_http_session->forward_out_bio = BIO_new(BIO_s_mem()) ;
		SSL_set_bio( p_http_session->forward_ssl , p_http_session->forward_in_bio , p_http_session->forward_out_bio );
#endif
		
		/* 投递发送事件 */
		forward_b = GetHttpRequestBuffer( p_http_session->forward_http );
		SSL_write( p_http_session->forward_ssl , GetHttpBufferBase( forward_b , NULL ) , GetHttpBufferLength( forward_b ) );
		buf.buf = p_http_session->forward_out_bio_buffer ;
		buf.len = BIO_read( p_http_session->forward_out_bio , p_http_session->forward_out_bio_buffer , sizeof(p_http_session->forward_out_bio_buffer)-1 ) ;
		dwFlags = 0 ;
		nret = WSASend( p_http_session->forward_netaddr.sock , & buf , 1 , NULL , dwFlags , & (p_http_session->overlapped) , NULL ) ;
		if( nret == SOCKET_ERROR )
		{
			if( WSAGetLastError() == ERROR_IO_PENDING )
			{
				DebugLog( __FILE__ , __LINE__ , "WSASend io pending" );
			}
			else
			{
				ErrorLog( __FILE__ , __LINE__ , "WSASend failed , errno[%d]" , ERRNO );
				return 1;
			}
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "WSASend ok" );
		}
#endif
		
		p_http_session->forward_ssl_connected = 1 ;
		
		return 0;
	}
	
	err = SSL_get_error( p_http_session->forward_ssl , nret ) ;
	if( err == SSL_ERROR_WANT_WRITE )
	{
#if ( defined __linux ) || ( defined __unix )
		/* 注册epoll写事件 */
		memset( & event , 0x00 , sizeof(struct epoll_event) );
		event.events = EPOLLOUT | EPOLLERR ;
		event.data.ptr = p_http_session ;
		nret = epoll_ctl( p_env->p_this_process_info->epoll_fd , EPOLL_CTL_MOD , p_http_session->forward_netaddr.sock , & event ) ;
		if( nret == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "epoll_ctl #%d# mod #%d# SSL EPOLLOUT failed , errno[%d]" , p_env->p_this_process_info->epoll_fd , p_http_session->forward_netaddr.sock , ERRNO );
			return 1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "epoll_ctl #%d# add #%d# SSL EPOLLOUT" , p_env->p_this_process_info->epoll_fd , p_http_session->forward_netaddr.sock );
		}
#elif ( defined _WIN32 )
		buf.buf = NULL ;
		buf.len = 0 ;
		dwFlags = 0 ;
		nret = WSASend( p_http_session->forward_netaddr.sock , & buf , 1 , NULL , dwFlags , & (p_http_session->overlapped) , NULL ) ;
		if( nret == SOCKET_ERROR )
		{
			if( WSAGetLastError() == ERROR_IO_PENDING )
			{
				DebugLog( __FILE__ , __LINE__ , "WSASend SSL io pending" );
			}
			else
			{
				ErrorLog( __FILE__ , __LINE__ , "WSASend SSL failed , errno[%d]" , ERRNO );
				return 1;
			}
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "WSASend SSL ok" );
		}
#endif
		
		return 0;
	}
	else if( err == SSL_ERROR_WANT_READ )
	{
#if ( defined __linux ) || ( defined __unix )
		/* 注册epoll读事件 */
		memset( & event , 0x00 , sizeof(struct epoll_event) );
		event.events = EPOLLIN | EPOLLERR ;
		event.data.ptr = p_http_session ;
		nret = epoll_ctl( p_env->p_this_process_info->epoll_fd , EPOLL_CTL_MOD , p_http_session->forward_netaddr.sock , & event ) ;
		if( nret == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "epoll_ctl #%d# mod #%d# SSL EPOLLIN failed , errno[%d]" , p_env->p_this_process_info->epoll_fd , p_http_session->forward_netaddr.sock , ERRNO );
			return 1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "epoll_ctl #%d# mod #%d# SSL EPOLLIN" , p_env->p_this_process_info->epoll_fd , p_http_session->forward_netaddr.sock );
		}
#elif ( defined _WIN32 )
		buf.buf = NULL ;
		buf.len = 0 ;
		dwFlags = 0 ;
		nret = WSARecv( p_http_session->forward_netaddr.sock , & buf , 1 , NULL , & dwFlags , & (p_http_session->overlapped) , NULL ) ;
		if( nret == SOCKET_ERROR )
		{
			if( WSAGetLastError() == ERROR_IO_PENDING )
			{
				DebugLog( __FILE__ , __LINE__ , "WSARecv SSL io pending" );
			}
			else
			{
				ErrorLog( __FILE__ , __LINE__ , "WSARecv SSL failed , errno[%d]" , ERRNO );
				return 1;
			}
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "WSARecv SSL ok" );
		}
#endif
		
		return 0;
	}
	else
	{
		ErrorLog( __FILE__ , __LINE__ , "SSL_get_error[%d]" , err );
		return 1;
	}
}


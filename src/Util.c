/*
 * hetao - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "hetao_in.h"

/* �ѵ�ǰ����ת��Ϊ�ػ����� */
#if ( defined __linux ) || ( defined __unix )
int BindDaemonServer( int (* ServerMain)( void *pv ) , void *pv )
{
	int	pid;
	
	chdir( "/tmp" );
	
	pid = fork() ;
	switch( pid )
	{
		case -1:
			return -1;
		case 0:
			break;
		default		:
			exit( 0 );	
			break;
	}
	
	pid = fork() ;
	switch( pid )
	{
		case -1:
			return -2;
		case 0:
			break ;
		default:
			exit( 0 );
			break;
	}
	
	return ServerMain( pv );
}
#elif ( defined _WIN32 )
#endif

/* ���Ŀ¼���� */
int AccessDirectoryExist( char *pathdirname )
{
	struct STAT	st ;
	int		nret = 0 ;
	
	nret = STAT( pathdirname , & st ) ;
	if( nret == -1 )
		return -1;
	
#if ( defined __linux ) || ( defined __unix )
	if( S_ISDIR(st.st_mode) )
#elif ( defined _WIN32 )
	if( _S_IFDIR & st.st_mode )
#endif
		return 1;
	else
		return 0;
}

/* ����ļ����� */
int AccessFileExist( char *pathfilename )
{
	int		nret = 0 ;
	
	nret = ACCESS( pathfilename , ACCESS_MODE ) ;
	if( nret == -1 )
		return -1;
	
	return 1;
}

#if ( defined __linux ) || ( defined __unix )
/* ��ǰ���̰�CPU */
int BindCpuAffinity( int processor_no )
{
	cpu_set_t	cpu_mask ;
	
	int		nret = 0 ;
	
	CPU_ZERO( & cpu_mask );
	CPU_SET( processor_no , & cpu_mask );
	nret = sched_setaffinity( 0 , sizeof(cpu_mask) , & cpu_mask ) ;
	return nret;
}
#elif ( defined _WIN32 )
#endif

/* ��ϣ���� */
unsigned long CalcHash( char *str , int len )
{
	char		*p = str ;
	char		*p_end = str + len ;
	unsigned long	ul = 0 ;
	
	for( ; p < p_end ; p++ )
	{
		ul  = (*p) + (ul<<6)+ (ul>>16) - ul ;
	}
	
	return ul;
}

#if ( defined _WIN32 )
char *strndup (const char *s, size_t n)
{
  char *result;
  size_t len = strlen (s);

  if (n < len)
    len = n;

  result = (char *) malloc (len + 1);
  if (!result)
    return 0;

  result[len] = '\0';
  return (char *) memcpy (result, s, len);
}
#endif

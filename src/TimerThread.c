/*
 * hetao - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "hetao_in.h"

#if ( defined __linux ) || ( defined __unix )
void *TimerThread( void *pv )
#elif ( defined _WIN32 )
DWORD WINAPI TimerThread( LPVOID lpParameter )
#endif
{
	/* ÿ��һ���ӣ�����д��־������ʱ�仺��������©�¼� */
	while(1)
	{
		UPDATEDATETIMECACHE
		
#if ( defined __linux ) || ( defined __unix )
		sleep(1);
#elif ( defined _WIN32 )
		Sleep(1000);
#endif
		
		g_second_elapse = 1 ;
	}
	
#if ( defined __linux ) || ( defined __unix )
	pthread_exit(NULL);
#elif ( defined _WIN32 )
	ExitThread( 0 );
#endif
}


#ifndef VWB_PLATFORM_INCLUDE_HPP
#define VWB_PLATFORM_INCLUDE_HPP

#define _CRT_SECURE_NO_WARNINGS
#define __Q(x) #x
#define _Q(x) __Q(x)

#if defined( _MSC_VER )
#if _MSC_VER <= 1600
#include <numeric>
#define NAN std::numeric_limits<float>::quiet_NaN()
#define VWB_inet_ntop_cast PVOID
#else
#define VWB_inet_ntop_cast IN_ADDR*
#endif //_MSC_VER <= 1600
#define _inline_ __forceinline
#else
#define _inline_ inline
#define VWB_inet_ntop_cast IN_ADDR*
#endif //defined( _MSC_VER )

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#define NOMINMAX
#include <sdkddkver.h>
#include <windows.h>
#include <stdint.h>
extern HMODULE g_hModDll;

#define sleep( x ) Sleep(x)
typedef HMODULE VWB_Module;
typedef HANDLE VWB_Thread;
typedef DWORD VWB_ThreadID;
typedef LPTHREAD_START_ROUTINE VWB_LPThreadStartRoutine;
#define VWB_startThread( fn, param ) CreateThread( NULL, 0, (VWB_LPThreadStartRoutine)fn, param, 0, NULL )
#define VWB_waitThread( th, timeout ) (WAIT_OBJECT_0 == WaitForSingleObject( th, timeout ))
#define VWB_closeThread( th ) CloseHandle( th )
#define VWB_getMyThreadID GetCurrentThreadId

typedef HANDLE VWB_Event;
#define VWB_createEvent( manual, signaled ) CreateEvent( NULL, manual, signaled, NULL )
#define VWB_setEvent( ev ) SetEvent( ev )
#define VWB_tryEvent( ev, timeout ) (WAIT_OBJECT_0 == WaitForSingleObject( ev, timeout ))
#define VWB_resetEvent( ev ) ResetEvent( ev )
#define VWB_closeEvent( ev ) CloseHandle( ev )

typedef HANDLE VWB_Mutex;
#define VWB_createMutex( locked ) CreateMutex( NULL, (locked) ? TRUE : FALSE, NULL )
#define VWB_enterMutex( mx ) (WAIT_OBJECT_0 == WaitForSingleObject( mx, INFINITE ))
#define VWB_tryEnterMutex( mx ) (WAIT_OBJECT_0 == WaitForSingleObject( mx, 0 ))
#define VWB_waitEnterMutex( mx, t_ms ) (WAIT_OBJECT_0 == WaitForSingleObject( mx, t_ms ))
#define VWB_leaveMutex( mx ) ReleaseMutex( mx )
#define VWB_closeMutex( mx ) CloseHandle( mx )


#elif __APPLE__
#include "mac_compat.h"
#else
#include "linux_compat.h"
#endif

#ifdef WIN32
class VWB_MutexLock
{
private:
	VWB_Mutex m_h;
public:
	VWB_MutexLock( VWB_Mutex h ) : m_h(h) 
	{ 
		if( 0 == m_h ||	!VWB_enterMutex( m_h ) )
			m_h = 0;
	}
	VWB_MutexLock( VWB_Mutex h, uint32_t t_ms ) : m_h(h) 
	{ 
		if( 0 == m_h ||	!VWB_waitEnterMutex( m_h, t_ms ) )
			m_h = 0;
	}
	~VWB_MutexLock() 
	{
		if( 0 != m_h )
			VWB_leaveMutex( m_h );
	}
	operator bool() { return 0 != m_h; }
};
#define VWB_LockedStatement( mx ) if( VWB_MutexLock __ml = VWB_MutexLock(mx) )
#define VWB_TryLockStatement( mx, t_ms ) if( VWB_MutexLock __ml = VWB_MutexLock(mx,t_ms) )

#endif

#endif //ndef VWB_PLATFORM_INCLUDE_HPP



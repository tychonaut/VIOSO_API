//#define SINEWAVE

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <sdkddkver.h>
#include <windows.h>
#include "../../../Include/EyePointProvider.h"
//#include "../../VIOSOWarpBlend/Include/EyePointProvider.h"
#include <math.h>
#include <ctime>

HMODULE g_hModDll = 0;
bool bSineWave = false;

struct Receiver
{
    DWORD beginTime;
};
 
void* CreateEyePointReceiver( char const* szParam )
{
    auto r = new Receiver;
    r->beginTime = ::GetTickCount();
	if( strstr( szParam, "sinewave" ) )
		bSineWave = true;
    return r;
}

// you need to fill the eyepoint with coordinates from the IG coordinate system
bool ReceiveEyePoint(void *receiver, EyePoint* eyePoint)
{
    auto r = static_cast<Receiver*>(receiver);
 
	auto duration = ::GetTickCount() - r->beginTime;
 
	const double tick = 5000.0;

	double ticks = double(duration) / tick;
 
    const double PI = 3.141592653589793e+00;

	const double mm[][12] =
	{
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,1,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,1,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,1,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,1,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,1,0},
	};
 
	eyePoint->x = 0;
	eyePoint->y = 0;
    eyePoint->z = 0;
    eyePoint->yaw = 0; // around y
    eyePoint->roll = 0; // around x
    eyePoint->pitch = 0; // around z

	int d = sizeof(mm)/sizeof(mm[0]);
	ticks = fmod( ticks, d );

	int s = int(ticks);
	double o = fmod( ticks, 1 ) * 2 * PI;

	if( bSineWave )
	{
		eyePoint->x		= mm[s][ 1] + mm[s][ 0] * sin( o );
		eyePoint->y		= mm[s][ 3] + mm[s][ 2] * sin( o );
		eyePoint->z		= mm[s][ 5] + mm[s][ 4] * sin( o );
		eyePoint->yaw	= mm[s][ 7] + mm[s][ 6] * sin( o );
		eyePoint->pitch	= mm[s][ 9] + mm[s][ 8] * sin( o );
		eyePoint->roll  = mm[s][11] + mm[s][10] * sin( o );
	}
	/*
	if( bSineWave )
	{
		switch( int(secs/4) )
		{
		case 0: // moving center - right - center - left - center
			eyePoint->x = sin( PI * secs/8 );
			break;
		case 1: // moving center - right - center - left - center
			eyePoint->x = 1;
			break;
		case 2: // moving center - right - center - left - center
			eyePoint->x = sin( PI * ( secs/8 - 0.5 ) );
			break;
		case 3: // turning center - right - center - left - center (30deg)
			eyePoint->yaw = sin( PI * ( secs/8 - 1.5 ) ) / 6 * PI;
			break;
		case 4: // turning center - right - center - left - center (30deg)
			eyePoint->yaw = PI / 6;
			break;
		case 5: // turning center - right - center - left - center (30deg)
			eyePoint->yaw = sin( PI * ( secs/8 - 2 ) ) / 6 * PI;
			break;
		case 6: // moving center - up - center - down - center
			eyePoint->y = sin( PI * ( secs/8 - 3 ) );
			break;
		case 7: // moving center - up - center - down - center
			eyePoint->y = 1;
			break;
		case 8: // moving center - up - center - down - center
			eyePoint->y = sin( PI * ( secs/8 - 3.5 ) );
			break;
		case 9: // turning center - up - center - down - center (30deg)
			eyePoint->pitch = sin( PI * ( secs/8 - 4.5 ) ) / 6 * PI;
			break;
		case 10: // turning center - up - center - down - center (30deg)
			eyePoint->pitch = PI / 6;
			break;
		case 11: // turning center - up - center - down - center (30deg)
			eyePoint->pitch = sin( PI * ( secs/8 - 5 ) ) / 6 * PI;
			break;
		case 12: // moving center - up - center - down - center
			eyePoint->z = sin( PI * ( secs/8 - 6 ) );
			break;
		case 13: // moving center - up - center - down - center
			eyePoint->z = 1;
			break;
		case 14: // moving center - up - center - down - center
			eyePoint->z = sin( PI * ( secs/8 - 6.5 ) );
			break;
		case 15: // turning center - up - center - down - center (30deg)
			eyePoint->roll = sin( PI * ( secs/8 - 7.5 ) ) / 6 * PI;
			break;
		case 16: // turning center - up - center - down - center (30deg)
			eyePoint->roll = PI / 6;
			break;
		case 17: // turning center - up - center - down - center (30deg)
			eyePoint->roll = sin( PI * ( secs/8 - 8 ) ) / 6 * PI;
			break;
		case 19: // moving center - right - center - left - center
			eyePoint->x = sin( PI * ( secs/8 - 9.5 ) );
			break;
		case 20: // moving center - right - center - left - center
			eyePoint->x = 1;
			eyePoint->yaw = sin( PI * ( secs/8 - 10 ) ) / 6 * PI;
			break;
		case 21: // moving center - right - center - left - center
			eyePoint->x = 1;
			eyePoint->yaw = PI / 6;
			break;
		case 22: // turning center - right - center - left - center (30deg)
			eyePoint->x = 1;
			eyePoint->yaw = sin( PI * ( secs/8 - 10.5 ) ) / 6 * PI;
			break;
		case 23: // turning center - right - center - left - center (30deg)
			eyePoint->x = sin( PI * ( secs/8 - 11 ) );
			break;
		}
	}
	*/
    return true;
}
 
void DeleteEyePointReceiver(void* handle)
{
    delete static_cast<Receiver*>(handle);
}
 

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_hModDll = hModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
    return TRUE;
}


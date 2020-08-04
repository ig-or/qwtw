// dllmain.cpp : Defines the entry point for the DLL application.
#include <windows.h>

void startQWT();
void stopQWT();

int APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		startQWT();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		stopQWT(); 
		break;
	}
	return TRUE;
}


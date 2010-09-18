/*////////////////////////////////////////////////////////////////////////

	gDebugger Loader

	by HyperIris 2010,09,18

	FreeBSD license

	do't ask me what this is, and how to use it.
////////////////////////////////////////////////////////////////////////*/
#include "stdafx.h"
#include <Windows.h>

int __stdcall WinMain(
	__in HINSTANCE hInstance,
	__in_opt HINSTANCE hPrevInstance,
	__in LPSTR lpCmdLine,
	__in int nShowCmd)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si,sizeof(STARTUPINFO));
    ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
    BOOL ret = CreateProcess(L"gDEBugger.exe", L"", NULL, NULL, FALSE, DEBUG_PROCESS, NULL, NULL, &si, &pi);
    if(ret == FALSE)
    {
        MessageBox(NULL, L"Fail to run gDEBugger.exe!", L"", MB_OK);
        return -1;
    }

    DEBUG_EVENT devent;
    while(TRUE)
    {
        if(WaitForDebugEvent(&devent,INFINITE))
        {
            switch(devent.dwDebugEventCode)
            {
            case EXIT_PROCESS_DEBUG_EVENT:
				goto myExit;
            default:
                break;
            }
           ContinueDebugEvent(devent.dwProcessId, devent.dwThreadId, DBG_CONTINUE);
        }
    }
myExit:
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}


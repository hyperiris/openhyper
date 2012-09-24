/********************************************************************
	created:	2007/03/03
	filename: 	HookFuncs.cpp
	author:		HyperIris
	
	product:	Lachesis Shield
	purpose:	Hooked Function
*********************************************************************/
#include <Windows.h>
#include <WinSafer.h>

#include "../Globals.h"
#include "HookFuncs.h"

#include <vector>

using namespace std;

typedef BOOL (_stdcall *OldCreateProcessW)(
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	); 


BOOL _stdcall NewCreateProcessW(
					   LPCWSTR lpApplicationName,
					   LPWSTR lpCommandLine,
					   LPSECURITY_ATTRIBUTES lpProcessAttributes,
					   LPSECURITY_ATTRIBUTES lpThreadAttributes,
					   BOOL bInheritHandles,
					   DWORD dwCreationFlags,
					   LPVOID lpEnvironment,
					   LPCWSTR lpCurrentDirectory,
					   LPSTARTUPINFOW lpStartupInfo,
					   LPPROCESS_INFORMATION lpProcessInformation
					   )
{
#ifdef _DEBUG
	OutputDebugStringW(L"Begin of NewCreateProcessW");
#endif
	BOOL ret = false;
	__try
	{
		// get the SAFER level
		DWORD hSaferLevel = SAFER_LEVELID_FULLYTRUSTED;

		switch(g_ShieldLevel)
		{
		case Disable : 
			hSaferLevel = SAFER_LEVELID_FULLYTRUSTED;
			break;
		case Normal :  
			hSaferLevel = SAFER_LEVELID_NORMALUSER;
			break;
		default	 :	
			break;
		}
		SAFER_LEVEL_HANDLE hAuthzLevel = NULL;
		if (SaferCreateLevel(SAFER_SCOPEID_USER,
			hSaferLevel,
			0, 
			&hAuthzLevel, NULL)) 
		{

			//  Generate the restricted token that we will use.
			HANDLE hToken = NULL;
			if (SaferComputeTokenFromLevel(
				hAuthzLevel,    // SAFER Level handle
				NULL,           // NULL is current thread token.
				&hToken,        // Target token
				0,              // No flags
				NULL))          // Reserved
			{
#ifdef _DEBUG
				OutputDebugStringW(L"Begin of CreateProcessAsUserW");
#endif

				switch(g_ShieldLevel)
				{
				case Disable:
					/*
					ret = ((OldCreateProcessW)(PBYTE)g_Substitute[INDEX_CreateProcessW])(
					lpApplicationName,
					lpCommandLine,
					lpProcessAttributes,
					lpThreadAttributes,
					bInheritHandles,
					dwCreationFlags,
					lpEnvironment,
					lpCurrentDirectory,
					lpStartupInfo,
					lpProcessInformation
					);
					break;
					//*/
				case Normal:
					ret = CreateProcessAsUserW(
						hToken,
						lpApplicationName,
						lpCommandLine,
						lpProcessAttributes,
						lpThreadAttributes,
						bInheritHandles,
						dwCreationFlags,
						lpEnvironment,
						lpCurrentDirectory,
						lpStartupInfo,
						lpProcessInformation
						);
					break;
				default:
					break;
				}


#ifdef _DEBUG
				OutputDebugStringW(L"End of CreateProcessAsUserW");
#endif

			} 
			SaferCloseLevel(hAuthzLevel);			
		} 
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		OutputDebugStringW(L"LShield error: In hook fail.");
		ret = FALSE;
	}
#ifdef _DEBUG
	OutputDebugStringW(L"...Now leave NewCreateProcessW...");
#endif
	return ret;
}

void InitHook(void)
{
	// hook CreateProcessW with NewCreateProcessW
}

void UninitHook(void)
{
	// unhook
}


/********************************************************************
	created:	2007/03/03
	filename: 	HookFuncs.h
	author:		HyperIris
	
	product:	Lachesis Shield
	purpose:	Hooked Function
*********************************************************************/
#pragma once

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
				   );

void InitHook(void);
void UninitHook(void);

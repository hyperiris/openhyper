/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1997 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          LShield.Cpp
   
   Description:   Contains DLLMain and standard OLE COM object creation stuff.

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/
#include <Windows.h>
//#include <GdiPlus.h>
#include <stdio.h>

#include <ole2.h>
#include <comcat.h>
#include <olectl.h>
#include "ClsFact.h"

//using namespace Gdiplus;

/**************************************************************************
   GUID stuff
**************************************************************************/

//this part is only done once
//if you need to use the GUID in another file, just include Guid.h
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "Guid.h"

/**************************************************************************
   private function prototypes
**************************************************************************/

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID);
BOOL RegisterServer(CLSID, LPTSTR);
BOOL RegisterComCat(CLSID, CATID);
BOOL UnregisterServer(CLSID);
BOOL UnregisterComCat(CLSID, CATID);

/**************************************************************************
   global variables
**************************************************************************/

HINSTANCE   g_hInst;
UINT        g_DllRefCount;

/**************************************************************************

   DllMain

**************************************************************************/

extern "C" BOOL WINAPI DllMain(  HINSTANCE hInstance, 
							   DWORD dwReason, 
							   LPVOID lpReserved)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hInstance;
		g_ShieldLevel = Disable;
		g_Hooked = false;
		//GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		break;

	case DLL_PROCESS_DETACH:
		//GdiplusShutdown(gdiplusToken);
		break;
	}

	return TRUE;
}                                 

/**************************************************************************

   DllCanUnloadNow

**************************************************************************/

STDAPI DllCanUnloadNow(void)
{
	return (g_DllRefCount ? S_FALSE : S_OK);
}

/**************************************************************************

   DllGetClassObject

**************************************************************************/

STDAPI DllGetClassObject(  REFCLSID rclsid, 
						 REFIID riid, 
						 LPVOID *ppReturn)
{
	*ppReturn = NULL;

	//if we don't support this classid, return the proper error code
	if(   !IsEqualCLSID(rclsid, CLSID_LachesisDeskBand)) 
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}
		

	//create a CClassFactory object and check it for validity
	CClassFactory *pClassFactory = new CClassFactory(rclsid);
	if(NULL == pClassFactory)
		return E_OUTOFMEMORY;

	//get the QueryInterface return for our return value
	HRESULT hResult = pClassFactory->QueryInterface(riid, ppReturn);

	//call Release to decement the ref count - creating the object set it to one 
	//and QueryInterface incremented it - since its being used externally (not by 
	//us), we only want the ref count to be 1
	pClassFactory->Release();

	//return the result from QueryInterface
	return hResult;
}

/**************************************************************************

   DllRegisterServer

**************************************************************************/

STDAPI DllRegisterServer(void)
{
	//Register the desk band object.
	if(!RegisterServer(CLSID_LachesisDeskBand, TEXT("Lachesis Shield")))
		return SELFREG_E_CLASS;

	//Register the component categories for the desk band object.
	if(!RegisterComCat(CLSID_LachesisDeskBand, CATID_DeskBand))
		return SELFREG_E_CLASS;
	
	/*
	Remove the cache of the deskbands on Windows 2000. This will cause the new 
	deskband to be displayed in the toolbar menu the next time the user brings it 
	up. See KB article Q214842 for more information on this.
	*/
	TCHAR    szSubKey[MAX_PATH];
	TCHAR    szCATID[MAX_PATH];
	LPWSTR   pwszCATID;

	StringFromCLSID(CATID_DeskBand, &pwszCATID);
	if(pwszCATID)
	{
#ifdef UNICODE
		lstrcpyn(szCATID, pwszCATID, ARRAYSIZE(szCATID));
#else
		WideCharToMultiByte( CP_ACP,
			0,
			pwszCATID,
			-1,
			szCATID,
			ARRAYSIZE(szCATID),
			NULL,
			NULL);
#endif

		//free the string
		CoTaskMemFree(pwszCATID);

		wsprintf(szSubKey, TEXT("Component Categories\\%s\\Enum"), szCATID);

		RegDeleteKey(HKEY_CLASSES_ROOT, szSubKey);
	}

	return S_OK;
}

/**************************************************************************

   RegisterServer

**************************************************************************/

typedef struct{
	HKEY  hRootKey;
	LPTSTR szSubKey;//TCHAR szSubKey[MAX_PATH];
	LPTSTR lpszValueName;
	LPTSTR szData;//TCHAR szData[MAX_PATH];
}DOREGSTRUCT, *LPDOREGSTRUCT;

BOOL RegisterServer(CLSID clsid, LPTSTR lpszTitle)
{
	int      i;
	HKEY     hKey;
	LRESULT  lResult;
	DWORD    dwDisp;
	TCHAR    szSubKey[MAX_PATH];
	TCHAR    szCLSID[MAX_PATH];
	TCHAR    szModule[MAX_PATH];
	LPWSTR   pwsz;

	//get the CLSID in string form
	StringFromIID(clsid, &pwsz);

	if(pwsz)
	{
#ifdef UNICODE
		lstrcpyn(szCLSID, pwsz, ARRAYSIZE(szCLSID));
#else
		WideCharToMultiByte( CP_ACP,
			0,
			pwsz,
			-1,
			szCLSID,
			ARRAYSIZE(szCLSID),
			NULL,
			NULL);
#endif

		//free the string
		CoTaskMemFree(pwsz);
	}

	//get this app's path and file name
	GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule));

	DOREGSTRUCT ClsidEntries[] = {HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s"),                  NULL,                   lpszTitle,
		HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"),  NULL,                   szModule,
		HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"),  TEXT("ThreadingModel"), TEXT("Apartment"),
		NULL,                NULL,                               NULL,                   NULL};

	//register the CLSID entries
	for(i = 0; ClsidEntries[i].hRootKey; i++)
	{
		//create the sub key string - for this case, insert the file extension
		wsprintf(szSubKey, ClsidEntries[i].szSubKey, szCLSID);

		lResult = RegCreateKeyEx(  ClsidEntries[i].hRootKey,
			szSubKey,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_WRITE,
			NULL,
			&hKey,
			&dwDisp);

		if(NOERROR == lResult)
		{
			TCHAR szData[MAX_PATH];

			//if necessary, create the value string
			wsprintf(szData, ClsidEntries[i].szData, szModule);

			lResult = RegSetValueEx(   hKey,
				ClsidEntries[i].lpszValueName,
				0,
				REG_SZ,
				(LPBYTE)szData,
				(lstrlen(szData) + 1) * sizeof(TCHAR));
			int i = lstrlen(szData);

			RegCloseKey(hKey);
		}
		else
			return FALSE;
	}

	//If running on NT, register the extension as approved.
	OSVERSIONINFO  osvi;

	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);

	if(VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
	{
		lstrcpyn( szSubKey, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), ARRAYSIZE(szSubKey));

		lResult = RegCreateKeyEx(  HKEY_LOCAL_MACHINE,
			szSubKey,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_WRITE,
			NULL,
			&hKey,
			&dwDisp);

		if(NOERROR == lResult)
		{
			TCHAR szData[MAX_PATH];

			//Create the value string.
			lstrcpyn(szData, lpszTitle, ARRAYSIZE(szData));

			lResult = RegSetValueEx(   hKey,
				szCLSID,
				0,
				REG_SZ,
				(LPBYTE)szData,
				(lstrlen(szData) + 1) * sizeof(TCHAR));

			RegCloseKey(hKey);
		}
		else
			return FALSE;
	}

	return TRUE;
}

/**************************************************************************

   RegisterComCat

**************************************************************************/

BOOL RegisterComCat(CLSID clsid, CATID CatID)
{
	ICatRegister   *pcr;
	HRESULT        hr = S_OK ;

	CoInitialize(NULL);

	hr = CoCreateInstance(  CLSID_StdComponentCategoriesMgr, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_ICatRegister, 
		(LPVOID*)&pcr);

	if(SUCCEEDED(hr))
	{
		hr = pcr->RegisterClassImplCategories(clsid, 1, &CatID);

		pcr->Release();
	}

	CoUninitialize();

	return SUCCEEDED(hr);
}

/**************************************************************************

DllRegisterServer

**************************************************************************/

STDAPI DllUnregisterServer(void)
{
	if(!UnregisterComCat(CLSID_LachesisDeskBand, CATID_DeskBand))
		return SELFREG_E_CLASS;

	if(!UnregisterServer(CLSID_LachesisDeskBand))
		return SELFREG_E_CLASS;

	return S_OK;
}

BOOL UnregisterServer(CLSID clsid)
{
	WCHAR    szSubKey[MAX_PATH] = {0};
	LPWSTR   pwsz;

	//get the CLSID in string form
	StringFromIID(clsid, &pwsz);
	if (pwsz)
	{
//#ifdef UNICODE
		int n = wsprintf(szSubKey, L"CLSID\\%s", pwsz);
//#else
//		sprintf(szSubKey, TEXT("CLSID\\%s"), pwsz);
//#endif
		//OutputDebugStringW(pwsz);
		//OutputDebugString((char*)pwsz);
		CoTaskMemFree(pwsz);
		if (n <= 0)
		{
			return FALSE;
		}
	}
	
	//OutputDebugStringW(szSubKey);
	//LONG ret = 0;
	RegDeleteKeyW(HKEY_CLASSES_ROOT, szSubKey);

	return TRUE;
}

BOOL UnregisterComCat(CLSID clsid, CATID CatID)
{
	ICatRegister *pcr = NULL;
	HRESULT hr = S_OK ;
	
	CoInitialize( NULL );
	hr = CoCreateInstance( CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (LPVOID*)&pcr );

	if( SUCCEEDED(hr) && pcr )
	{
		hr = pcr->UnRegisterClassImplCategories( clsid, 1, &CatID);
		pcr->Release();
	}

	CoUninitialize();

	return SUCCEEDED(hr);
}


#pragma once

//
// Unicode->MultiByte String Functions
//
NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToMultiByteN(
					   PCHAR MbString,
					   ULONG MbSize,
					   PULONG ResultSize,
					   PCWCH UnicodeString,
					   ULONG UnicodeSize
					   );

//
// MultiByte->Unicode String Functions
//
NTSYSAPI
NTSTATUS
NTAPI
RtlMultiByteToUnicodeN(
					   PWCHAR UnicodeString,
					   ULONG UnicodeSize,
					   PULONG ResultSize,
					   PCSTR MbString,
					   ULONG MbSize
					   );

//NlsAnsiCodePage
//NlsMbCodePageTag

USHORT NlsAnsiCodePage = 0; /* exported */
BOOLEAN NlsMbCodePageTag = FALSE; /* exported */


//GdiGetCodePage
DWORD
WINAPI
GdiGetCodePage(HDC hdc);

// GetACP
UINT
WINAPI
GetACP(VOID);
// GetOEMCP
UINT
WINAPI
GetOEMCP(VOID);

// GetCPInfo
BOOL
WINAPI
GetCPInfo(UINT CodePage,
		  LPCPINFO CodePageInfo);

// * GetCPInfoEx
BOOL
WINAPI
GetCPInfoExA(UINT CodePage,
			 DWORD dwFlags,
			 LPCPINFOEXA lpCPInfoEx);

// CreateFileA
HANDLE WINAPI CreateFileA (LPCSTR			lpFileName,
						   DWORD			dwDesiredAccess,
						   DWORD			dwShareMode,
						   LPSECURITY_ATTRIBUTES	lpSecurityAttributes,
						   DWORD			dwCreationDisposition,
						   DWORD			dwFlagsAndAttributes,
						   HANDLE			hTemplateFile);
// CompareStringA
INT WINAPI CompareStringA(LCID lcid, DWORD style,
						  LPCSTR str1, INT len1, LPCSTR str2, INT len2);

// CreateProcessA
BOOL
WINAPI
CreateProcessA(LPCSTR lpApplicationName,
			   LPSTR lpCommandLine,
			   LPSECURITY_ATTRIBUTES lpProcessAttributes,
			   LPSECURITY_ATTRIBUTES lpThreadAttributes,
			   BOOL bInheritHandles,
			   DWORD dwCreationFlags,
			   LPVOID lpEnvironment,
			   LPCSTR lpCurrentDirectory,
			   LPSTARTUPINFOA lpStartupInfo,
			   LPPROCESS_INFORMATION lpProcessInformation);
// CreateProcessW
BOOL
WINAPI
CreateProcessW(LPCWSTR lpApplicationName,
			   LPWSTR lpCommandLine,
			   LPSECURITY_ATTRIBUTES lpProcessAttributes,
			   LPSECURITY_ATTRIBUTES lpThreadAttributes,
			   BOOL bInheritHandles,
			   DWORD dwCreationFlags,
			   LPVOID lpEnvironment,
			   LPCWSTR lpCurrentDirectory,
			   LPSTARTUPINFOW lpStartupInfo,
			   LPPROCESS_INFORMATION lpProcessInformation);

// GetTimeZoneInformation
DWORD
WINAPI
GetTimeZoneInformation(LPTIME_ZONE_INFORMATION lpTimeZoneInformation);

// VerQueryValueA
BOOL WINAPI VerQueryValueA( LPCVOID pBlock, LPCSTR lpSubBlock,
						   LPVOID *lplpBuffer, PUINT puLen );

// GetThreadLocale
LCID WINAPI GetThreadLocale(void);

// GetSystemDefaultLCID
LCID WINAPI GetSystemDefaultLCID(void);

// GetUserDefaultLCID
LCID WINAPI GetUserDefaultLCID(void);

// GetSystemDefaultLangID
LANGID WINAPI GetSystemDefaultLangID(void);

// GetUserDefaultLangID
LANGID WINAPI GetUserDefaultLangID(void)

// GetCommandLineA
LPSTR
WINAPI
GetCommandLineA(VOID);

// GetCommandLineW
LPWSTR
WINAPI
GetCommandLineW(VOID);

// WideCharToMultiByte
INT
WINAPI
WideCharToMultiByte(UINT CodePage,
					DWORD Flags,
					LPCWSTR WideCharString,
					INT WideCharCount,
					LPSTR MultiByteString,
					INT MultiByteCount,
					LPCSTR DefaultChar,
					LPBOOL UsedDefaultChar);

// MultiByteToWideChar
INT
WINAPI
MultiByteToWideChar(UINT CodePage,
					DWORD Flags,
					LPCSTR MultiByteString,
					INT MultiByteCount,
					LPWSTR WideCharString,
					INT WideCharCount);

// DefWindowProcA
LRESULT WINAPI
DefWindowProcA(HWND hWnd,
			   UINT Msg,
			   WPARAM wParam,
			   LPARAM lParam);

// DefMDIChildProcA
LRESULT WINAPI DefMDIChildProcA( HWND hwnd, UINT message,
								WPARAM wParam, LPARAM lParam );

// DefDlgProcA
LRESULT
WINAPI
DefDlgProcA(
			HWND hDlg,
			UINT Msg,
			WPARAM wParam,
			LPARAM lParam);

// DefFrameProcA
LRESULT WINAPI DefFrameProcA( HWND hwnd, HWND hwndMDIClient,
							 UINT message, WPARAM wParam, LPARAM lParam);

// DialogBoxParamA
INT_PTR
WINAPI
DialogBoxParamA(
				HINSTANCE hInstance,
				LPCSTR lpTemplateName,
				HWND hWndParent,
				DLGPROC lpDialogFunc,
				LPARAM dwInitParam);


// DialogBoxIndirectParamA
/*
 * @implemented
 */
INT_PTR
WINAPI
DialogBoxIndirectParamA(
  HINSTANCE hInstance,
  LPCDLGTEMPLATE hDialogTemplate,
  HWND hWndParent,
  DLGPROC lpDialogFunc,
  LPARAM dwInitParam);

// CreateDialogIndirectParamA
HWND
WINAPI
CreateDialogIndirectParamA(
						   HINSTANCE hInstance,
						   LPCDLGTEMPLATE lpTemplate,
						   HWND hWndParent,
						   DLGPROC lpDialogFunc,
						   LPARAM lParamInit);

// CreateDialogParamA
HWND
WINAPI
CreateDialogParamA(
				   HINSTANCE hInstance,
				   LPCSTR lpTemplateName,
				   HWND hWndParent,
				   DLGPROC lpDialogFunc,
				   LPARAM dwInitParam);
// CreateWindowExA
HWND WINAPI
CreateWindowExA(DWORD dwExStyle,
				LPCSTR lpClassName,
				LPCSTR lpWindowName,
				DWORD dwStyle,
				int x,
				int y,
				int nWidth,
				int nHeight,
				HWND hWndParent,
				HMENU hMenu,
				HINSTANCE hInstance,
				LPVOID lpParam);

// CallWindowProcA
LRESULT WINAPI
CallWindowProcA(WNDPROC lpPrevWndFunc,
				HWND hWnd,
				UINT Msg,
				WPARAM wParam,
				LPARAM lParam);

// SetWindowTextA
BOOL WINAPI
SetWindowTextA(HWND hWnd,
			   LPCSTR lpString);

// GetWindowTextA
int WINAPI
GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount);

// SendMessageA
LRESULT WINAPI
SendMessageA(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam);

// SendMessageCallbackA
BOOL
WINAPI
SendMessageCallbackA(
					 HWND hWnd,
					 UINT Msg,
					 WPARAM wParam,
					 LPARAM lParam,
					 SENDASYNCPROC lpCallBack,
					 ULONG_PTR dwData);

// SendMessageTimeoutA
LRESULT
WINAPI
SendMessageTimeoutA(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					UINT fuFlags,
					UINT uTimeout,
					PDWORD_PTR lpdwResult);


// SendNotifyMessageA
BOOL
WINAPI
SendNotifyMessageA(
				   HWND hWnd,
				   UINT Msg,
				   WPARAM wParam,
				   LPARAM lParam);

// PostMessageA
BOOL
WINAPI
PostMessageA(
			 HWND hWnd,
			 UINT Msg,
			 WPARAM wParam,
			 LPARAM lParam);

// SetWindowLongA
LONG
WINAPI
SetWindowLongA(
			   HWND hWnd,
			   int nIndex,
			   LONG dwNewLong);

// GetWindowLongA
LONG
WINAPI
GetWindowLongA ( HWND hWnd, int nIndex );

// GetMenuStringA
int
WINAPI
GetMenuStringA(
			   HMENU hMenu,
			   UINT uIDItem,
			   LPSTR lpString,
			   int nMaxCount,
			   UINT uFlag);

// IsDBCSLeadByte
BOOL WINAPI IsDBCSLeadByte(BYTE);

// CharPrevA
LPSTR WINAPI CharPrevA(_In_ LPCSTR, _In_ LPCSTR);

// CharNextA
LPSTR WINAPI CharNextA(_In_ LPCSTR);

// EnumFontFamiliesExA
int WINAPI
EnumFontFamiliesExA (HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpEnumFontFamExProc,
					 LPARAM lParam, DWORD dwFlags);

// CreateFontIndirectA
HFONT
WINAPI
CreateFontIndirectA(
    CONST LOGFONTA		*lplf
);

// RtlMultiByteToUnicodeSize
NTSTATUS
NTAPI
RtlMultiByteToUnicodeSize(PULONG UnicodeSize,
						  PCSTR MbString,
						  ULONG MbSize);
// RtlUnicodeToMultiByteSize
NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(PULONG MbSize,
						  PCWCH UnicodeString,
						  ULONG UnicodeSize);
// RtlOemToUnicodeN
NTSTATUS NTAPI
RtlOemToUnicodeN (PWCHAR UnicodeString,
				  ULONG UnicodeSize,
				  PULONG ResultSize,
				  PCCH OemString,
				  ULONG OemSize);
// RtlUnicodeToOemN
NTSTATUS NTAPI
RtlUnicodeToOemN (PCHAR OemString,
				  ULONG OemSize,
				  PULONG ResultSize,
				  PCWCH UnicodeString,
				  ULONG UnicodeSize);







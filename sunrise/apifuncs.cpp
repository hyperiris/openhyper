#include "apifuncs.h"

/*
 * @unimplemented
 */
NTSTATUS NTAPI
RtlUnicodeToMultiByteN (PCHAR MbString,
                        ULONG MbSize,
                        PULONG ResultSize,
                        PCWCH UnicodeString,
                        ULONG UnicodeSize)
{
   ULONG Size = 0;
   ULONG i;

   PAGED_CODE_RTL();

   if (NlsMbCodePageTag == FALSE)
   {
      /* single-byte code page */
      Size =  (UnicodeSize > (MbSize * sizeof (WCHAR)))
                 ? MbSize
	         : (UnicodeSize / sizeof (WCHAR));

      if (ResultSize != NULL)
      {
         *ResultSize = Size;
      }

      for (i = 0; i < Size; i++)
      {
         *MbString++ = NlsUnicodeToAnsiTable[*UnicodeString++];
      }
   }
   else
   {
      /* multi-byte code page */
      /* FIXME */

      USHORT WideChar;
      USHORT MbChar;

      for (i = MbSize, Size = UnicodeSize / sizeof(WCHAR); i && Size; i--, Size--)
      {
         WideChar = *UnicodeString++;

         if (WideChar < 0x80)
         {
            *MbString++ = LOBYTE(WideChar);
            continue;
         }

         MbChar = NlsDbcsUnicodeToAnsiTable[WideChar];

         if (!HIBYTE(MbChar))
         {
            *MbString++ = LOBYTE(MbChar);
            continue;
         }

         if (i >= 2)
         {
            *MbString++ = HIBYTE(MbChar);
            *MbString++ = LOBYTE(MbChar);
            i--;
         }
         else break;
      }

      if (ResultSize != NULL)
         *ResultSize = MbSize - i;
   }

   return STATUS_SUCCESS;
}



/*
 * @unimplemented
 */
NTSTATUS NTAPI
RtlMultiByteToUnicodeN(
   OUT PWCHAR UnicodeString,
   IN ULONG UnicodeSize,
   OUT PULONG ResultSize,
   IN PCSTR MbString,
   IN ULONG MbSize)
{
   ULONG Size = 0;
   ULONG i;

   PAGED_CODE_RTL();

   if (NlsMbCodePageTag == FALSE)
   {
      /* single-byte code page */
      if (MbSize > (UnicodeSize / sizeof(WCHAR)))
         Size = UnicodeSize / sizeof(WCHAR);
      else
         Size = MbSize;

      if (ResultSize != NULL)
         *ResultSize = Size * sizeof(WCHAR);

      for (i = 0; i < Size; i++)
         UnicodeString[i] = NlsAnsiToUnicodeTable[(UCHAR)MbString[i]];
   }
   else
   {
      /* multi-byte code page */
      /* FIXME */

      UCHAR Char;
      USHORT LeadByteInfo;
      PCSTR MbEnd = MbString + MbSize;

      for (i = 0; i < UnicodeSize / sizeof(WCHAR) && MbString < MbEnd; i++)
      {
         Char = *(PUCHAR)MbString++;

         if (Char < 0x80)
         {
            *UnicodeString++ = Char;
            continue;
         }

         LeadByteInfo = NlsLeadByteInfo[Char];

         if (!LeadByteInfo)
         {
            *UnicodeString++ = NlsAnsiToUnicodeTable[Char];
            continue;
         }

         if (MbString < MbEnd)
            *UnicodeString++ = NlsLeadByteInfo[LeadByteInfo + *(PUCHAR)MbString++];
      }

      if (ResultSize != NULL)
         *ResultSize = i * sizeof(WCHAR);
   }

   return STATUS_SUCCESS;
}

/*
 * @implemented
 */
DWORD
WINAPI
GdiGetCodePage(HDC hdc)
{
    PDC_ATTR Dc_Attr;
    if (!GdiGetHandleUserData((HGDIOBJ) hdc, GDI_OBJECT_TYPE_DC, (PVOID) &Dc_Attr)) return 0;
    if (Dc_Attr->ulDirty_ & DIRTY_CHARSET) return LOWORD(NtGdiGetCharSet(hdc));
    return LOWORD(Dc_Attr->iCS_CP);
}

/**
 * @name GetACP
 *
 * Get active ANSI code page number.
 *
 * @implemented
 */

UINT
WINAPI
GetACP(VOID)
{
    return AnsiCodePage.CodePageTable.CodePage;
}

/**
 * @name GetOEMCP
 *
 * Get active OEM code page number.
 *
 * @implemented
 */

UINT
WINAPI
GetOEMCP(VOID)
{
    return OemCodePage.CodePageTable.CodePage;
}

/*
 * @implemented
 */
BOOL
WINAPI
GetCPInfo(UINT CodePage,
          LPCPINFO CodePageInfo)
{
    PCODEPAGE_ENTRY CodePageEntry;

    if (!CodePageInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CodePageEntry = IntGetCodePageEntry(CodePage);
    if (CodePageEntry == NULL)
    {
        switch(CodePage)
        {
            case CP_UTF7:
            case CP_UTF8:
                CodePageInfo->DefaultChar[0] = 0x3f;
                CodePageInfo->DefaultChar[1] = 0;
                CodePageInfo->LeadByte[0] = CodePageInfo->LeadByte[1] = 0;
                CodePageInfo->MaxCharSize = (CodePage == CP_UTF7) ? 5 : 4;
                return TRUE;
        }

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (CodePageEntry->CodePageTable.DefaultChar & 0xff00)
    {
        CodePageInfo->DefaultChar[0] = (CodePageEntry->CodePageTable.DefaultChar & 0xff00) >> 8;
        CodePageInfo->DefaultChar[1] = CodePageEntry->CodePageTable.DefaultChar & 0x00ff;
    }
    else
    {
        CodePageInfo->DefaultChar[0] = CodePageEntry->CodePageTable.DefaultChar & 0xff;
        CodePageInfo->DefaultChar[1] = 0;
    }

    if ((CodePageInfo->MaxCharSize = CodePageEntry->CodePageTable.MaximumCharacterSize) == 2)
        memcpy(CodePageInfo->LeadByte, CodePageEntry->CodePageTable.LeadByte, sizeof(CodePageInfo->LeadByte));
    else
        CodePageInfo->LeadByte[0] = CodePageInfo->LeadByte[1] = 0;

    return TRUE;
}

/*
 * @implemented
 */
BOOL
WINAPI
GetCPInfoExA(UINT CodePage,
             DWORD dwFlags,
             LPCPINFOEXA lpCPInfoEx)
{
    CPINFOEXW CPInfo;

    if (!GetCPInfoExW(CodePage, dwFlags, &CPInfo))
        return FALSE;

    /* the layout is the same except for CodePageName */
    memcpy(lpCPInfoEx, &CPInfo, sizeof(CPINFOEXA));

    WideCharToMultiByte(CP_ACP,
                        0,
                        CPInfo.CodePageName,
                        -1,
                        lpCPInfoEx->CodePageName,
                        sizeof(lpCPInfoEx->CodePageName),
                        NULL,
                        NULL);
    return TRUE;
}

/*
 * @implemented
 */
HANDLE WINAPI CreateFileA (LPCSTR			lpFileName,
			    DWORD			dwDesiredAccess,
			    DWORD			dwShareMode,
			    LPSECURITY_ATTRIBUTES	lpSecurityAttributes,
			    DWORD			dwCreationDisposition,
			    DWORD			dwFlagsAndAttributes,
			    HANDLE			hTemplateFile)
{
   PWCHAR FileNameW;
   HANDLE FileHandle;

   TRACE("CreateFileA(lpFileName %s)\n",lpFileName);

   if (!(FileNameW = FilenameA2W(lpFileName, FALSE)))
      return INVALID_HANDLE_VALUE;

   FileHandle = CreateFileW (FileNameW,
			     dwDesiredAccess,
			     dwShareMode,
			     lpSecurityAttributes,
			     dwCreationDisposition,
			     dwFlagsAndAttributes,
			     hTemplateFile);

   return FileHandle;
}

/******************************************************************************
 *           CompareStringA    (KERNEL32.@)
 *
 * Compare two locale sensitive strings.
 *
 * PARAMS
 *  lcid  [I] LCID for the comparison
 *  style [I] Flags for the comparison (NORM_ constants from "winnls.h").
 *  str1  [I] First string to compare
 *  len1  [I] Length of str1, or -1 if str1 is NUL terminated
 *  str2  [I] Second string to compare
 *  len2  [I] Length of str2, or -1 if str2 is NUL terminated
 *
 * RETURNS
 *  Success: CSTR_LESS_THAN, CSTR_EQUAL or CSTR_GREATER_THAN depending on whether
 *           str1 is less than, equal to or greater than str2 respectively.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
INT WINAPI CompareStringA(LCID lcid, DWORD style,
                          LPCSTR str1, INT len1, LPCSTR str2, INT len2)
{
    WCHAR *buf1W = NtCurrentTeb()->StaticUnicodeBuffer;
    WCHAR *buf2W = buf1W + 130;
    LPWSTR str1W, str2W;
    INT len1W, len2W, ret;
    UINT locale_cp = CP_ACP;

    if (!str1 || !str2)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (len1 < 0) len1 = strlen(str1);
    if (len2 < 0) len2 = strlen(str2);

    if (!(style & LOCALE_USE_CP_ACP)) locale_cp = get_lcid_codepage( lcid );

    len1W = MultiByteToWideChar(locale_cp, 0, str1, len1, buf1W, 130);
    if (len1W)
        str1W = buf1W;
    else
    {
        len1W = MultiByteToWideChar(locale_cp, 0, str1, len1, NULL, 0);
        str1W = HeapAlloc(GetProcessHeap(), 0, len1W * sizeof(WCHAR));
        if (!str1W)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
        MultiByteToWideChar(locale_cp, 0, str1, len1, str1W, len1W);
    }
    len2W = MultiByteToWideChar(locale_cp, 0, str2, len2, buf2W, 130);
    if (len2W)
        str2W = buf2W;
    else
    {
        len2W = MultiByteToWideChar(locale_cp, 0, str2, len2, NULL, 0);
        str2W = HeapAlloc(GetProcessHeap(), 0, len2W * sizeof(WCHAR));
        if (!str2W)
        {
            if (str1W != buf1W) HeapFree(GetProcessHeap(), 0, str1W);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
        MultiByteToWideChar(locale_cp, 0, str2, len2, str2W, len2W);
    }

    ret = CompareStringW(lcid, style, str1W, len1W, str2W, len2W);

    if (str1W != buf1W) HeapFree(GetProcessHeap(), 0, str1W);
    if (str2W != buf2W) HeapFree(GetProcessHeap(), 0, str2W);
    return ret;
}

/*
 * FUNCTION: The CreateProcess function creates a new process and its
 * primary thread. The new process executes the specified executable file
 * ARGUMENTS:
 *
 *     lpApplicationName = Pointer to name of executable module
 *     lpCommandLine = Pointer to command line string
 *     lpProcessAttributes = Process security attributes
 *     lpThreadAttributes = Thread security attributes
 *     bInheritHandles = Handle inheritance flag
 *     dwCreationFlags = Creation flags
 *     lpEnvironment = Pointer to new environment block
 *     lpCurrentDirectory = Pointer to current directory name
 *     lpStartupInfo = Pointer to startup info
 *     lpProcessInformation = Pointer to process information
 *
 * @implemented
 */
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
               LPPROCESS_INFORMATION lpProcessInformation)
{
    /* Call the internal (but exported) version */
    return CreateProcessInternalA(0,
                                  lpApplicationName,
                                  lpCommandLine,
                                  lpProcessAttributes,
                                  lpThreadAttributes,
                                  bInheritHandles,
                                  dwCreationFlags,
                                  lpEnvironment,
                                  lpCurrentDirectory,
                                  lpStartupInfo,
                                  lpProcessInformation,
                                  NULL);
}

/*
 * @implemented
 */
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
               LPPROCESS_INFORMATION lpProcessInformation)
{
    /* Call the internal (but exported) version */
    return CreateProcessInternalW(0,
                                  lpApplicationName,
                                  lpCommandLine,
                                  lpProcessAttributes,
                                  lpThreadAttributes,
                                  bInheritHandles,
                                  dwCreationFlags,
                                  lpEnvironment,
                                  lpCurrentDirectory,
                                  lpStartupInfo,
                                  lpProcessInformation,
                                  NULL);
}

/*
 * @implemented
 */
DWORD
WINAPI
GetTimeZoneInformation(LPTIME_ZONE_INFORMATION lpTimeZoneInformation)
{
    NTSTATUS Status;

    DPRINT("GetTimeZoneInformation()\n");

    Status = NtQuerySystemInformation(SystemCurrentTimeZoneInformation,
                                      lpTimeZoneInformation,
                                      sizeof(TIME_ZONE_INFORMATION),
                                      NULL);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return TIME_ZONE_ID_INVALID;
    }

    return TIME_ZoneID(lpTimeZoneInformation);
}

/***********************************************************************
 *           VerQueryValueA              [VERSION.@]
 */
BOOL WINAPI VerQueryValueA( LPCVOID pBlock, LPCSTR lpSubBlock,
                               LPVOID *lplpBuffer, PUINT puLen )
{
    static const char rootA[] = "\\";
    static const char varfileinfoA[] = "\\VarFileInfo\\Translation";
    const VS_VERSION_INFO_STRUCT16 *info = pBlock;

    TRACE("(%p,%s,%p,%p)\n",
                pBlock, debugstr_a(lpSubBlock), lplpBuffer, puLen );

     if (!pBlock)
        return FALSE;

    if (lpSubBlock == NULL || lpSubBlock[0] == '\0')
        lpSubBlock = rootA;

    if ( !VersionInfoIs16( info ) )
    {
        BOOL ret;
        INT len;
        LPWSTR lpSubBlockW;

        len  = MultiByteToWideChar(CP_ACP, 0, lpSubBlock, -1, NULL, 0);
        lpSubBlockW = HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));

        if (!lpSubBlockW)
            return FALSE;

        MultiByteToWideChar(CP_ACP, 0, lpSubBlock, -1, lpSubBlockW, len);

        ret = VersionInfo32_QueryValue(pBlock, lpSubBlockW, lplpBuffer, puLen);

        HeapFree(GetProcessHeap(), 0, lpSubBlockW);

        if (ret && strcasecmp( lpSubBlock, rootA ) && strcasecmp( lpSubBlock, varfileinfoA ))
        {
            /* Set lpBuffer so it points to the 'empty' area where we store
             * the converted strings
             */
            LPSTR lpBufferA = (LPSTR)pBlock + info->wLength + 4;
            DWORD pos = (LPCSTR)*lplpBuffer - (LPCSTR)pBlock;

            len = WideCharToMultiByte(CP_ACP, 0, *lplpBuffer, -1,
                                      lpBufferA + pos, info->wLength - pos, NULL, NULL);
            *lplpBuffer = lpBufferA + pos;
            *puLen = len;
        }
        return ret;
    }

    return VersionInfo16_QueryValue(info, lpSubBlock, lplpBuffer, puLen);
}

/***********************************************************************
 *           GetThreadLocale    (KERNEL32.@)
 *
 * Get the current threads locale.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The LCID currently associated with the calling thread.
 */
LCID WINAPI GetThreadLocale(void)
{
    LCID ret = NtCurrentTeb()->CurrentLocale;
    if (!ret) NtCurrentTeb()->CurrentLocale = ret = GetUserDefaultLCID();
    return ret;
}

/***********************************************************************
 *		GetSystemDefaultLCID (KERNEL32.@)
 *
 * Get the default locale Id for the system.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LCID of the default locale for the system.
 */
LCID WINAPI GetSystemDefaultLCID(void)
{
    LCID lcid;
    NtQueryDefaultLocale( FALSE, &lcid );
    return lcid;
}

/***********************************************************************
 *		GetUserDefaultLCID (KERNEL32.@)
 *
 * Get the default locale Id for the current user.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LCID of the default locale for the current user.
 */
LCID WINAPI GetUserDefaultLCID(void)
{
    LCID lcid;
    NtQueryDefaultLocale( TRUE, &lcid );
    return lcid;
}

/***********************************************************************
 *		GetSystemDefaultLangID (KERNEL32.@)
 *
 * Get the default language Id for the system.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LANGID of the default language for the system.
 */
LANGID WINAPI GetSystemDefaultLangID(void)
{
    return LANGIDFROMLCID(GetSystemDefaultLCID());
}

/***********************************************************************
 *		GetUserDefaultLangID (KERNEL32.@)
 *
 * Get the default language Id for the current user.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LANGID of the default language for the current user.
 */
LANGID WINAPI GetUserDefaultLangID(void)
{
    return LANGIDFROMLCID(GetUserDefaultLCID());
}

/*
 * @implemented
 */
LPSTR
WINAPI
GetCommandLineA(VOID)
{
    return BaseAnsiCommandLine.Buffer;
}

/*
 * @implemented
 */
LPWSTR
WINAPI
GetCommandLineW(VOID)
{
    return BaseUnicodeCommandLine.Buffer;
}

/**
 * @name WideCharToMultiByte
 *
 * Convert a wide-charater string to closest multi-byte equivalent.
 *
 * @param CodePage
 *        Code page to be used to perform the conversion. It can be also
 *        one of the special values (CP_ACP for ANSI code page, CP_MACCP
 *        for Macintosh code page, CP_OEMCP for OEM code page, CP_THREAD_ACP
 *        for thread active code page, CP_UTF7 or CP_UTF8).
 * @param Flags
 *        Additional conversion flags (WC_NO_BEST_FIT_CHARS, WC_COMPOSITECHECK,
 *        WC_DISCARDNS, WC_SEPCHARS, WC_DEFAULTCHAR).
 * @param WideCharString
 *        Points to the wide-character string to be converted.
 * @param WideCharCount
 *        Size in WCHARs of WideCharStr, or 0 if the caller just wants to
 *        know how large WideCharString should be for a successful conversion.
 * @param MultiByteString
 *        Points to the buffer to receive the translated string.
 * @param MultiByteCount
 *        Specifies the size in bytes of the buffer pointed to by the
 *        MultiByteString parameter. If this value is zero, the function
 *        returns the number of bytes required for the buffer.
 * @param DefaultChar
 *        Points to the character used if a wide character cannot be
 *        represented in the specified code page. If this parameter is
 *        NULL, a system default value is used.
 * @param UsedDefaultChar
 *        Points to a flag that indicates whether a default character was
 *        used. This parameter can be NULL.
 *
 * @return Zero on error, otherwise the number of bytes written in the
 *         MultiByteString buffer. Or the number of bytes needed for
 *         the MultiByteString buffer if MultiByteCount is zero.
 *
 * @implemented
 */

INT
WINAPI
WideCharToMultiByte(UINT CodePage,
                    DWORD Flags,
                    LPCWSTR WideCharString,
                    INT WideCharCount,
                    LPSTR MultiByteString,
                    INT MultiByteCount,
                    LPCSTR DefaultChar,
                    LPBOOL UsedDefaultChar)
{
    /* Check the parameters. */
    if (WideCharString == NULL ||
        (MultiByteString == NULL && MultiByteCount > 0) ||
        (PVOID)WideCharString == (PVOID)MultiByteString ||
        MultiByteCount < 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    /* Determine the input string length. */
    if (WideCharCount < 0)
    {
        WideCharCount = lstrlenW(WideCharString) + 1;
    }

    switch (CodePage)
    {
        case CP_UTF8:
            return IntWideCharToMultiByteUTF8(CodePage,
                                              Flags,
                                              WideCharString,
                                              WideCharCount,
                                              MultiByteString,
                                              MultiByteCount,
                                              DefaultChar,
                                              UsedDefaultChar);

        case CP_UTF7:
            if (DefaultChar != NULL || UsedDefaultChar != NULL)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return 0;
            }
            if (Flags)
            {
                SetLastError(ERROR_INVALID_FLAGS);
                return 0;
            }
            return WideCharToUtf7(WideCharString, WideCharCount,
                                  MultiByteString, MultiByteCount);

        case CP_SYMBOL:
            if ((DefaultChar!=NULL) || (UsedDefaultChar!=NULL))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return 0;
            }
            return IntWideCharToMultiByteSYMBOL(Flags,
                                                WideCharString,
                                                WideCharCount,
                                                MultiByteString,
                                                MultiByteCount);

        default:
            return IntWideCharToMultiByteCP(CodePage,
                                            Flags,
                                            WideCharString,
                                            WideCharCount,
                                            MultiByteString,
                                            MultiByteCount,
                                            DefaultChar,
                                            UsedDefaultChar);
   }
}

/**
 * @name MultiByteToWideChar
 *
 * Convert a multi-byte string to wide-charater equivalent.
 *
 * @param CodePage
 *        Code page to be used to perform the conversion. It can be also
 *        one of the special values (CP_ACP for ANSI code page, CP_MACCP
 *        for Macintosh code page, CP_OEMCP for OEM code page, CP_THREAD_ACP
 *        for thread active code page, CP_UTF7 or CP_UTF8).
 * @param Flags
 *        Additional conversion flags (MB_PRECOMPOSED, MB_COMPOSITE,
 *        MB_ERR_INVALID_CHARS, MB_USEGLYPHCHARS).
 * @param MultiByteString
 *        Input buffer.
 * @param MultiByteCount
 *        Size of MultiByteString, or -1 if MultiByteString is NULL
 *        terminated.
 * @param WideCharString
 *        Output buffer.
 * @param WideCharCount
 *        Size in WCHARs of WideCharString, or 0 if the caller just wants
 *        to know how large WideCharString should be for a successful
 *        conversion.
 *
 * @return Zero on error, otherwise the number of WCHARs written
 *         in the WideCharString buffer.
 *
 * @implemented
 */

INT
WINAPI
MultiByteToWideChar(UINT CodePage,
                    DWORD Flags,
                    LPCSTR MultiByteString,
                    INT MultiByteCount,
                    LPWSTR WideCharString,
                    INT WideCharCount)
{
    /* Check the parameters. */
    if (MultiByteString == NULL ||
        (WideCharString == NULL && WideCharCount > 0) ||
        (PVOID)MultiByteString == (PVOID)WideCharString)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    /* Determine the input string length. */
    if (MultiByteCount < 0)
    {
        MultiByteCount = lstrlenA(MultiByteString) + 1;
    }

    switch (CodePage)
    {
        case CP_UTF8:
            return IntMultiByteToWideCharUTF8(Flags,
                                              MultiByteString,
                                              MultiByteCount,
                                              WideCharString,
                                              WideCharCount);

        case CP_UTF7:
            if (Flags)
            {
                SetLastError(ERROR_INVALID_FLAGS);
                return 0;
            }
            return Utf7ToWideChar(MultiByteString, MultiByteCount,
                                  WideCharString, WideCharCount);

        case CP_SYMBOL:
            return IntMultiByteToWideCharSYMBOL(Flags,
                                                MultiByteString,
                                                MultiByteCount,
                                                WideCharString,
                                                WideCharCount);
        default:
            return IntMultiByteToWideCharCP(CodePage,
                                            Flags,
                                            MultiByteString,
                                            MultiByteCount,
                                            WideCharString,
                                            WideCharCount);
    }
}

LRESULT WINAPI
DefWindowProcA(HWND hWnd,
			   UINT Msg,
			   WPARAM wParam,
			   LPARAM lParam)
{
	BOOL Hook, msgOverride = FALSE;
	LRESULT Result = 0;

	LoadUserApiHook();

	Hook = BeginIfHookedUserApiHook();
	if (Hook)
	{
		msgOverride = IsMsgOverride(Msg, &guah.DefWndProcArray);
		if(msgOverride == FALSE)
		{
			EndUserApiHook();
		}
	}

	/* Bypass SEH and go direct. */
	if (!Hook || !msgOverride)
		return RealDefWindowProcA(hWnd, Msg, wParam, lParam);

	_SEH2_TRY
	{
		Result = guah.DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	_SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	}
	_SEH2_END;

	EndUserApiHook();

	return Result;
}

/***********************************************************************
 *		DefMDIChildProcA (USER32.@)
 */
LRESULT WINAPI DefMDIChildProcA( HWND hwnd, UINT message,
                                   WPARAM wParam, LPARAM lParam )
{
    HWND client = GetParent(hwnd);
    MDICLIENTINFO *ci = get_client_info( client );

    TRACE("%p %04x (%s) %08lx %08lx\n", hwnd, message, SPY_GetMsgName(message, hwnd), wParam, lParam);
    hwnd = WIN_GetFullHandle( hwnd );
    if (!ci) return DefWindowProcA( hwnd, message, wParam, lParam );

    switch (message)
    {
    case WM_SETTEXT:
	DefWindowProcA(hwnd, message, wParam, lParam);
	if( ci->hwndChildMaximized == hwnd )
	    MDI_UpdateFrameText( GetParent(client), client, TRUE, NULL );
        return 1; /* success. FIXME: check text length */

    case WM_GETMINMAXINFO:
    case WM_MENUCHAR:
    case WM_CLOSE:
    case WM_SETFOCUS:
    case WM_CHILDACTIVATE:
    case WM_SYSCOMMAND:
    case WM_SHOWWINDOW:
#ifndef __REACTOS__
    case WM_SETVISIBLE:
#endif
    case WM_SIZE:
    case WM_NEXTMENU:
    case WM_SYSCHAR:
    case WM_DESTROY:
        return DefMDIChildProcW( hwnd, message, wParam, lParam );
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}

/*
 * @implemented
 */
LRESULT
WINAPI
DefDlgProcA(
  HWND hDlg,
  UINT Msg,
  WPARAM wParam,
  LPARAM lParam)
{
    DIALOGINFO *dlgInfo;
    WNDPROC dlgproc;
    BOOL result = FALSE;

    /* Perform DIALOGINFO initialization if not done */
    if(!(dlgInfo = DIALOG_get_info( hDlg, TRUE ))) return 0;

    SetWindowLongPtrW( hDlg, DWLP_MSGRESULT, 0 );

    if ((dlgproc = (WNDPROC)GetWindowLongPtrW( hDlg, DWLP_DLGPROC )))
    {
        /* Call dialog procedure */
        result = CallWindowProcA( dlgproc, hDlg, Msg, wParam, lParam );
    }

    if (!result && IsWindow(hDlg))
    {
        /* callback didn't process this message */

        switch(Msg)
        {
            case WM_ERASEBKGND:
            case WM_SHOWWINDOW:
            case WM_ACTIVATE:
            case WM_SETFOCUS:
            case DM_SETDEFID:
            case DM_GETDEFID:
            case WM_NEXTDLGCTL:
            case WM_GETFONT:
            case WM_CLOSE:
            case WM_NCDESTROY:
            case WM_ENTERMENULOOP:
            case WM_LBUTTONDOWN:
            case WM_NCLBUTTONDOWN:
                 return DEFDLG_Proc( hDlg, Msg, wParam, lParam, dlgInfo );
            case WM_INITDIALOG:
            case WM_VKEYTOITEM:
            case WM_COMPAREITEM:
            case WM_CHARTOITEM:
                 break;

            default:
                 return DefWindowProcA( hDlg, Msg, wParam, lParam );
        }
    }
    return DEFDLG_Epilog(hDlg, Msg, wParam, lParam, result, TRUE);
}

/***********************************************************************
 *		DefFrameProcA (USER32.@)
 */
LRESULT WINAPI DefFrameProcA( HWND hwnd, HWND hwndMDIClient,
                                UINT message, WPARAM wParam, LPARAM lParam)
{
    if (hwndMDIClient)
    {
	switch (message)
	{
        case WM_SETTEXT:
            {
                DWORD len = MultiByteToWideChar( CP_ACP, 0, (LPSTR)lParam, -1, NULL, 0 );
                LPWSTR text = HeapAlloc( GetProcessHeap(), 0, len * sizeof(WCHAR) );
                if (text == NULL)
                    return 0;
                MultiByteToWideChar( CP_ACP, 0, (LPSTR)lParam, -1, text, len );
                MDI_UpdateFrameText( hwnd, hwndMDIClient, FALSE, text );
                HeapFree( GetProcessHeap(), 0, text );
            }
            return 1; /* success. FIXME: check text length */

        case WM_COMMAND:
        case WM_NCACTIVATE:
        case WM_NEXTMENU:
        case WM_SETFOCUS:
        case WM_SIZE:
            return DefFrameProcW( hwnd, hwndMDIClient, message, wParam, lParam );
        }
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}

/*
 * @implemented
 */
INT_PTR
WINAPI
DialogBoxParamA(
  HINSTANCE hInstance,
  LPCSTR lpTemplateName,
  HWND hWndParent,
  DLGPROC lpDialogFunc,
  LPARAM dwInitParam)
{
    HWND hwnd;
    HRSRC hrsrc;
    LPCDLGTEMPLATE ptr;
//// ReactOS rev 33532
    if (!(hrsrc = FindResourceA( hInstance, lpTemplateName, (LPCSTR)RT_DIALOG )) ||
        !(ptr = LoadResource(hInstance, hrsrc)))
    {
        SetLastError(ERROR_RESOURCE_NAME_NOT_FOUND);
        return -1;
    }
    if (hWndParent != NULL && !IsWindow(hWndParent))
    {
        SetLastError(ERROR_INVALID_WINDOW_HANDLE);
        return 0;
    }
    hwnd = DIALOG_CreateIndirect(hInstance, ptr, hWndParent, lpDialogFunc, dwInitParam, FALSE, TRUE);
    if (hwnd) return DIALOG_DoDialogBox(hwnd, hWndParent);
    return -1;
}


//////////////////////////////////////////////////////////////////////////
/**
 * @name IntIsLeadByte
 *
 * Internal function to detect if byte is lead byte in specific character
 * table.
 */

static BOOL
WINAPI
IntIsLeadByte(PCPTABLEINFO TableInfo, BYTE Byte)
{
    UINT i;

    if (TableInfo->MaximumCharacterSize == 2)
    {
        for (i = 0; i < MAXIMUM_LEADBYTES && TableInfo->LeadByte[i]; i += 2)
        {
            if (Byte >= TableInfo->LeadByte[i] && Byte <= TableInfo->LeadByte[i+1])
                return TRUE;
        }
    }

    return FALSE;
}

/**
 * @name IsDBCSLeadByteEx
 *
 * Determine if passed byte is lead byte in current ANSI code page.
 *
 * @implemented
 */

BOOL
WINAPI
IsDBCSLeadByte(BYTE TestByte)
{
    return IntIsLeadByte(&AnsiCodePage.CodePageTable, TestByte);
}

/*
 * @implemented
 */
LPSTR
WINAPI
CharPrevA(LPCSTR start, LPCSTR ptr)
{
    while (*start && (start < ptr)) {
        LPCSTR next = CharNextA(start);
        if (next >= ptr) break;
        start = next;
    }
    return (LPSTR)start;
}


/*
 * @implemented
 */
LPSTR
WINAPI
CharNextA(LPCSTR ptr)
{
    if (!*ptr) return (LPSTR)ptr;
    if (IsDBCSLeadByte(ptr[0]) && ptr[1]) return (LPSTR)(ptr + 2);
    return (LPSTR)(ptr + 1);
}

/*
 * @implemented
 */
int WINAPI
EnumFontFamiliesExA (HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpEnumFontFamExProc,
                     LPARAM lParam, DWORD dwFlags)
{
    LOGFONTW LogFontW, *pLogFontW;

    if (lpLogfont)
    {
        LogFontA2W(&LogFontW,lpLogfont);
        pLogFontW = &LogFontW;
    }
    else pLogFontW = NULL;

    /* no need to convert LogFontW back to lpLogFont b/c it's an [in] parameter only */
    return IntEnumFontFamilies(hdc, pLogFontW, lpEnumFontFamExProc, lParam, FALSE);
}

/*
 * @implemented
 */
HFONT
WINAPI
CreateFontIndirectA(
    CONST LOGFONTA		*lplf
)
{
    if (lplf)
    {
        LOGFONTW tlf;

        LogFontA2W(&tlf, lplf);
        return CreateFontIndirectW(&tlf);
    }
    else return NULL;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
RtlMultiByteToUnicodeSize(PULONG UnicodeSize,
                          PCSTR MbString,
                          ULONG MbSize)
{
    ULONG Length = 0;

    PAGED_CODE_RTL();

    if (!NlsMbCodePageTag)
    {
        /* single-byte code page */
        *UnicodeSize = MbSize * sizeof (WCHAR);
    }
    else
    {
        /* multi-byte code page */
        /* FIXME */

        while (MbSize--)
        {
            UCHAR Char = *(PUCHAR)MbString++;

            if (Char >= 0x80 && NlsLeadByteInfo[Char])
            {
                if (MbSize)
                {
                    /* Move on */
                    MbSize--;
                    MbString++;
                }
            }

            /* Increase returned size */
            Length++;
        }

        /* Return final size */
        *UnicodeSize = Length * sizeof(WCHAR);
    }

    /* Success */
    return STATUS_SUCCESS;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(PULONG MbSize,
                          PCWCH UnicodeString,
                          ULONG UnicodeSize)
{
    ULONG UnicodeLength = UnicodeSize / sizeof(WCHAR);
    ULONG MbLength = 0;

    if (!NlsMbCodePageTag)
    {
        /* single-byte code page */
        *MbSize = UnicodeLength;
    }
    else
    {
        /* multi-byte code page */
        /* FIXME */

        while (UnicodeLength--)
        {
            USHORT WideChar = *UnicodeString++;

            if (WideChar >= 0x80 && HIBYTE(NlsDbcsUnicodeToAnsiTable[WideChar]))
            {
                MbLength += sizeof(WCHAR);
            }
            else
            {
                MbLength++;
            }
        }

        *MbSize = MbLength;
    }

    /* Success */
    return STATUS_SUCCESS;
}


/*
 * @unimplemented
 */
NTSTATUS NTAPI
RtlOemToUnicodeN (PWCHAR UnicodeString,
                  ULONG UnicodeSize,
                  PULONG ResultSize,
                  PCCH OemString,
                  ULONG OemSize)
{
   ULONG Size = 0;
   ULONG i;

   if (NlsMbOemCodePageTag == FALSE)
   {
      /* single-byte code page */
      if (OemSize > (UnicodeSize / sizeof(WCHAR)))
         Size = UnicodeSize / sizeof(WCHAR);
      else
         Size = OemSize;

      if (ResultSize != NULL)
         *ResultSize = Size * sizeof(WCHAR);

      for (i = 0; i < Size; i++)
      {
         *UnicodeString = NlsOemToUnicodeTable[(UCHAR)*OemString];
         UnicodeString++;
         OemString++;
      }
   }
   else
   {
      /* multi-byte code page */
      /* FIXME */

      UCHAR Char;
      USHORT OemLeadByteInfo;
      PCCH OemEnd = OemString + OemSize;

      for (i = 0; i < UnicodeSize / sizeof(WCHAR) && OemString < OemEnd; i++)
      {
         Char = *(PUCHAR)OemString++;

         if (Char < 0x80)
         {
            *UnicodeString++ = Char;
            continue;
         }

         OemLeadByteInfo = NlsOemLeadByteInfo[Char];

         if (!OemLeadByteInfo)
         {
            *UnicodeString++ = NlsOemToUnicodeTable[Char];
            continue;
         }

         if (OemString < OemEnd)
            *UnicodeString++ =
               NlsOemLeadByteInfo[OemLeadByteInfo + *(PUCHAR)OemString++];
      }

      if (ResultSize != NULL)
         *ResultSize = i * sizeof(WCHAR);
   }

   return STATUS_SUCCESS;
}


/*
 * @unimplemented
 */
NTSTATUS NTAPI
RtlUnicodeToOemN (PCHAR OemString,
                  ULONG OemSize,
                  PULONG ResultSize,
                  PCWCH UnicodeString,
                  ULONG UnicodeSize)
{
   ULONG Size = 0;
   ULONG i;

   if (NlsMbOemCodePageTag == FALSE)
   {
      /* single-byte code page */
      if (UnicodeSize > (OemSize * sizeof(WCHAR)))
         Size = OemSize;
      else
         Size = UnicodeSize / sizeof(WCHAR);

      if (ResultSize != NULL)
         *ResultSize = Size;

      for (i = 0; i < Size; i++)
      {
         *OemString = NlsUnicodeToOemTable[*UnicodeString];
         OemString++;
         UnicodeString++;
      }
   }
   else
   {
      /* multi-byte code page */
      /* FIXME */

      USHORT WideChar;
      USHORT OemChar;

      for (i = OemSize, Size = UnicodeSize / sizeof(WCHAR); i && Size; i--, Size--)
      {
         WideChar = *UnicodeString++;

         if (WideChar < 0x80)
         {
            *OemString++ = LOBYTE(WideChar);
            continue;
         }

         OemChar = NlsDbcsUnicodeToOemTable[WideChar];

         if (!HIBYTE(OemChar))
         {
            *OemString++ = LOBYTE(OemChar);
            continue;
         }

         if (i >= 2)
         {
            *OemString++ = HIBYTE(OemChar);
            *OemString++ = LOBYTE(OemChar);
            i--;
         }
         else break;
      }

      if (ResultSize != NULL)
         *ResultSize = OemSize - i;
   }

   return STATUS_SUCCESS;
}



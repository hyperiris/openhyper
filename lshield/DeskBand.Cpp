/********************************************************************
	created:	2007/03/03
	filename: 	DeskBand.cpp
	author:		HyperIris
	
	product:	Lachesis Shield
	purpose:	DeskBand Main source
*********************************************************************/
#include <Windows.h>
#include <GdiPlus.h>
#include "Globals.h"
#include "hook/HookFuncs.h"

#include "DeskBand.h"
#include "Guid.h"

#include <Uxtheme.h>
#include <vssym32.h>

#ifdef WIN7SHIELD
#include <dwmapi.h>     // DWM APIs
#endif

#include "resource.h"

using namespace Gdiplus;

#define RECTWIDTH(x)   ((x).right - (x).left)
#define RECTHEIGHT(x)  ((x).bottom - (x).top)

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken;

CDeskBand::CDeskBand()
{
	m_pSite = NULL;

	m_hWnd = NULL;
	m_hwndParent = NULL;

	m_bFocus = FALSE;

	m_dwViewMode = 0;
	m_dwBandID = 0;

	m_ObjRefCount = 1;
	g_DllRefCount++;

	m_iconSafe = 0;
	m_iconUnsafe = 0;
	m_iconWarn = 0;

	m_taskBarHeight = 23;

	m_bFloating = FALSE;

	m_bComposited = TRUE;

	if (!g_Hooked)
	//if (g_DllRefCount == 1)
	{
		//HANDLE hMutex = CreateMutex(NULL,FALSE, "LachesisShieldMutex");  //启动互斥线程对象
		//DWORD WaitResult := WaitForSingleObject(hMutex,10);      //检验是否有实例在运行
		//if (WaitResult != WAIT_TIMEOUT)
		{
			InitHook();
			g_Hooked = true;
		}
	}
}

CDeskBand::~CDeskBand()
{
	if (g_Hooked)
	//if (g_DllRefCount == 1)
	{
		//UninitHook();
		//g_Hooked = false;
	}
	//this should have been freed in a call to SetSite(NULL), but just to be safe
	if(m_pSite)
	{
		m_pSite->Release();
		m_pSite = NULL;
	}

	g_DllRefCount--;
}

STDMETHODIMP CDeskBand::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;

	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
	{
		*ppReturn = this;
	}

	//IOleWindow
	else if(IsEqualIID(riid, IID_IOleWindow))
	{
		*ppReturn = (IOleWindow*)this;
	}
///*
	//IDockingWindow
	else if(IsEqualIID(riid, IID_IDockingWindow))
	{
		*ppReturn = (IDockingWindow*)this;
	}   
//*/
	//IInputObject
	else if(IsEqualIID(riid, IID_IInputObject))
	{
		*ppReturn = (IInputObject*)this;
	}   

	//IObjectWithSite
	else if(IsEqualIID(riid, IID_IObjectWithSite))
	{
		*ppReturn = (IObjectWithSite*)this;
	}   

	//IDeskBand
	else if(IsEqualIID(riid, IID_IDeskBand))
	{
		*ppReturn = (IDeskBand*)this;
	}   
	
	//IDeskBand2
	else if(IsEqualIID(riid, IID_IDeskBand2))
	{
		*ppReturn = (IDeskBand2*)this;
	}   

	//IPersist
	else if(IsEqualIID(riid, IID_IPersist))
	{
		*ppReturn = (IPersist*)this;
	}   

	//IPersistStream
	else if(IsEqualIID(riid, IID_IPersistStream))
	{
		// disable drag off from taskbar
		*ppReturn = (IPersistStream*)this;
	}   

	//IPersistStreamInit
	//else if(IsEqualIID(riid, IID_IPersistStreamInit))
	{
		// disable drag off from taskbar
		//*ppReturn = (IPersistStreamInit*)this;
	}   

	//IContextMenu
	//else if(IsEqualIID(riid, IID_IContextMenu))
	{
		//*ppReturn = (IContextMenu*)this;
	}   

	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CDeskBand::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CDeskBand::Release()
{
	if(--m_ObjRefCount == 0)
	{
		delete this;
		return 0;
	}

	return m_ObjRefCount;
}

STDMETHODIMP CDeskBand::GetWindow(HWND *phWnd)
{
	*phWnd = m_hWnd;

	return S_OK;
}

STDMETHODIMP CDeskBand::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDeskBand::ShowDW(BOOL fShow)
{
	if(m_hWnd)
	{
		if(fShow)
		{
			ShowWindow(m_hWnd, SW_SHOW);
		}
		else
		{
			ShowWindow(m_hWnd, SW_HIDE);
		}
	}

	return S_OK;
}

STDMETHODIMP CDeskBand::CloseDW(DWORD dwReserved)
{
	ShowDW(FALSE);

	if(IsWindow(m_hWnd))
		DestroyWindow(m_hWnd);

	GdiplusShutdown(gdiplusToken);
	m_hWnd = NULL;

	// 让 Shell_TrayWnd 释放不使用的 DeskBand 动态链接库
	HWND hwndTray = FindWindow(TEXT("Shell_TrayWnd"), NULL);
	PostMessage(hwndTray, WM_TIMER, 24, 0);

	return S_OK;
}

STDMETHODIMP CDeskBand::ResizeBorderDW(LPCRECT prcBorder, IUnknown* punkSite, BOOL fReserved)
{
	/*
	This method is never called for Band Objects.
	*/
	OutputDebugString(L"Never Call !?");
	return E_NOTIMPL;
}

STDMETHODIMP CDeskBand::UIActivateIO(BOOL fActivate, LPMSG pMsg)
{
	if(fActivate)
	{
		SetFocus(m_hWnd);
	}
	return S_OK;
}

STDMETHODIMP CDeskBand::HasFocusIO(void)
{
	if(m_bFocus)
	{
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CDeskBand::TranslateAcceleratorIO(LPMSG pMsg)
{
	return S_FALSE;
}

STDMETHODIMP CDeskBand::SetSite(IUnknown* punkSite)
{
	//If a site is being held, release it.
	if(m_pSite)
	{
		m_pSite->Release();
		m_pSite = NULL;
	}

	//If punkSite is not NULL, a new site is being set.
	if(punkSite)
	{
		//Get the parent window.
		IOleWindow  *pOleWindow;

		m_hwndParent = NULL;

		if(SUCCEEDED(punkSite->QueryInterface(IID_IOleWindow, (LPVOID*)&pOleWindow)))
		{
			pOleWindow->GetWindow(&m_hwndParent);
			pOleWindow->Release();
		}

		if(!m_hwndParent)
			return E_FAIL;

		if(!RegisterAndCreateWindow())
			return E_FAIL;

		//Get and keep the IInputObjectSite pointer.
		if(SUCCEEDED(punkSite->QueryInterface(IID_IInputObjectSite, (LPVOID*)&m_pSite)))
		{
			return S_OK;
		}

		return E_FAIL;
	}

	return S_OK;
}

STDMETHODIMP CDeskBand::GetSite(REFIID riid, LPVOID *ppvReturn)
{
	*ppvReturn = NULL;

	if(m_pSite)
	{
		return m_pSite->QueryInterface(riid, ppvReturn);
	}
	return E_FAIL;
}

STDMETHODIMP CDeskBand::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi)
{
	DWORD height = 22;
	RECT rect;
	HWND hWnd = FindWindow(L"Shell_TrayWnd", NULL);
	if (hWnd)
	{
		GetWindowRect(hWnd, &rect);
		height = rect.bottom - rect.top;
	}

	if(pdbi)
	{
		if (DBIF_VIEWMODE_FLOATING & dwViewMode)
		{
			m_bFloating = TRUE;
		} 
		else
		{
			m_bFloating = FALSE;
		}

		m_dwBandID = dwBandID;
		m_dwViewMode = dwViewMode;

		if(pdbi->dwMask & DBIM_MINSIZE)
		{
			if(DBIF_VIEWMODE_FLOATING & dwViewMode)
			{
				pdbi->ptMinSize.x = -1;//DB_MIN_SIZE_Y;
				pdbi->ptMinSize.y = -1;//DB_MIN_SIZE_X;//DB_MIN_SIZE_Y;
			}
			else
			{
				pdbi->ptMinSize.x = DB_MIN_SIZE_X;
				
				if (GetUserDefaultLangID() != 0x804)
				{
					pdbi->ptMinSize.x += 6;
				}
				pdbi->ptMinSize.y = DB_MIN_SIZE_Y;
			}
		}

		if(pdbi->dwMask & DBIM_MAXSIZE)
		{
			if(DBIF_VIEWMODE_FLOATING & dwViewMode)
			{
				/*
				if (DBIF_VIEWMODE_VERTICAL & dwViewMode)
				{
					pdbi->ptMaxSize.x = 35;//DB_MIN_SIZE_Y;
					//pdbi->ptMaxSize.y = 80;//DB_MIN_SIZE_X;//DB_MIN_SIZE_Y;
					pdbi->ptMaxSize.y = 95;
				}
				else
				//*/
				{
					pdbi->ptMaxSize.x = 95;//DB_MIN_SIZE_Y;
					//pdbi->ptMaxSize.y = 80;//DB_MIN_SIZE_X;//DB_MIN_SIZE_Y;
					pdbi->ptMaxSize.y = 35;
				}
			}
			else
			{
				pdbi->ptMaxSize.x = DB_MIN_SIZE_X;//-1;
				
				if (GetUserDefaultLangID() != 0x804)
				{
					pdbi->ptMinSize.x += 6;
				}
				pdbi->ptMaxSize.y = -1;
				//pdbi->ptMaxSize.y = 23;
				//pdbi->ptMaxSize.y = DB_MIN_SIZE_Y;//-1;
			}
		}

		if(pdbi->dwMask & DBIM_INTEGRAL)
		{
			pdbi->ptIntegral.x = 1;
			pdbi->ptIntegral.y = 1;
		}

		if(pdbi->dwMask & DBIM_ACTUAL)
		{
			RECT  rc;
			GetClientRect(m_hwndParent, &rc);

			//pdbi->ptActual.x = DB_MIN_SIZE_X;
			//pdbi->ptActual.y = DB_MIN_SIZE_Y;

			if(DBIF_VIEWMODE_FLOATING & dwViewMode)
			{
				pdbi->ptActual.x = 90;//DB_MIN_SIZE_Y;
				pdbi->ptActual.y = 30;//DB_MIN_SIZE_X;//DB_MIN_SIZE_Y;
			}
			else
			{
				pdbi->ptActual.x = DB_MIN_SIZE_X;//-1;

				if (GetUserDefaultLangID() != 0x804)
				{
					pdbi->ptActual.x += 6;
				}
				pdbi->ptActual.y = DB_MIN_SIZE_Y;
			}

			//pdbi->ptActual.y = rc.bottom - rc.top;

		}

		if(pdbi->dwMask & DBIM_TITLE)
		{
			//lstrcpyW(pdbi->wszTitle, L"Sample Desk Band");
			pdbi->dwMask &= ~DBIM_TITLE;
		}

		if(pdbi->dwMask & DBIM_MODEFLAGS)
		{
			pdbi->dwModeFlags = DBIMF_FIXED;
			//pdbi->dwModeFlags = DBIMF_NORMAL; // | DBIMF_FIXED;

			//pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;
		}

		if(pdbi->dwMask & DBIM_BKCOLOR)
		{
			//Use the default background color by removing this flag.
			pdbi->dwMask &= ~DBIM_BKCOLOR;
		}

		return S_OK;
	}

	return E_INVALIDARG;
}

STDMETHODIMP CDeskBand::GetClassID(LPCLSID pClassID)
{
	*pClassID = CLSID_LachesisDeskBand;

	return S_OK;
}

STDMETHODIMP CDeskBand::IsDirty(void)
{
	return S_FALSE;
}

STDMETHODIMP CDeskBand::Load(LPSTREAM pStream)
{
	return S_OK;
}

STDMETHODIMP CDeskBand::Save(LPSTREAM pStream, BOOL fClearDirty)
{
	return S_OK;
}

STDMETHODIMP CDeskBand::GetSizeMax(ULARGE_INTEGER *pul)
{
	if (pul)
	{
		pul->HighPart = 0;
		pul->LowPart = 0;
	}
	return S_OK;
	//return E_NOTIMPL;
}

STDMETHODIMP CDeskBand::InitNew(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDeskBand::QueryContextMenu( HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	return E_NOTIMPL;
	/*
	if(!(CMF_DEFAULTONLY & uFlags))
	{
		InsertMenu( hMenu, 
			indexMenu, 
			MF_STRING | MF_BYPOSITION, 
			idCmdFirst + IDM_COMMAND, 
			"&Desk Band Command");

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_COMMAND + 1));
	}

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	*/
}

STDMETHODIMP CDeskBand::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
/*
	switch (LOWORD(lpcmi->lpVerb))
	{
	case IDM_COMMAND:
		MessageBox(lpcmi->hwnd, TEXT("Desk Band Command selected."), TEXT("Sample Desk Band"), MB_OK | MB_ICONINFORMATION);
		break;

	default:
		return E_INVALIDARG;
	}
*/
	return NOERROR;
}

STDMETHODIMP CDeskBand::GetCommandString( UINT_PTR idCommand, UINT uFlags, LPUINT lpReserved, LPSTR lpszName, UINT uMaxNameLen)
{
	return E_NOTIMPL;
/*
	HRESULT  hr = E_INVALIDARG;

	switch(uFlags)
	{
	case GCS_HELPTEXT:
		switch(idCommand)
		{
		case IDM_COMMAND:
			lstrcpyn(lpszName, TEXT("Desk Band command help text"), uMaxNameLen);
			hr = NOERROR;
			break;
		}
		break;

	case GCS_VERB:
		switch(idCommand)
		{
		case IDM_COMMAND:
			lstrcpyn(lpszName, TEXT("command"), uMaxNameLen);
			hr = NOERROR;
			break;
		}
		break;

	case GCS_VALIDATE:
		hr = NOERROR;
		break;
	}

	return hr;
*/
}

LRESULT CALLBACK CDeskBand::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	CDeskBand  *pThis = (CDeskBand*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMessage)
	{
	case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			pThis = (CDeskBand*)(lpcs->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

			//set the window handle
			pThis->m_hWnd = hWnd;
		}
		break;
	case WM_CREATE:
		{
			pThis->m_iconSafe = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SAFE));
			pThis->m_iconUnsafe = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_UNSAFE));
			pThis->m_iconWarn = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_WARN));

			//SetTimer(hWnd, ID_TIMER, TIME_LENGTH, NULL);
/*
			MARGINS mar = {100, 100, 100, 100};
			DwmExtendFrameIntoClientArea ( hWnd, &mar );

			EnableThemeDialogTexture ( hWnd, ETDT_ENABLE );
*/
			OutputDebugString(L"REG HOT KEY\n");
			pThis->m_Atom = GlobalAddAtom(SHIELD_HOT_KEY_ID);
			pThis->m_Atom2 = GlobalAddAtom(SHIELD_HOT_KEY_ID2);
			RegisterHotKey(pThis->m_hWnd, pThis->m_Atom, SHIELD_HOT_KEY, 'Z');
			RegisterHotKey(pThis->m_hWnd, pThis->m_Atom2, SHIELD_HOT_KEY, 'L');
		}
		break;
	//case WM_CLOSE:
/*
	case WM_INITDIALOG:
		{
			OutputDebugString(L"WM_INITDIALOG\n");
			pThis->m_Atom = GlobalAddAtom(SHIELD_HOT_KEY_ID);
			pThis->m_Atom2 = GlobalAddAtom(SHIELD_HOT_KEY_ID2);
			RegisterHotKey(pThis->m_hWnd, pThis->m_Atom, SHIELD_HOT_KEY, 'Z');
			RegisterHotKey(pThis->m_hWnd, pThis->m_Atom2, SHIELD_HOT_KEY, 'L');
		}
		break;
*/
	case WM_PAINT:
		return pThis->OnPaint();

	case WM_COMMAND:
		return pThis->OnCommand(wParam, lParam);

	case WM_SETFOCUS:
		return pThis->OnSetFocus();

	case WM_KILLFOCUS:
		return pThis->OnKillFocus();

	case WM_SIZE:
		return pThis->OnSize(lParam);

	case WM_HOTKEY:
		OutputDebugString(L"WM_HOTKEY\n");
	case WM_LBUTTONUP:
		return pThis->OnMouseUp();

	case WM_ERASEBKGND:
		return TRUE;

	case WM_TIMER:
		return pThis->OnTimer();
	case WM_DESTROY:
		{
			//KillTimer(hWnd, ID_TIMER); 
			UnregisterHotKey(pThis->m_hWnd, pThis->m_Atom);
			UnregisterHotKey(pThis->m_hWnd, pThis->m_Atom2);

			GlobalDeleteAtom(pThis->m_Atom);
			GlobalDeleteAtom(pThis->m_Atom2);
		}
		break; 
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* pInfo = (MINMAXINFO*)lParam;
			if (pThis->m_bFloating)
			{
				pInfo->ptMinTrackSize.x = 60;
				pInfo->ptMinTrackSize.y = 22;
				pInfo->ptMaxTrackSize.x = 120;
				pInfo->ptMaxTrackSize.y = 40;
			}
		}
		return S_OK;
	case WM_THEMECHANGED:
		{
			OutputDebugString(L"WM_THEMECHANGED\n");
		}
	}

	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

/**************************************************************************

   CDeskBand::OnPaint()
   
**************************************************************************/

LRESULT CDeskBand::OnPaint(void)
{
	PAINTSTRUCT ps;
	RECT        rc;

	HDC hdc = BeginPaint(m_hWnd, &ps);
	GetClientRect(m_hWnd, &rc);

	try
	{
		//WCHAR szWBuffer[] = L"暴露";
		WCHAR szWBuffer[MAX_PATH] = {0};

		switch(g_ShieldLevel)
		{
		case Disable:
			LoadString(g_hInst, IDS_DISABLE, szWBuffer, MAX_PATH);
			break;
		case Normal:
			LoadString(g_hInst, IDS_NORMAL, szWBuffer, MAX_PATH);
			break;
		case High:
			LoadString(g_hInst, IDS_HIGH, szWBuffer, MAX_PATH);
			break;
		default:
			break;
		}
//*
		if (IsThemeActive())
		{
#ifdef WIN7SHIELD
			HTHEME hTheme = OpenThemeData(NULL, L"BUTTON");
#else
			HTHEME hTheme = OpenThemeData(m_hWnd, L"TASKBAR");
#endif // WIN7SHIELD

			if (hTheme)
#ifdef WIN7SHIELD
			{
				SIZE size;
				HDC hdcPaint = NULL;
				HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, NULL, &hdcPaint);

				DrawThemeParentBackground(m_hWnd, hdcPaint, &rc);

				Graphics    graphics(hdcPaint);
				RectF		rectF(0.0f, 4.0f, 16.0f, 16.0f);

				switch(g_ShieldLevel)
				{
				case Disable:
					{
						Bitmap icon(m_iconUnsafe);		
						graphics.DrawImage(&icon, rectF);
						//DrawIconEx(hdcPaint, 2, ((rc.bottom - rc.top) - 16) / 2, m_iconUnsafe, 16, 16, 0, NULL, DI_IMAGE);
						break;
					}
				case Normal:
					{
						Bitmap icon(m_iconSafe);		
						graphics.DrawImage(&icon, rectF);
						//DrawIconEx(hdcPaint, 2, ((rc.bottom - rc.top) - 16) / 2, m_iconSafe, 16, 16, 0, NULL, DI_NORMAL);
						break;
					}
				default:
					break;
				}	

				NONCLIENTMETRICS ncMetrics = {};
				ncMetrics.cbSize = sizeof(NONCLIENTMETRICS);
				SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncMetrics, 0);

				HFONT hFont, hFontOld;
				hFont = CreateFontIndirect(&ncMetrics.lfCaptionFont);
				hFontOld = (HFONT)SelectObject(hdcPaint, hFont);       //选择字体

				GetTextExtentPointW(hdc, szWBuffer, wcslen(szWBuffer), &size);
				RECT rcText;
				rcText.left   = (RECTWIDTH(rc) - size.cx) / 2 + 16;
				rcText.top    = (RECTHEIGHT(rc) - size.cy) / 2;
				rcText.right  = rcText.left + size.cx;
				rcText.bottom = rcText.top + size.cy;

				DTTOPTS dttOpts = {sizeof(dttOpts)};
				dttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR | DTT_GLOWSIZE;
				switch (g_ShieldLevel)
				{
				case Disable:
					dttOpts.crText = RGB(255, 0, 0);
					break;
				case Normal:
					dttOpts.crText = RGB(255, 255, 255);
					break;
				default:
					break;
				}
				dttOpts.iGlowSize = 10;
				DrawThemeTextEx(hTheme, hdcPaint, 0, 0, szWBuffer, -1, 0, &rcText, &dttOpts);

				SelectObject(hdcPaint, hFontOld);                      //选择旧字体
				DeleteObject(hFont);                              //删除新字体

				EndBufferedPaint(hBufferedPaint, TRUE);

				CloseThemeData(hTheme);
			}
#else
			{
				DWORD taskBarPos = TBP_BACKGROUNDBOTTOM;
				/*
				if (m_dwViewMode & DBIF_VIEWMODE_NORMAL)
				{
				taskBarPos = TBP_BACKGROUNDBOTTOM;
				} 
				if (m_dwViewMode & DBIF_VIEWMODE_VERTICAL)
				{
				taskBarPos = TBP_BACKGROUNDRIGHT;
				}
				//*/
				///*
				RECT parentRect;
				GetClientRect(m_hwndParent, &parentRect);

				if ((parentRect.top == 0) && (parentRect.left == 0) && (parentRect.bottom >= parentRect.right))
				{
				taskBarPos = TBP_BACKGROUNDLEFT;
				}
				if ((parentRect.top == 0) && (parentRect.left == 0) && (parentRect.bottom <= parentRect.right))
				{
				taskBarPos = TBP_BACKGROUNDTOP;
				}
				if ((parentRect.top == 0) && (parentRect.left != 0))
				{
				taskBarPos = TBP_BACKGROUNDRIGHT;
				}
				if ((parentRect.top != 0) && (parentRect.left == 0))
				{
				taskBarPos = TBP_BACKGROUNDBOTTOM;
				}
				//* /
				//DrawThemeBackground(hTheme, ps.hdc, /*TBP_BACKGROUNDBOTTOM |* / TBP_BACKGROUNDRIGHT, 0, &rc, NULL);
				DrawThemeBackground(hTheme, ps.hdc, taskBarPos, 0, &rc, NULL);

				Graphics    graphics(ps.hdc);
				RectF		rectF(2.0f, 8.0f, 16.0f, 16.0f);

				switch(g_ShieldLevel)
				{
				case Disable:
					{
						Bitmap icon(m_iconUnsafe);		
						graphics.DrawImage(&icon, rectF);
						//DrawIconEx(hdcPaint, 2, ((rc.bottom - rc.top) - 16) / 2, m_iconUnsafe, 16, 16, 0, NULL, DI_IMAGE);
						break;
					}
				case Normal:
					{
						Bitmap icon(m_iconSafe);		
						graphics.DrawImage(&icon, rectF);
						//DrawIconEx(hdcPaint, 2, ((rc.bottom - rc.top) - 16) / 2, m_iconSafe, 16, 16, 0, NULL, DI_NORMAL);
						break;
					}
				default:
					break;
				}	

				SolidBrush  brushWhite(Color(255, 255, 255, 255));
				SolidBrush  brushRed(Color(255, 255, 0, 0));
				float fontSize = 12.0f;
				float fontTop = 7.0;
				//FontFamily  fontFamily;

				FontFamily* fontFamily;
				switch(GetUserDefaultLangID())
				{
				case 0x0804:
					fontFamily = new FontFamily(L"宋体");
					fontSize = 12.0f;
					fontTop = ((rc.bottom - rc.top) - 14.0f) / 2.0f; //7.0;
					break;
				default:
					fontFamily = new FontFamily(L"Tahoma");
					fontSize = 10.0f;
					fontTop = 5.0;
					break;
				}

				//FontFamily  fontFamily(L"Times New Roman");
				Font        font(fontFamily, 12, FontStyleRegular, UnitPixel);
				PointF      pointF(16.0f, fontTop);

				switch(g_ShieldLevel)
				{
				case Disable:
					DrawIconEx(ps.hdc, 2, ((rc.bottom - rc.top) - 16) / 2, m_iconUnsafe, 16, 16, 0, NULL, DI_NORMAL);
					graphics.DrawString(szWBuffer, -1, &font, pointF, &brushRed);
					break;
				case Normal:
					DrawIconEx(ps.hdc, 2, ((rc.bottom - rc.top) - 16) / 2, m_iconSafe, 16, 16, 0, NULL, DI_NORMAL);
					graphics.DrawString(szWBuffer, -1, &font, pointF, &brushWhite);
					break;
				default:
					break;
				}	
				delete fontFamily;
			}
#endif // WIN7SHIELD
		} 
		else
		{
			FillRect(ps.hdc, &rc, (HBRUSH)(COLOR_BTNFACE+1));

			Graphics    graphics(ps.hdc);

			SolidBrush  brushWhite(Color(255, 255, 255, 255));
			SolidBrush  brushRed(Color(255, 255, 0, 0));
			float fontSize = 12.0f;
			float fontTop = 7.0;
			//FontFamily  fontFamily;

			FontFamily* fontFamily;
			switch(GetUserDefaultLangID())
			{
			case 0x0804:
				fontFamily = new FontFamily(L"宋体");
				fontSize = 12.0f;
				fontTop = ((rc.bottom - rc.top) - 14.0f) / 2.0f; //7.0;
				break;
			default:
				fontFamily = new FontFamily(L"Tahoma");
				fontSize = 10.0f;
				fontTop = 5.0;
				break;
			}

			//FontFamily  fontFamily(L"Times New Roman");
			Font        font(fontFamily, 12, FontStyleRegular, UnitPixel);
			PointF      pointF(16.0f, fontTop);

			switch(g_ShieldLevel)
			{
			case Disable:
				DrawIconEx(ps.hdc, 2, ((rc.bottom - rc.top) - 16) / 2, m_iconUnsafe, 16, 16, 0, NULL, DI_NORMAL);
				graphics.DrawString(szWBuffer, -1, &font, pointF, &brushRed);
				break;
			case Normal:
				DrawIconEx(ps.hdc, 2, ((rc.bottom - rc.top) - 16) / 2, m_iconSafe, 16, 16, 0, NULL, DI_NORMAL);
				graphics.DrawString(szWBuffer, -1, &font, pointF, &brushWhite);
				break;
			default:
				break;
			}	
			delete fontFamily;
		}
	}
	catch (...)
	{
		OutputDebugStringW(L"LShield error: Fail drawing UI.");
	}
	EndPaint(m_hWnd, &ps);
	return 0;
}

/**************************************************************************

   CDeskBand::OnCommand()
   
**************************************************************************/

LRESULT CDeskBand::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

/**************************************************************************

   CDeskBand::OnSize()
   
**************************************************************************/

LRESULT CDeskBand::OnSize(LPARAM lParam)
{
	int   cx, cy;

	cx = LOWORD(lParam);
	cy = HIWORD(lParam);
	return 0;
}

/**************************************************************************

   CDeskBand::FocusChange()
   
**************************************************************************/

void CDeskBand::FocusChange(BOOL bFocus)
{
	m_bFocus = bFocus;

	//inform the input object site that the focus has changed
	if(m_pSite)
	{
		m_pSite->OnFocusChangeIS((IDockingWindow*)this, bFocus);
	}
}

/**************************************************************************

   CDeskBand::OnSetFocus()
   
**************************************************************************/

LRESULT CDeskBand::OnSetFocus(void)
{
	FocusChange(TRUE);

	return 0;
}

/**************************************************************************

   CDeskBand::OnKillFocus()
   
**************************************************************************/

LRESULT CDeskBand::OnKillFocus(void)
{
	FocusChange(FALSE);

	return 0;
}

/**************************************************************************

CDeskBand::OnMouseUp()

**************************************************************************/

LRESULT CDeskBand::OnMouseUp()
{
	//::MessageBox(m_hWnd, "test", "info", MB_OK);

	g_ShieldLevel++;
	// High 好像用不到，目前先做两种
	if (g_ShieldLevel == High)
	{
		g_ShieldLevel = Disable;
	}

	RECT        rc;
	GetClientRect(m_hWnd, &rc);
	InvalidateRect(m_hWnd, &rc, TRUE);

	//SetTimer(m_hWnd, ID_TIMER, TIME_LENGTH, NULL);
	return 0;
}

/**************************************************************************

   CDeskBand::RegisterAndCreateWindow()
   
**************************************************************************/

BOOL CDeskBand::RegisterAndCreateWindow(void)
{
	//If the window doesn't exist yet, create it now.
	if(!m_hWnd)
	{
		//Can't create a child window without a parent.
		if(!m_hwndParent)
		{
			return FALSE;
		}

		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		//If the window class has not been registered, then do so.
		WNDCLASS wc;
		if(!GetClassInfo(g_hInst, DB_CLASS_NAME, &wc))
		{
			ZeroMemory(&wc, sizeof(wc));
			wc.style          = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
			wc.lpfnWndProc    = (WNDPROC)WndProc;
			wc.cbClsExtra     = 0;
			wc.cbWndExtra     = 0;
			wc.hInstance      = g_hInst;
			wc.hIcon          = NULL;
			wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground  = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			wc.lpszMenuName   = NULL;
			wc.lpszClassName  = DB_CLASS_NAME;

			if(!RegisterClass(&wc))
			{
				//If RegisterClass fails, CreateWindow below will fail.
			}
		}

		//RECT  rc;
		//GetClientRect(m_hwndParent, &rc);
/*
		DWORD height = DB_MIN_SIZE_Y;
		RECT rect;
		HWND hWnd = FindWindow(L"Shell_TrayWnd", NULL);
		if (hWnd)
		{
			GetWindowRect(hWnd, &rect);
			//height = rect.bottom - rect.top;
		}
*/
		//Create the window. The WndProc will set m_hWnd.
		CreateWindowEx(   0,
			DB_CLASS_NAME,
			NULL,
			WS_CHILD | WS_CLIPCHILDREN  | WS_CLIPSIBLINGS,// | WS_BORDER,
			0,//rc.left,
			0,//rc.top,
#ifdef WIN7SHIELD
			//0,
			//0,
			DB_MIN_SIZE_X,
			DB_MIN_SIZE_Y,
#else
			DB_MIN_SIZE_X,
			DB_MIN_SIZE_Y,
#endif // WIN7SHIELD
			m_hwndParent,
			NULL,
			g_hInst,
			(LPVOID)this);

		//m_taskBarHeight = rc.bottom - rc.top;
		m_taskBarHeight = DB_MIN_SIZE_Y;
	}
	return (NULL != m_hWnd);
}

LRESULT CDeskBand::OnTimer()
{
	g_ShieldLevel = Normal;

	RECT        rc;
	GetClientRect(m_hWnd, &rc);
	InvalidateRect(m_hWnd, &rc, TRUE);

	//SetTimer(m_hWnd, ID_TIMER, TIME_LENGTH, NULL);

	return S_OK;
}

#ifdef WIN7SHIELD
HRESULT CDeskBand::CanRenderComposited( BOOL *pfCanRenderComposited )
{
	*pfCanRenderComposited = TRUE;
	return S_OK;
}

HRESULT CDeskBand::GetCompositionState( BOOL *pfCompositionEnabled )
{
	*pfCompositionEnabled = m_bComposited;
	return S_OK;
}

HRESULT CDeskBand::SetCompositionState( BOOL fCompositionEnabled )
{
	m_bComposited = fCompositionEnabled;

	RECT        rc;
	GetClientRect(m_hWnd, &rc);
	InvalidateRect(m_hWnd, &rc, TRUE);

	return S_OK;
}
#endif // WIN7SHIELD
/*
LRESULT CDeskBand::OnHotKey()
{
	g_ShieldLevel++;
	// High 好像用不到，目前先做两种
	if (g_ShieldLevel == High)
	{
		g_ShieldLevel = Disable;
	}

	RECT        rc;
	GetClientRect(m_hWnd, &rc);
	InvalidateRect(m_hWnd, &rc, TRUE);

	//SetTimer(m_hWnd, ID_TIMER, TIME_LENGTH, NULL);
	return 0;
}
*/

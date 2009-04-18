// bi_resetDlg.cpp : implementation file
//

#include "stdafx.h"

#include <TlHelp32.h>

#include "bi_reset.h"
#include "bi_resetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CResetDlg dialog

CResetDlg::CResetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResetDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CResetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_COUNT, m_Label);
	DDX_Control(pDX, IDC_STATIC_INFO, m_Info);
	DDX_Control(pDX, IDC_BUTTON_RESET, m_Button);
}

BEGIN_MESSAGE_MAP(CResetDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CResetDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CResetDlg::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_MIN, &CResetDlg::OnBnClickedButtonMin)
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CResetDlg message handlers

BOOL CResetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_Reset = FALSE;
	SetTimer(1, 200, NULL);

	//CMenu *pMenu = ::AfxGetMainWnd()->GetSystemMenu(false);
	//pMenu->EnableMenuItem(SC_CLOSE,MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CResetDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_BI_RESET_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_BI_RESET_DIALOG));
	}
}
#endif


void CResetDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString info = L"BlueInput not found...";
	CString label = L"Can't get data...";
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS, 0);
	if (hProcessSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		label = L"Try to find BI...";
		if (Process32First(hProcessSnap, &pe32))
		{
			do 
			{
				LPTSTR name = NULL;
				TCHAR buffer[MAX_PATH] = {0};
				if (_tcsstr( pe32.szExeFile,L"\\"))
				{
					name = _tcsrchr(pe32.szExeFile,'\\');
				}
				else
				{
					name = pe32.szExeFile;
				}
				_tcscpy(buffer, name);
				_tcslwr(buffer);
				if (_tcscmp(buffer, L"btdrvtray.exe") == 0)
				{
					info = L"BlueInput found...";

					HANDLE hProcess = OpenProcess(0, FALSE, pe32.th32ProcessID);
					if (hProcess != INVALID_HANDLE_VALUE)
					{
						DWORD address = 0x000262D0;
						//DWORD address = 0xD0;
						DWORD data = 0;
						DWORD size = 4;
						DWORD count = 0;
						if (!m_Reset)
						{
							m_Button.SetWindowText(L"Reset");
							if (ReadProcessMemory(hProcess, (LPVOID)address, &data, size, &count) && (count == 4))
							{
								label.Format(L"%d times pressed...", data);
							}
							else
							{
								label = L"Fail to get key press count...";
							}
						}
						else
						{
							m_Button.SetWindowText(L"Stop");
							if (WriteProcessMemory(hProcess, (LPVOID)address, &data, size, &count) && (count == 4))
							{
								label = L"Key press count reset to 0...";
							}
							else
							{
								label = L"Fail to reset key press count...";
							}			
						}
						CloseHandle(hProcess);
					}
				}
			} while (Process32Next(hProcessSnap, &pe32));
		}
		CloseToolhelp32Snapshot(hProcessSnap);
	}
	m_Info.SetWindowText(info);
	m_Label.SetWindowText(label);
	CDialog::OnTimer(nIDEvent);
}

void CResetDlg::OnBnClickedButtonReset()
{
	// TODO: Add your control notification handler code here
	m_Reset = !m_Reset;
}

void CResetDlg::OnBnClickedButtonExit()
{
	// TODO: Add your control notification handler code here
	EndDialog(TRUE);
}

void CResetDlg::OnBnClickedButtonMin()
{
	// TODO: Add your control notification handler code here
	ShowWindow(SW_MINIMIZE);
	ShowWindow(SW_HIDE);
}

void CResetDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: Add your message handler code here and/or call default
	/*
	if (nID == SC_CLOSE)  
	{
		PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
		return;
	}
	*/
	CDialog::OnSysCommand(nID, lParam);
}


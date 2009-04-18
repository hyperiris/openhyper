// bi_resetDlg.h : header file
//

#pragma once
#include "afxwin.h"

// CResetDlg dialog
class CResetDlg : public CDialog
{
// Construction
public:
	CResetDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BI_RESET_DIALOG };


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_Label;
	BOOL m_Reset;
	CStatic m_Info;
	CButton m_Button;
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonMin();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};

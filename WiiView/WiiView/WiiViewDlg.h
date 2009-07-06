// WiiViewDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CWiiViewDlg dialog
class CWiiViewDlg : public CDialog
{
// Construction
public:
	CWiiViewDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WIIVIEW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonOpen();
	void LoadIso();
	CEdit m_edtDiskType;
	CEdit m_edtGameCode;
	CEdit m_edtRegion;
	CEdit m_edtMakerCode;
	CEdit m_edtGameTite;
	CEdit m_edtPartCount;

	CString csIsoPathName;

	TIsoHead IsoHead;
	TPartitionsInfo PartitionsInfo;
	TPartitionsTableItem PartitionsTableItem;
	TFirstCertificationItem FirstCertificationItem;

	AES_KEY aes_key;
	DWORD vec[4];
	DWORD nullvec[4];

	TFSTItem* fst;

	CComboBox m_cbPart;
	CImageList m_imgList;
	unsigned char partitionKey[16];
	__int64 realDataOffset;

	afx_msg void OnCbnSelchangeComboPart();
	CButton m_btnOpen;
	void ListPartFiles(void);
	void FillTreeView(TFSTItem* fst, DWORD fstSize);
	CTreeCtrl m_tvMain;
	void AddTreeViewItem(HTREEITEM base, TFSTItem* fst, int begin, int count, char* nameTableBase);
	void PartitionRead(__int64 offset, unsigned char*data, DWORD len);
	void PartitionReadBlock(__int64 blockno, unsigned char*block);
	afx_msg void OnDestroy();
	afx_msg void OnTvnSelchangedTreeMain(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonExtract();
	CButton m_btnExtract;
};

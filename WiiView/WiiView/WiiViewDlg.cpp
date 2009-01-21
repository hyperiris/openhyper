// WiiViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WiiView.h"
#include "WiiViewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CWiiViewDlg dialog




CWiiViewDlg::CWiiViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWiiViewDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWiiViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITDISKTYPE, m_edtDiskType);
	DDX_Control(pDX, IDC_EDITGAMECODE, m_edtGameCode);
	DDX_Control(pDX, IDC_EDITREGION, m_edtRegion);
	DDX_Control(pDX, IDC_EDITMAKERCODE, m_edtMakerCode);
	DDX_Control(pDX, IDC_EDITGAMETITLE, m_edtGameTite);
	DDX_Control(pDX, IDC_EDITPARTCOUNT, m_edtPartCount);
	DDX_Control(pDX, IDC_COMBO_PART, m_cbPart);
	DDX_Control(pDX, IDC_BUTTON_OPEN, m_btnOpen);
	DDX_Control(pDX, IDC_TREE_MAIN, m_tvMain);
	DDX_Control(pDX, IDC_BUTTON_EXTRACT, m_btnExtract);
}

BEGIN_MESSAGE_MAP(CWiiViewDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CWiiViewDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CWiiViewDlg::OnBnClickedButtonOpen)
	ON_CBN_SELCHANGE(IDC_COMBO_PART, &CWiiViewDlg::OnCbnSelchangeComboPart)
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_MAIN, &CWiiViewDlg::OnTvnSelchangedTreeMain)
	ON_BN_CLICKED(IDC_BUTTON_EXTRACT, &CWiiViewDlg::OnBnClickedButtonExtract)
END_MESSAGE_MAP()


// CWiiViewDlg message handlers

BOOL CWiiViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	FILE* MasterKeyFile = fopen("ckey.bin", "rb");
	if (MasterKeyFile)
	{
		fseek(MasterKeyFile, 0, SEEK_SET);
		fread(masterKey, 1, 16, MasterKeyFile);
		fclose(MasterKeyFile);
	}
	else
	{
		m_btnOpen.EnableWindow(FALSE);
	}
	m_imgList.Create(IDB_BITMAP_TREE, 16, 2, 0xff00ff);
	m_tvMain.SetImageList(&m_imgList, TVSIL_NORMAL);

	fst = NULL;

	m_btnExtract.EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWiiViewDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWiiViewDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWiiViewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWiiViewDlg::OnBnClickedButtonAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CWiiViewDlg::OnBnClickedButtonOpen()
{
	// TODO: Add your control notification handler code here
	// open
	CFileDialog diaOpenIso(TRUE, "iso");
	
	if (diaOpenIso.DoModal() == IDOK)
	{
		csIsoPathName = diaOpenIso.GetPathName();
		LoadIso();
	}
}

void CWiiViewDlg::LoadIso()
{

	FILE* IsoFile = fopen(csIsoPathName, "rb");

	fseek(IsoFile, OFFSET_ISO_HEAD, SEEK_SET);
	fread(&IsoHead, 1, sizeof(TIsoHead), IsoFile);

	fseek(IsoFile, OFFSET_PARTITIONS_INFO, SEEK_SET);
	fread(&PartitionsInfo, 1, sizeof(TPartitionsInfo), IsoFile);

	switch(IsoHead.diskType)
	{
	case 'R':
		m_edtDiskType.SetWindowText("Wii");
		break;
	case 'G':
		m_edtDiskType.SetWindowText("GameCube");
		break;
	case 'U':
		m_edtDiskType.SetWindowText("Utility");
	    break;
	default:
		m_edtDiskType.SetWindowText("Unknown");
	    break;
	}
	CString csGameCode(IsoHead.gameCode[0]);
	csGameCode += IsoHead.gameCode[1];
	m_edtGameCode.SetWindowText(csGameCode);
	
	switch(IsoHead.region)
	{
	case 'E':
		m_edtRegion.SetWindowText("USA");
		break;
	case 'P':
		m_edtRegion.SetWindowText("PAL");
		break;
	case 'J':
		m_edtRegion.SetWindowText("JAP");
	    break;
	default:
		m_edtRegion.SetWindowText("Unknown");
	    break;
	}
	
	CString csMakerCode(IsoHead.makerCode[0]);
	csMakerCode += IsoHead.makerCode[1];
	m_edtMakerCode.SetWindowText(csMakerCode);

	m_edtGameTite.SetWindowText(IsoHead.gameTitle);

	CString csPartCount;
	csPartCount.Format("%d", toDWORD(PartitionsInfo.partitionsCount));
	m_edtPartCount.SetWindowText(csPartCount);

	for (int i = 0; i < toDWORD(PartitionsInfo.partitionsCount); i++)
	{
		CString csPart;
		csPart.Format("Partition %d", i);
		m_cbPart.AddString(csPart);
	}
	

	fclose(IsoFile);
}

void CWiiViewDlg::OnCbnSelchangeComboPart()
{
	// 

	if (m_cbPart.GetCurSel() == -1)
	{
		return;
	}
	if (fst)
	{
		free(fst);
		fst = NULL;
	}
	m_btnExtract.EnableWindow(FALSE);
	m_tvMain.DeleteAllItems();
	ListPartFiles();
}

void CWiiViewDlg::ListPartFiles(void)
{
	DWORD partTableOffset;
	DWORD partOffset;

	__int64 dataOffset;
	__int64 dataSize;

	unsigned char partDataHead[0x480];
	__int64 fstOffset;
	DWORD fstSize;
	

	FILE* IsoFile = fopen(csIsoPathName, "rb");

	//partCount = toDWORD(PartitionsInfo.partitionsCount);
	partTableOffset = toDWORD(PartitionsInfo.partitionTableOffset);

	_fseeki64(IsoFile, partTableOffset << 2, SEEK_SET);
	_fseeki64(IsoFile, sizeof(TPartitionsInfo) * m_cbPart.GetCurSel(), SEEK_CUR);

	fread(&PartitionsTableItem, 1, sizeof(TPartitionsTableItem), IsoFile);

	partOffset = toDWORD(PartitionsTableItem.partitionOffset);
	//printf("Partition offset %010I64X\n", partOffset << 2);

	_fseeki64(IsoFile, partOffset << 2, SEEK_SET);
	fread(&FirstCertificationItem, 1, sizeof(TFirstCertificationItem), IsoFile);

	dataOffset = toDWORD(FirstCertificationItem.dataOffset);
	//printf("Partition Data offset %010I64X\n", dataOffset << 2);

	dataSize = toDWORD(FirstCertificationItem.dataSize);
	//printf("Partition Data size %010I64X\n", dataSize << 2);

	memset(&vec, 0, 4 * sizeof(DWORD));
	//memset(&nullvec, 0, 4 * sizeof(DWORD));
	memcpy(vec, FirstCertificationItem.firstHalfOfPartitionKeyIV, 2 * sizeof(DWORD));

	AES_set_decrypt_key(masterKey, 128, &aes_key);
	AES_cbc_encrypt(FirstCertificationItem.encryptedPartitionKey, partitionKey, 16, &aes_key, (unsigned char *)vec, AES_DECRYPT);

	AES_set_decrypt_key(partitionKey, 128, &aes_key);

	realDataOffset = partOffset + dataOffset;
	realDataOffset = realDataOffset << 2;

	//read data
	PartitionRead(0, partDataHead, 0x480);

	fstOffset = 4 * toDWORD(*(DWORD*)&partDataHead[0x0424]);
	fstSize = 4 * toDWORD(*(DWORD*)&partDataHead[0x0428]);

	//read  fst
	fst = (TFSTItem*)malloc(fstSize);

	PartitionRead(fstOffset, (unsigned char*)fst, fstSize);

	FillTreeView(fst, fstSize);

	//free(fst);
	fclose(IsoFile);
}

void CWiiViewDlg::FillTreeView(TFSTItem* fst, DWORD fstSize)
{
	DWORD fstItemCount;
	int i;
	//TFSTItem* fst;
	char* fileNameTableBase;

	fstItemCount = toDWORD(fst[0].FileSize);

	fileNameTableBase = (char*)fst + fstItemCount * sizeof(TFSTItem);
	HTREEITEM hRoot = m_tvMain.InsertItem((LPCTSTR)"Root", 0);

	for (i = 1; i < fstItemCount; i++)
	{
		DWORD fileNameOffset = (fst[i].FileName[2]) | ((fst[i].FileName[1] << 8) & 0xff00) | ((fst[i].FileName[0] << 16) & 0xff0000);

		switch(fst[i].ItemType)
		{
		case 0:
			{
				HTREEITEM hItem = m_tvMain.InsertItem((LPCTSTR)fileNameTableBase + fileNameOffset, 1, 1, hRoot);
				m_tvMain.SetItemData(hItem, i);
			}
			break;
		case 1:
			{
				if (0 == toDWORD(fst[i].FileOffset))
				{
					int count = toDWORD(fst[i].FileSize) - i - 1;

					HTREEITEM hNewItem =  m_tvMain.InsertItem((LPCTSTR)fileNameTableBase + fileNameOffset, 0 ,0 ,hRoot);
					m_tvMain.SetItemData(hNewItem, i);
					//Add
					AddTreeViewItem(hNewItem, fst, i + 1, count, fileNameTableBase);

					i = i + count;
					//parentIndex = i;
				} 
			}
			break;
		}
	}
}

void CWiiViewDlg::AddTreeViewItem(HTREEITEM base, TFSTItem* fst, int begin, int count, char* nameTableBase)
{
	int i = 0;
	for (i = begin; i< begin + count; i++)
	{
		DWORD fileNameOffset = (fst[i].FileName[2]) | ((fst[i].FileName[1] << 8) & 0xff00) | ((fst[i].FileName[0] << 16) & 0xff0000);

		if (fst[i].ItemType == 0)
		{
			//DWORD fileNameOffset = (fst[i].FileName[2]) | ((fst[i].FileName[1] << 8) & 0xff00) | ((fst[i].FileName[0] << 16) & 0xff0000);
			HTREEITEM hItem = m_tvMain.InsertItem((LPCTSTR)nameTableBase + fileNameOffset, 1, 1, base);
			m_tvMain.SetItemData(hItem, i);
			//i++;
		}
		else
		{

			//DWORD fileNameOffset = (fst[i].FileName[2]) | ((fst[i].FileName[1] << 8) & 0xff00) | ((fst[i].FileName[0] << 16) & 0xff0000);
			HTREEITEM hNewItem = m_tvMain.InsertItem((LPCTSTR)nameTableBase + fileNameOffset, 0,  0, base);
			m_tvMain.SetItemData(hNewItem, i);

			int j = toDWORD(fst[i].FileSize) - i - 1;

			AddTreeViewItem(hNewItem, fst, i + 1, j, nameTableBase);

			//i = i + (toDWORD(fst[i].FileSize) - toDWORD(fst[i].FileOffset) - 1);
			i = i + j;

		}
	}
}

void CWiiViewDlg::PartitionRead(__int64 offset, unsigned char*data, DWORD len)
{
	unsigned char block[0x8000];
	DWORD offset_in_block;
	DWORD len_in_block;

	while (len) 
	{
		offset_in_block = offset % 0x7c00;
		len_in_block = 0x7c00 - offset_in_block;
		if (len_in_block > len)
			len_in_block = len;
		PartitionReadBlock(offset / 0x7c00, block);
		memcpy(data, block + offset_in_block, len_in_block);
		data += len_in_block;
		offset += len_in_block;
		len -= len_in_block;
	}
}

void CWiiViewDlg::PartitionReadBlock(__int64 blockno, unsigned char* block)
{
	unsigned char raw[0x8000];
	unsigned char iv[16];
	unsigned char h[20];
	unsigned char *h0, *h1, *h2;
	DWORD b1, b2, b3;
	__int64 offset;
	DWORD i;

	offset = realDataOffset + 0x8000 * blockno;
	//partition_raw_read(offset, raw, 0x8000);
	FILE* IsoFile = fopen(csIsoPathName, "rb");
	_fseeki64(IsoFile, offset, SEEK_SET);
	fread(raw, 0x8000, 1, IsoFile);
	fclose(IsoFile);

	// decrypt data
	memcpy(iv, raw + 0x3d0, 16);
	AES_cbc_encrypt(raw + 0x400, block, 0x7c00, &aes_key, iv, AES_DECRYPT);

}
void CWiiViewDlg::OnDestroy()
{
	CDialog::OnDestroy();
	// TODO: Add your message handler code here
	if (fst)
	{
		free(fst);
		fst = NULL;
	}
}

void CWiiViewDlg::OnTvnSelchangedTreeMain(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	HTREEITEM hItem = m_tvMain.GetSelectedItem();

	int index = m_tvMain.GetItemData(hItem);
	if (fst[index].ItemType == 0)
	{
		m_btnExtract.EnableWindow(TRUE);
	} 
	else
	{
		m_btnExtract.EnableWindow(FALSE);
	}

	*pResult = 0;
}

void CWiiViewDlg::OnBnClickedButtonExtract()
{
	// TODO: Add your control notification handler code here
	HTREEITEM hItem = m_tvMain.GetSelectedItem();

	int index = m_tvMain.GetItemData(hItem);

	__int64 offset = 4 * toDWORD(fst[index].FileOffset);
	DWORD size = toDWORD(fst[index].FileSize);
	CString csFileName = m_tvMain.GetItemText(hItem);

	CFileDialog diaSaveFile(FALSE, NULL, csFileName);

	if (diaSaveFile.DoModal() == IDOK)
	{
		unsigned char data[0x80000];
		FILE *fp;
		DWORD block_size;

		fp = fopen(diaSaveFile.GetPathName(), "wb");
		//if (fp == 0)
		//	fatal("cannot open output file");

		while (size)
		{
			block_size = sizeof data;
			if (block_size > size)
			{
				block_size = size;
			}
			PartitionRead(offset, data, block_size);
			
			fwrite(data, block_size, 1, fp);
			//if (fwrite(data, block_size, 1, fp) != 1)
			//	fatal("error writing output file");

			offset += block_size;
			size -= block_size;
		}
		fclose(fp);	
	}

}

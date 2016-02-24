// DlgConfigDisk.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfigdisk.h"
#include "diskdrive.h"
#include "aipcdefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigDisk dialog


CDlgConfigDisk::CDlgConfigDisk(CDiskInterface* pInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfigDisk::IDD, pParent)
{
	m_pInterface = pInterface;
	//{{AFX_DATA_INIT(CDlgConfigDisk)
	m_strDisk1Image = _T("");
	m_strDisk2Image = _T("");
	m_bEnhanced1 = FALSE;
	m_bEnhanced2 = FALSE;
	//}}AFX_DATA_INIT
}


void CDlgConfigDisk::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgConfigDisk)
	DDX_Control(pDX, IDC_CHK_DISK2, m_btnEnableDisk2);
	DDX_Control(pDX, IDC_CHK_DISK1, m_btnEnableDisk1);
	DDX_Control(pDX, IDC_BTN_BROWSE2, m_btnBrowseDisk2);
	DDX_Control(pDX, IDC_BTN_BROWSE1, m_btnBrowseDisk1);
	DDX_Text(pDX, IDC_EDIT_FILENAME1, m_strDisk1Image);
	DDX_Text(pDX, IDC_EDIT_FILENAME2, m_strDisk2Image);
	DDX_Check(pDX, IDC_ENHANCED_DISK1, m_bEnhanced1);
	DDX_Check(pDX, IDC_ENHANCED_DISK2, m_bEnhanced2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgConfigDisk, CDialog)
	//{{AFX_MSG_MAP(CDlgConfigDisk)
	ON_BN_CLICKED(IDC_BTN_BROWSE1, OnBtnBrowse1)
	ON_BN_CLICKED(IDC_BTN_BROWSE2, OnBtnBrowse2)
	ON_BN_CLICKED(IDC_CHK_DISK2, OnChkDisk2)
	ON_BN_CLICKED(IDC_CHK_DISK1, OnChkDisk1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigDisk message handlers

BOOL CDlgConfigDisk::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if ( m_pInterface->GetDrive(0) )
	{
		m_btnEnableDisk1.SetCheck(1);
		m_strDisk1Image = m_pInterface->GetDrive(0)->GetFileName();
	}
	else
	{
		m_btnEnableDisk1.SetCheck(0);
		m_btnEnableDisk1.EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FILENAME1)->EnableWindow(FALSE);
		m_btnBrowseDisk1.EnableWindow(FALSE);
	}
	if ( m_pInterface->GetDrive(1) )
	{
		m_btnEnableDisk2.SetCheck(1);
		m_strDisk2Image = m_pInterface->GetDrive(1)->GetFileName();
	}
	else
	{
		m_btnEnableDisk2.SetCheck(0);
		m_btnEnableDisk2.EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FILENAME2)->EnableWindow(FALSE);
		m_btnBrowseDisk2.EnableWindow(FALSE);
	}
	m_bEnhanced1 = m_pInterface->GetDrive(0)->m_bEnhanced;
	m_bEnhanced2 = m_pInterface->GetDrive(1)->m_bEnhanced;
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgConfigDisk::OnBtnBrowse1() 
{
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DSK Files (*.dsk;*.do;*.po;*.nib)|*.dsk;*.do;*.po;*.nib|All Files (*.*)|*.*||");

	if (dlgFile.DoModal() == IDOK)
	{
		CString oldStr = m_strDisk1Image;
		m_strDisk1Image = dlgFile.GetPathName();
		UpdateData(FALSE);
	}
}

void CDlgConfigDisk::OnBtnBrowse2() 
{
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DSK Files (*.dsk;*.do;*.po;*.nib)|*.dsk;*.do;*.po;*.nib|All Files (*.*)|*.*||");

	if (dlgFile.DoModal() == IDOK)
	{
		CString oldStr = m_strDisk2Image;
		m_strDisk2Image = dlgFile.GetPathName();
		UpdateData(FALSE);
	}
}

void CDlgConfigDisk::OnChkDisk1() 
{
	if (m_btnEnableDisk1.GetCheck() == 0)
	{
		GetDlgItem(IDC_EDIT_FILENAME1)->EnableWindow(FALSE);
		m_btnBrowseDisk1.EnableWindow(FALSE);
//		m_pInterface->GetDrive(0)->Umount();
	}
	else
	{
		GetDlgItem(IDC_EDIT_FILENAME1)->EnableWindow();
		m_btnBrowseDisk1.EnableWindow();
//		m_pInterface->GetDrive(0)->Mount(m_strDisk1Image);
	}
}

void CDlgConfigDisk::OnChkDisk2() 
{
	if (m_btnEnableDisk2.GetCheck() == 0)
	{
		GetDlgItem(IDC_EDIT_FILENAME2)->EnableWindow(FALSE);
		m_btnBrowseDisk2.EnableWindow(FALSE);
//		m_pInterface->GetDrive(1)->Umount();
	}
	else
	{
		GetDlgItem(IDC_EDIT_FILENAME2)->EnableWindow();
		m_btnBrowseDisk2.EnableWindow();
//		m_pInterface->GetDrive(1)->Mount(m_strDisk2Image);
	}
}

void CDlgConfigDisk::OnOK() 
{
	UpdateData(TRUE);
	m_pInterface->GetDrive(0)->m_bEnhanced = m_bEnhanced1;
	m_pInterface->GetDrive(1)->m_bEnhanced = m_bEnhanced2;
	if ( m_btnEnableDisk1.GetCheck() && m_strDisk1Image != "" )
	{
		if ( m_pInterface->GetDrive(0)->Mount( m_strDisk1Image ) != E_SUCCESS )
		{
			CString str;
			str.Format( "Could not open the image.\nDisk1: %s", m_strDisk1Image );
			::AfxGetMainWnd()->MessageBox( str, _T("aipc information") ) ;
			return;
		}
		else if ( m_pInterface->GetDrive(0)->IsWriteProtected() )
		{
			CString str;
			str.Format( "Open write protected disk.\nDisk1: %s", m_strDisk1Image );
			::AfxGetMainWnd()->MessageBox( str, _T("aipc information") ) ;
		}
	}
	else
		m_pInterface->GetDrive(0)->Umount();

	if ( m_btnEnableDisk2.GetCheck() && m_strDisk2Image != "" )
	{
		if ( m_pInterface->GetDrive(1)->Mount( m_strDisk2Image ) != E_SUCCESS )
		{
			CString str;
			str.Format( "Could not open the image.\nDisk2: %s", m_strDisk2Image );
			::AfxGetMainWnd()->MessageBox( str, _T("aipc information") ) ;
			return;
		}
		if ( m_pInterface->GetDrive(1)->IsWriteProtected() )
		{
			CString str;
			str.Format( "Open write protected disk.\nDisk2: %s", m_strDisk2Image );
			::AfxGetMainWnd()->MessageBox( str, _T("aipc information") ) ;
		}
	}
	else
	{
		m_pInterface->GetDrive(1)->Umount();
	}
	CDialog::OnOK();
}

// DlgConfigDisk.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfigsddisk.h"
#include "sddiskii.h"
#include "aipcdefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgConfigSDDisk::CDlgConfigSDDisk(CSDDiskII* pInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfigSDDisk::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgConfigSDDisk)
	m_strDisk1Image = _T("");
	m_strDisk2Image = _T("");
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pInterface = pInterface;
}


void CDlgConfigSDDisk::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgConfigSDDisk)
	DDX_Control(pDX, IDC_CHK_DISK2, m_btnEnableDisk2);
	DDX_Control(pDX, IDC_CHK_DISK1, m_btnEnableDisk1);
	DDX_Control(pDX, IDC_BTN_BROWSE2, m_btnBrowseDisk2);
	DDX_Control(pDX, IDC_BTN_BROWSE1, m_btnBrowseDisk1);
	DDX_Text(pDX, IDC_EDIT_FILENAME1, m_strDisk1Image);
	DDX_Text(pDX, IDC_EDIT_FILENAME2, m_strDisk2Image);
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgConfigSDDisk, CDialog)
	//{{AFX_MSG_MAP(CDlgConfigSDDisk)
	ON_BN_CLICKED(IDC_BTN_BROWSE1, OnBtnBrowse1)
	ON_BN_CLICKED(IDC_BTN_BROWSE2, OnBtnBrowse2)
	ON_BN_CLICKED(IDC_CHK_DISK2, OnChkDisk2)
	ON_BN_CLICKED(IDC_CHK_DISK1, OnChkDisk1)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigSDDisk message handlers

BOOL CDlgConfigSDDisk::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_btnEnableDisk1.SetCheck(1);
	m_strDisk1Image = m_pInterface->GetDrive(0)->GetFileName();

	m_btnEnableDisk2.SetCheck(1);
	m_strDisk2Image = m_pInterface->GetDrive(1)->GetFileName();

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgConfigSDDisk::OnBtnBrowse1() 
{
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DSK Files (*.hdv)|*.hdv|All Files (*.*)|*.*||");

	if (dlgFile.DoModal() == IDOK)
	{
		CString oldStr = m_strDisk1Image;
		m_strDisk1Image = dlgFile.GetPathName();
		UpdateData(FALSE);
	}
}

void CDlgConfigSDDisk::OnBtnBrowse2() 
{
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DSK Files (*.hdv)|*.hdv|All Files (*.*)|*.*||");

	if (dlgFile.DoModal() == IDOK)
	{
		CString oldStr = m_strDisk2Image;
		m_strDisk2Image = dlgFile.GetPathName();
		UpdateData(FALSE);
	}
}

void CDlgConfigSDDisk::OnChkDisk1() 
{
	if (m_btnEnableDisk1.GetCheck() == 0)
	{
		GetDlgItem(IDC_EDIT_FILENAME1)->EnableWindow(FALSE);
		m_btnBrowseDisk1.EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_FILENAME1)->EnableWindow();
		m_btnBrowseDisk1.EnableWindow();
	}
}

void CDlgConfigSDDisk::OnChkDisk2() 
{
	if (m_btnEnableDisk2.GetCheck() == 0)
	{
		GetDlgItem(IDC_EDIT_FILENAME2)->EnableWindow(FALSE);
		m_btnBrowseDisk2.EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_FILENAME2)->EnableWindow();
		m_btnBrowseDisk2.EnableWindow();
	}
}

void CDlgConfigSDDisk::OnOK() 
{
	UpdateData(TRUE);

	if ( m_btnEnableDisk1.GetCheck() && m_strDisk1Image != "" )
	{
		if ( m_pInterface->GetDrive(0)->Mount( m_strDisk1Image ) != E_SUCCESS )
		{
			CString str;
			str.Format( "Could not open the image.\nDisk1: %s", m_strDisk1Image );
			::AfxGetMainWnd()->MessageBox( str, _T("aipc information") ) ;
			return;
		}
		/*
		else if ( m_pInterface->GetDrive(0)->IsWriteProtected() )
		{
			CString str;
			str.Format( "Open write protected disk.\nDisk1: %s", m_strDisk1Image );
			::AfxGetMainWnd()->MessageBox( str, _T("aipc information") ) ;
		}
		*/
	}
	else
	{
		m_pInterface->GetDrive(0)->Umount();
	}

	if ( m_btnEnableDisk2.GetCheck() && m_strDisk2Image != "" )
	{
		if ( m_pInterface->GetDrive(1)->Mount( m_strDisk2Image ) != E_SUCCESS )
		{
			CString str;
			str.Format( "Could not open the image.\nDisk2: %s", m_strDisk2Image );
			::AfxGetMainWnd()->MessageBox( str, _T("aipc information") ) ;
			return;
		}
		/*
		if ( m_pInterface->GetDrive(1)->IsWriteProtected() )
		{
			CString str;
			str.Format( "Open write protected disk.\nDisk2: %s", m_strDisk2Image );
			::AfxGetMainWnd()->MessageBox( str, _T("aipc information") ) ;
		}
		*/
	}
	else
	{
		m_pInterface->GetDrive(1)->Umount();
	}
	CDialog::OnOK();
}

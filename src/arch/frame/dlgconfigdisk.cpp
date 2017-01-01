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
	m_strDisk1Image = _T("");
	m_strDisk2Image = _T("");
	m_bEnhanced1 = FALSE;
	m_bEnhanced2 = FALSE;
}


void CDlgConfigDisk::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_DISK2, m_btnEnableDisk2);
	DDX_Control(pDX, IDC_CHK_DISK1, m_btnEnableDisk1);
	DDX_Control(pDX, IDC_BTN_BROWSE2, m_btnBrowseDisk2);
	DDX_Control(pDX, IDC_BTN_BROWSE1, m_btnBrowseDisk1);
	DDX_Check(pDX, IDC_ENHANCED_DISK1, m_bEnhanced1);
	DDX_Check(pDX, IDC_ENHANCED_DISK2, m_bEnhanced2);
	DDX_Control(pDX, IDC_EDIT_FILENAME1, m_cDisk1Path);
	DDX_Control(pDX, IDC_EDIT_FILENAME2, m_cDisk2Path);
}


BEGIN_MESSAGE_MAP(CDlgConfigDisk, CDialog)
	ON_BN_CLICKED(IDC_BTN_BROWSE1, OnBtnBrowse1)
	ON_BN_CLICKED(IDC_BTN_BROWSE2, OnBtnBrowse2)
	ON_BN_CLICKED(IDC_CHK_DISK2, OnChkDisk2)
	ON_BN_CLICKED(IDC_CHK_DISK1, OnChkDisk1)
	ON_EN_SETFOCUS(IDC_EDIT_FILENAME1, &CDlgConfigDisk::OnSetfocusEditFilename1)
	ON_EN_KILLFOCUS(IDC_EDIT_FILENAME1, &CDlgConfigDisk::OnKillfocusEditFilename1)
	ON_EN_SETFOCUS(IDC_EDIT_FILENAME2, &CDlgConfigDisk::OnSetfocusEditFilename2)
	ON_EN_KILLFOCUS(IDC_EDIT_FILENAME2, &CDlgConfigDisk::OnKillfocusEditFilename2)
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

	SetFileName(&m_cDisk1Path, m_strDisk1Image);
	SetFileName(&m_cDisk2Path, m_strDisk2Image);

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgConfigDisk::OnBtnBrowse1() 
{
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DSK Files (*.dsk;*.do;*.po;*.nib;*.2mg;*.2img)|*.dsk;*.do;*.po;*.nib;*.2mg;*.2img|All Files (*.*)|*.*||");

	if (dlgFile.DoModal() == IDOK)
	{
		m_strDisk1Image = dlgFile.GetPathName();
		SetFileName(&m_cDisk1Path, m_strDisk1Image);
	}
}

void CDlgConfigDisk::OnBtnBrowse2() 
{
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DSK Files (*.dsk;*.do;*.po;*.nib;*.2mg;*.2img)|*.dsk;*.do;*.po;*.nib;*.2mg;*.2img|All Files (*.*)|*.*||");

	if (dlgFile.DoModal() == IDOK)
	{
		m_strDisk2Image = dlgFile.GetPathName();
		SetFileName(&m_cDisk2Path, m_strDisk2Image);
	}
}

void CDlgConfigDisk::OnChkDisk1() 
{
	if (m_btnEnableDisk1.GetCheck() == 0)
	{
		m_cDisk1Path.EnableWindow(FALSE);
		m_btnBrowseDisk1.EnableWindow(FALSE);
//		m_pInterface->GetDrive(0)->Umount();
	}
	else
	{
		m_cDisk1Path.EnableWindow();
		m_btnBrowseDisk1.EnableWindow();
//		m_pInterface->GetDrive(0)->Mount(m_strDisk1Image);
	}
}

void CDlgConfigDisk::OnChkDisk2() 
{
	if (m_btnEnableDisk2.GetCheck() == 0)
	{
		m_cDisk2Path.EnableWindow(FALSE);
		m_btnBrowseDisk2.EnableWindow(FALSE);
//		m_pInterface->GetDrive(1)->Umount();
	}
	else
	{
		m_cDisk2Path.EnableWindow();
		m_btnBrowseDisk2.EnableWindow();
//		m_pInterface->GetDrive(1)->Mount(m_strDisk2Image);
	}
}

void CDlgConfigDisk::OnOK() 
{
	UpdateData(TRUE);
	m_pInterface->GetDrive(0)->m_bEnhanced = m_bEnhanced1;
	m_pInterface->GetDrive(1)->m_bEnhanced = m_bEnhanced2;
	m_btnEnableDisk1.SetFocus();

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

void CDlgConfigDisk::OnSetfocusEditFilename1()
{
	m_cDisk1Path.SetWindowText(m_strDisk1Image);
}


void CDlgConfigDisk::OnKillfocusEditFilename1()
{
	m_cDisk1Path.GetWindowText(m_strDisk1Image);
	SetFileName(&m_cDisk1Path, m_strDisk1Image);
}

void CDlgConfigDisk::OnSetfocusEditFilename2()
{
	m_cDisk2Path.SetWindowText(m_strDisk2Image);
}


void CDlgConfigDisk::OnKillfocusEditFilename2()
{
	m_cDisk2Path.GetWindowText(m_strDisk2Image);
	SetFileName(&m_cDisk2Path, m_strDisk2Image);
}

void CDlgConfigDisk::SetFileName(CEdit *edit, CString path)
{
	edit->SetWindowText(path.Mid(path.ReverseFind('\\') + 1));
}

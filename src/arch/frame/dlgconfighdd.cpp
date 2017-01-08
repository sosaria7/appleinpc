// DlgConfigDisk.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfighdd.h"
#include "hdd.h"
#include "aipcdefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgConfigHDD::CDlgConfigHDD(CHDDInterface* pInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfigHDD::IDD, pParent)
{
	m_strDisk1Image = _T("");
	m_pInterface = pInterface;
}


void CDlgConfigHDD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_DISK1, m_btnEnableDisk1);
	DDX_Control(pDX, IDC_BTN_BROWSE1, m_btnBrowseDisk1);
	DDX_Control(pDX, IDC_EDIT_FILENAME1, m_cDisk1Path);
}


BEGIN_MESSAGE_MAP(CDlgConfigHDD, CDialog)
	ON_BN_CLICKED(IDC_BTN_BROWSE1, OnBtnBrowse1)
	ON_BN_CLICKED(IDC_CHK_DISK1, OnChkDisk1)
	ON_EN_SETFOCUS(IDC_EDIT_FILENAME1, &CDlgConfigHDD::OnSetfocusEditFilename)
	ON_EN_KILLFOCUS(IDC_EDIT_FILENAME1, &CDlgConfigHDD::OnKillfocusEditFilename)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigHDD message handlers

BOOL CDlgConfigHDD::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_btnEnableDisk1.SetCheck(1);
	m_strDisk1Image = m_pInterface->GetDrive(0)->GetFileName();
	SetFileName(&m_cDisk1Path, m_strDisk1Image);

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgConfigHDD::OnBtnBrowse1() 
{
	const TCHAR* pFileName = NULL;
	TCHAR buffer[4096] = TEXT("");
	TCHAR** lppPart = { NULL };

	if (m_strDisk1Image.IsEmpty())
	{
		pFileName = NULL;
	}
	else
	{
		if (GetFullPathName(m_strDisk1Image, 4096, buffer, lppPart) != 0)
			pFileName = buffer;
		else
			pFileName = (const TCHAR*)m_strDisk1Image;
	}

	CFileDialog dlgFile(TRUE, TEXT("hdv"), pFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DSK Files (*.hdv;*.2mg;*.2img)|*.hdv;*.2mg;*.2img|All Files (*.*)|*.*||");

	if (dlgFile.DoModal() == IDOK)
	{
		m_strDisk1Image = dlgFile.GetPathName();
		SetFileName(&m_cDisk1Path, m_strDisk1Image);
	}
}

void CDlgConfigHDD::OnChkDisk1() 
{
	if (m_btnEnableDisk1.GetCheck() == 0)
	{
		m_cDisk1Path.EnableWindow(FALSE);
		m_btnBrowseDisk1.EnableWindow(FALSE);
	}
	else
	{
		m_cDisk1Path.EnableWindow();
		m_btnBrowseDisk1.EnableWindow();
	}
}

void CDlgConfigHDD::OnOK() 
{
	UpdateData(TRUE);
	m_btnEnableDisk1.SetFocus();

	if ( m_btnEnableDisk1.GetCheck() && m_strDisk1Image != "" )
	{
		if ( m_pInterface->GetDrive(0)->Mount( m_strDisk1Image ) != E_SUCCESS )
		{
			CString str;
			str.Format( "Could not open the image.\nDisk: %s", m_strDisk1Image );
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
	{
		m_pInterface->GetDrive(0)->Umount();
	}

	CDialog::OnOK();
}

void CDlgConfigHDD::OnSetfocusEditFilename()
{
	// TODO: Add your control notification handler code here
	m_cDisk1Path.SetWindowText(m_strDisk1Image);
}


void CDlgConfigHDD::OnKillfocusEditFilename()
{
	// TODO: Add your control notification handler code here
	m_cDisk1Path.GetWindowText(m_strDisk1Image);
	SetFileName(&m_cDisk1Path, m_strDisk1Image);
}

void CDlgConfigHDD::SetFileName(CEdit *edit, CString path)
{
	edit->SetWindowText(path.Mid(path.ReverseFind('\\') + 1));
}

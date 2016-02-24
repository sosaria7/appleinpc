// DlgDebug.cpp : implementation file
//


#include "arch/frame/stdafx.h"
#include "arch/frame/dlgdebug.h"
#include "optable.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDebug dialog


CDlgDebug::CDlgDebug(C65c02* pCPU, CAppleIOU* pMemory, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDebug::IDD, pParent)
{
	m_pCPU = pCPU;
	m_pMemory = pMemory;
	//{{AFX_DATA_INIT(CDlgDebug)
	//}}AFX_DATA_INIT
}


void CDlgDebug::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDebug)
	DDX_Control(pDX, IDC_FLAG_Z, m_flagZ);
	DDX_Control(pDX, IDC_EIDT_DATA, m_editData);
	DDX_Control(pDX, IDC_LIST_REGISTERS, m_listRegisters);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_listProcess);
	DDX_Control(pDX, IDC_FLAG_V, m_flagV);
	DDX_Control(pDX, IDC_FLAG_N, m_flagN);
	DDX_Control(pDX, IDC_FLAG_I, m_flagI);
	DDX_Control(pDX, IDC_FLAG_EXTEND, m_flagX);
	DDX_Control(pDX, IDC_FLAG_D, m_flagD);
	DDX_Control(pDX, IDC_FLAG_C, m_flagC);
	DDX_Control(pDX, IDC_FLAG_B, m_flagB);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDebug, CDialog)
	//{{AFX_MSG_MAP(CDlgDebug)
	ON_BN_CLICKED(IDC_BUTTON_TRACE, OnButtonTrace)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgDebug message handlers

void CDlgDebug::OnButtonTrace() 
{
	m_pCPU->Process();
	UpdateData();
}

int CDlgDebug::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	for(int i=0; i < 10; i++)
		m_aiAddrs[i] = -1;
	return 0;
}

HBRUSH CDlgDebug::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	BYTE flag = m_pCPU->m_regF;
	BYTE bit=0;
	if(pWnd==&m_flagN){
		bit = 0x80;
	}
	else if(pWnd==&m_flagV){
		bit = 0x40;
	}
	else if(pWnd==&m_flagX){
		bit = 0x20;
	}
	else if(pWnd==&m_flagB){
		bit = 0x10;
	}
	else if(pWnd==&m_flagD){
		bit = 0x08;
	}
	else if(pWnd==&m_flagI){
		bit = 0x04;
	}
	else if(pWnd==&m_flagZ){
		bit = 0x02;
	}
	else if(pWnd==&m_flagC){
		bit = 0x01;
	}
	if(flag&bit)
		pDC->SetTextColor(RGB(255, 0, 0));
	else
		pDC->SetTextColor(RGB(0, 0, 0));
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

BOOL CDlgDebug::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CRect rect;
	m_listProcess.GetClientRect(&rect);
	LV_COLUMN lvcolumn;
	lvcolumn.mask = LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH|LVIS_SELECTED;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.iSubItem = 0;
	lvcolumn.cx =130;
	lvcolumn.pszText = "code";
	m_listProcess.InsertColumn(0, &lvcolumn);
	lvcolumn.iSubItem = 1;
	lvcolumn.cx = rect.Width()-130;
	lvcolumn.pszText = "operator";
	m_listProcess.InsertColumn(1, &lvcolumn);
	m_listProcess.InsertItem(0, "$0300: AD 03 0F");
	m_listProcess.SetItemText(0, 1, "LDA $0300,X");

	m_listRegisters.GetClientRect(&rect);
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.iSubItem = 0;
	lvcolumn.cx =70;
	lvcolumn.pszText = "name";
	m_listRegisters.InsertColumn(0, &lvcolumn);
	lvcolumn.iSubItem = 1;
	lvcolumn.cx = rect.Width()-70;
	lvcolumn.pszText = "value";
	m_listRegisters.InsertColumn(1, &lvcolumn);
	m_listRegisters.InsertItem(0, "PC");
	m_listRegisters.InsertItem(1, "A");
	m_listRegisters.InsertItem(2, "X");
	m_listRegisters.InsertItem(3, "Y");
	m_listRegisters.InsertItem(4, "S");
	m_listRegisters.InsertItem(5, "P");

	UpdateData();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgDebug::addProcess(int num, WORD address)
{
	// TODO: Add your control notification handler code here

	CString codes;
	CString str;
	BYTE op = m_pMemory->ReadMem8(address&0xFFFF);
	BYTE d1 = m_pMemory->ReadMem8((address+1)&0xFFFF);
	BYTE d2 = m_pMemory->ReadMem8((address+2)&0xFFFF);

	OP_TABLE* optable = &operators_65c02[op];
	unsigned int type = optable->type;
	switch(type){
	case M_NONE:
		str.Format("%s", optable->OPERATOR);
		codes.Format("%02X", op);

		break;
	case M_IMM:
		str.Format("%s #$%02X", optable->OPERATOR, d1);
		codes.Format("%02X %02X", op, d1);
		break;

	case M_REL:
		WORD offset;
		if(d1&0x80)
			offset = d1 - 0x100;
		else
			offset = d1;
		str.Format("%s $%04X", optable->OPERATOR, (address+offset+2)&0xFFFF);
		codes.Format("%02X %02X", op, d1);
		break;

	case M_ZP:
		str.Format("%s $%02X", optable->OPERATOR, d1);
		codes.Format("%02X %02X", op, d1);
		break;

	case M_ZPX:
		str.Format("%s $%02X,X", optable->OPERATOR, d1);
		codes.Format("%02X %02X", op, d1);
		break;

	case M_ZPY:
		str.Format("%s $%02X,Y", optable->OPERATOR, d1);
		codes.Format("%02X %02X", op, d1);
		break;

	case M_ABS:
		str.Format("%s $%02X%02X", optable->OPERATOR, d2, d1);
		codes.Format("%02X %02X %02X", op, d1, d2);
		break;

	case M_ABSX:
		str.Format("%s $%02X%02X,X", optable->OPERATOR, d2, d1);
		codes.Format("%02X %02X %02X", op, d1, d2);
		break;

	case M_ABSY:
		str.Format("%s $%02X%02X,Y", optable->OPERATOR, d2, d1);
		codes.Format("%02X %02X %02X", op, d1, d2);
		break;

	case M_ZIND:
		str.Format("%s ($%02X)", optable->OPERATOR, d1);
		codes.Format("%02X %02X", op, d1);
		break;

	case M_IND:
		str.Format("%s ($%02X%02X)", optable->OPERATOR, d2, d1);
		codes.Format("%02X %02X %02X", op, d1, d2);
		break;

	case M_INDX:
		str.Format("%s ($%02X,X)", optable->OPERATOR, d1);
		codes.Format("%02X %02X", op, d1);
		break;

	case M_INDY:
		str.Format("%s ($%02X),Y", optable->OPERATOR, d1);
		codes.Format("%02X %02X", op, d1);
		break;

	case M_ABINDX:
		str.Format("%s ($%02X%02X,X)", optable->OPERATOR, d2, d1);
		codes.Format("%02X %02X %02X", op, d1, d2);
		break;

	case M_BB:
		if(d2&0x80)
			offset = d2 - 0x100;
		else
			offset = d2;
		str.Format("%s $%02X $%04X", optable->OPERATOR, d1, (address+offset+3)&0xFFFF );
		codes.Format("%02X %02X %02X", op, d1, d2);
		break;

	default:
		str.Format("%s", optable->OPERATOR );
		codes.Format("%02X", op );
		break;
	}

//	str.Format("OP:%02X %02X %02X", mem[cpu->m_regPC], mem[(cpu->m_regPC+1)%0xFFFF], mem[(cpu->m_regPC+2)%0xFFFF]);
	CString code;
	code.Format("$%04X: %s", address, codes);
	m_listProcess.InsertItem(num, code);
	m_listProcess.SetItemText(num, 1, str);
}

void CDlgDebug::UpdateData()
{
	WORD address = m_pCPU->getRegPC();
	int i;
	for(i=0; i < 10; i++){
		if(address==m_aiAddrs[i]){
			m_listProcess.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
//			m_listProcess.SetSelectionMark(i);
			break;
		}
	}

	// could not find the address in process list;
	if(i==10){
		m_listProcess.DeleteAllItems();
		for(i=0; i < 10; i++){
			addProcess(i, address);
			m_aiAddrs[i] = address;
			address+=operators_65c02[m_pMemory->ReadMem8(address)].len;
		}
		m_listProcess.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
//		m_listProcess.SetSelectionMark(0);
	}

	// redraw Flags
	m_flagN.RedrawWindow();
	m_flagV.RedrawWindow();
	m_flagX.RedrawWindow();
	m_flagB.RedrawWindow();
	m_flagD.RedrawWindow();
	m_flagI.RedrawWindow();
	m_flagZ.RedrawWindow();
	m_flagC.RedrawWindow();
	
	CString value;
	value.Format("$%04X", m_pCPU->getRegPC());
	m_listRegisters.SetItemText(0, 1, value);
	value.Format("$%02X", m_pCPU->m_regA);
	m_listRegisters.SetItemText(1, 1, value);
	value.Format("$%02X", m_pCPU->m_regX);
	m_listRegisters.SetItemText(2, 1, value);
	value.Format("$%02X", m_pCPU->m_regY);
	m_listRegisters.SetItemText(3, 1, value);
	value.Format("$%02X", m_pCPU->m_regS);
	m_listRegisters.SetItemText(4, 1, value);
	value.Format("$%02X", m_pCPU->m_regF);
	m_listRegisters.SetItemText(5, 1, value);
}

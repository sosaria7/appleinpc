// F:\night_project\appleinpc\src\arch\frame\dlgsettingsslot.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "afxdialogex.h"
#include "aipcdefs.h"
#include "dlgsettingsslot.h"
#include "card.h"
#include "appleclock.h"
#include "mousecard.h"
#include "phasor.h"
#include "diskinterface.h"
#include "hdd.h"

// CDlgSettingsSlot dialog

IMPLEMENT_DYNAMIC(CDlgSettingsSlot, CDialogEx)

CDlgSettingsSlot::CDlgSettingsSlot(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS_SLOT, pParent)
{
	int i;
	for (i = 0; i < 7; i++)
	{
		m_pCards[i] = NULL;
	}
}

CDlgSettingsSlot::~CDlgSettingsSlot()
{
	int i;
	for (i = 0; i < 7; i++)
	{
		// delete the cards that newly created.
		if (m_pCards[i] != NULL && m_pCards[i] != g_pBoard->m_cSlots.GetCard(i))
		{
			delete m_pCards[i];
			m_pCards[i] = NULL;
		}
	}
}

void CDlgSettingsSlot::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLOT1, m_acSlots[0]);
	DDX_Control(pDX, IDC_SLOT2, m_acSlots[1]);
	DDX_Control(pDX, IDC_SLOT3, m_acSlots[2]);
	DDX_Control(pDX, IDC_SLOT4, m_acSlots[3]);
	DDX_Control(pDX, IDC_SLOT5, m_acSlots[4]);
	DDX_Control(pDX, IDC_SLOT6, m_acSlots[5]);
	DDX_Control(pDX, IDC_SLOT7, m_acSlots[6]);
	DDX_Control(pDX, IDC_SLOT1_SETUP, m_acSetupCard[0]);
	DDX_Control(pDX, IDC_SLOT2_SETUP, m_acSetupCard[1]);
	DDX_Control(pDX, IDC_SLOT3_SETUP, m_acSetupCard[2]);
	DDX_Control(pDX, IDC_SLOT4_SETUP, m_acSetupCard[3]);
	DDX_Control(pDX, IDC_SLOT5_SETUP, m_acSetupCard[4]);
	DDX_Control(pDX, IDC_SLOT6_SETUP, m_acSetupCard[5]);
	DDX_Control(pDX, IDC_SLOT7_SETUP, m_acSetupCard[6]);
}


BEGIN_MESSAGE_MAP(CDlgSettingsSlot, CDialogEx)
	ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_SLOT1, IDC_SLOT7, OnSelchangeSlot)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_SLOT1_SETUP, IDC_SLOT7_SETUP, OnClickedSlotSetup)
END_MESSAGE_MAP()


// CDlgSettingsSlot message handlers

int CDlgSettingsSlot::DeviceNameToDeviceID(CString strDeviceName)
{
	//Empty;Mouse Interface;Apple Disk II;Mocking B./Phasor;Videx VideoTerm;
	if (strDeviceName.Compare("Empty") == 0)
		return CARD_EMPTY;

	else if (strDeviceName.Compare("Mouse Interface") == 0)
		return CARD_MOUSE_INTERFACE;

	else if (strDeviceName.Compare("Apple DISK ][") == 0)
		return CARD_DISK_INTERFACE;

	else if (strDeviceName.Compare("Mocking B./Phasor") == 0)
		return CARD_PHASOR;

	else if (strDeviceName.Compare("HDD") == 0)
		return CARD_HDD;

	else if (strDeviceName.Compare("Videx VideoTerm") == 0)
		return CARD_VIDEX_VIDEOTERM;

	return CARD_EMPTY;
}

void CDlgSettingsSlot::OnSelchangeSlot(UINT uId)
{
	int nCurSel;
	CCard* pCard;
	CString strSelDeviceName;
	if (uId < IDC_SLOT1 || uId > IDC_SLOT7)
		return;			// it must not occur

	uId -= IDC_SLOT1;
	nCurSel = m_acSlots[uId].GetCurSel();
	m_acSlots[uId].GetLBText(nCurSel, strSelDeviceName);

	if (m_pCards[uId] == NULL || m_pCards[uId]->GetDeviceName().Compare(strSelDeviceName) != 0)
	{
		pCard = g_pBoard->m_cSlots.GetCard(uId);
		if (m_pCards[uId] != NULL)
		{
			// 새로 설치된 카드인 경우 삭제
			// rollback(cancel)할 경우를 대비해서 기존의 카드는 삭제하지 않는다.
			if (m_pCards[uId] != pCard)
				delete m_pCards[uId];
			m_pCards[uId] = NULL;
		}
		// 기존에 있던 카드와 동일하면 기존의 카드를 사용한다.
		if (pCard != NULL && pCard->GetDeviceName().Compare(strSelDeviceName) == 0)
			m_pCards[uId] = pCard;
		else
		{
			int nDeviceID = DeviceNameToDeviceID(strSelDeviceName);
			switch (nDeviceID)
			{
			case CARD_EMPTY:		// empty
				m_acSetupCard[uId].EnableWindow(FALSE);
				return;
			case CARD_MOUSE_INTERFACE:		// mouse interface card
				m_pCards[uId] = new CMouseCard();
				break;
			case CARD_DISK_INTERFACE:		// apple disk ii interface card
				m_pCards[uId] = new CDiskInterface();
				break;
			case CARD_PHASOR:				// phasor/mocking board sound card
				m_pCards[uId] = new CPhasor();
				break;
			case CARD_HDD:					// HDD interface card
				m_pCards[uId] = new CHDDInterface();
				break;
			case CARD_VIDEX_VIDEOTERM:
				//m_pCards[uId] = new CVidexTerm();
				break;
			default:
				return;	// it must not occur
			}
		}
	}
	m_acSetupCard[uId].EnableWindow(TRUE);
}


void CDlgSettingsSlot::OnClickedSlotSetup(UINT uId)
{
	// TODO: Add your control notification handler code here
	if (uId < IDC_SLOT1_SETUP || uId > IDC_SLOT7_SETUP)
		return;			// it must not occur
	uId -= IDC_SLOT1_SETUP;

	// if m_pCards[uId] is NULL, the slot is empty and setup button should be disabled.
	if (m_pCards[uId] == NULL)
	{
		m_acSetupCard[uId].EnableWindow(FALSE);
		return;
	}
	m_pCards[uId]->Configure();
}


BOOL CDlgSettingsSlot::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	BOOL bIsPowerOn;
	int i;
	int j;

	// TODO:  Add extra initialization here
	bIsPowerOn = g_pBoard->GetIsActive();
	for (i = 0; i < 7; i++)
	{
		m_pCards[i] = g_pBoard->m_cSlots.GetCard(i);
		if (m_pCards[i] != NULL)
		{
			CString strDeviceName = m_pCards[i]->GetDeviceName();
			CString strSlotText;
			for (j = 0; j < m_acSlots[i].GetCount(); j++)
			{
				m_acSlots[i].GetLBText(j, strSlotText);
				if (strSlotText.Compare(strDeviceName) == 0)
				{
					m_acSlots[i].SetCurSel(j);
					m_acSetupCard[i].EnableWindow(TRUE);
					break;
				}
			}
		}
		else
		{
			m_acSlots[i].SetCurSel(0);
			m_acSetupCard[i].EnableWindow(FALSE);
		}
		m_acSlots[i].EnableWindow(!bIsPowerOn);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgSettingsSlot::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	int i;
	UpdateData(TRUE);

	for (i = 0; i < 7; i++)
	{
		if (m_pCards[i] != g_pBoard->m_cSlots.GetCard(i))
		{
			g_pBoard->m_cSlots.RemoveCard(i);
			g_pBoard->m_cSlots.InsertCard(i, m_pCards[i]);
		}
	}
	g_pBoard->m_cSlots.Initialize();

	CDialogEx::OnOK();
}


void CDlgSettingsSlot::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	int i;
	for (i = 0; i < 7; i++)
	{
		// delete the cards that newly created.
		if (m_pCards[i] != NULL && m_pCards[i] != g_pBoard->m_cSlots.GetCard(i))
		{
			delete m_pCards[i];
			m_pCards[i] = NULL;		// not necessary but...
		}
	}

	CDialogEx::OnCancel();
}


BOOL CDlgSettingsSlot::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;                // Do not process further
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

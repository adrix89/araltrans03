  // PageMain.cpp : implementation file
//

#pragma warning(disable:4996)
#pragma warning(disable:4101)

#include "stdafx.h"
#include "ATCode.h"
#include "PageMain.h"
#include "OptionMgr.h"
#include "OptionDlg.h"

// CPageMain dialog

IMPLEMENT_DYNAMIC(CPageMain, CDialog)

LPCTSTR _FONT_LOAD_DESC[] = {
	_T("�ѱ� ��Ʈ�� �ε����� �ʽ��ϴ�."),
	_T("���� ��� �Լ��� �ѱ���Ʈ�� �����մϴ�."),
	_T("�ѱ���Ʈ ���� �� �������� �ʽ��ϴ�."),
	_T("��Ʈ �ε� �� �ѱ� ��Ʈ�� �ε����ݴϴ�."),
	_T("���α׷��� ��� ��Ʈ�� �ѱ۷� �ٲߴϴ�.")
};


CPageMain::CPageMain(CWnd* pParent /*=NULL*/)
	: CDialog(CPageMain::IDD, pParent), m_pRootNode(NULL)
{

}

CPageMain::~CPageMain()
{
}

void CPageMain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CPageMain::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();
	return bRetVal;
}

BEGIN_MESSAGE_MAP(CPageMain, CDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_HOOK, &CPageMain::OnBnClickedBtnAddHook)
END_MESSAGE_MAP()


// CPageMain message handlers
BOOL CPageMain::InitFromRootNode( COptionNode* pRootNode )
{
	BOOL bRetVal = FALSE;

	try
	{
		if(NULL==pRootNode) throw -1;

		// ��Ʈ�ѵ� �⺻ ���·� ����
		ClearCtrlValues();

		// ��� ��� ��ȸ
		int cnt = pRootNode->GetChildCount();
		for(int i=0; i<cnt; i++)
		{
			COptionNode* pNode = pRootNode->GetChild(i);
			CString strValue = pNode->GetValue().MakeUpper();
		}

		bRetVal = TRUE;

		m_pRootNode = pRootNode;


	}
	catch (int nErr)
	{
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// ��� UI ��Ʈ�� �ʱ�ȭ
//
//////////////////////////////////////////////////////////////////////////
void CPageMain::ClearCtrlValues()
{
}



//////////////////////////////////////////////////////////////////////////
//
// ���ο� ��ŷ�ڵ� �߰�
//
//////////////////////////////////////////////////////////////////////////
void CPageMain::SetChildNodeFromCheckbox(COptionNode* pParentNode, LPCTSTR cszChildName, CButton& checkbox)
{
	if(NULL==pParentNode || NULL==cszChildName || _T('\0')==cszChildName[0]) return;

	COptionNode* pNode = pParentNode->GetChild(cszChildName);

	// üũ�� ���
	if(checkbox.GetCheck())
	{
		if(NULL==pNode)
		{
			pNode = pParentNode->CreateChild();
			pNode->SetValue(cszChildName);
		}
	}
	// üũ ���� �� ���
	else
	{
		if(pNode)
		{
			pParentNode->DeleteChild(pNode);
		}
	}

	
	if( COptionDlg::_Inst && ::IsWindow(COptionDlg::_Inst->m_btnApply.m_hWnd))
	{
		COptionDlg::_Inst->m_btnApply.EnableWindow(TRUE);
	}
}


void CPageMain::OnBnClickedBtnAddHook()
{
	// TODO: Add your control notification handler code here
	COptionDlg::_Inst->OnBnClickedBtnAddHook();
}

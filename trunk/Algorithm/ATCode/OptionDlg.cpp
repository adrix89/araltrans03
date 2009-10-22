// OptionDlg.cpp : implementation file
//
#pragma warning(disable:4996)

#include "stdafx.h"
#include "ATCode.h"
#include "ATCodeMgr.h"
#include "OptionDlg.h"
#include "PageMain.h"
#include "PageHook.h"
#include "OptionMgr.h"
#include "NewHookDlg.h"
#include "OptionInputDlg.h"

// COptionDlg dialog

IMPLEMENT_DYNAMIC(COptionDlg, CDialog)

COptionDlg* COptionDlg::_Inst = NULL;


void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tabMain);
	DDX_Control(pDX, IDAPPLY, m_btnApply);
}


BEGIN_MESSAGE_MAP(COptionDlg, CDialog)
	ON_MESSAGE(WM_DELETE_HOOK, &COptionDlg::OnDeleteHook)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &COptionDlg::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDOK, &COptionDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDAPPLY, &COptionDlg::OnBnClickedApply)
	ON_BN_CLICKED(IDC_BTN_ADD_HOOK, &COptionDlg::OnBnClickedBtnAddHook)
	ON_BN_CLICKED(IDC_BUTTON1, &COptionDlg::OnBnClickedBtnInputString)
END_MESSAGE_MAP()



COptionDlg::COptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionDlg::IDD, pParent), m_pRootNode(NULL)
{
	_Inst = this;
}

COptionDlg::~COptionDlg()
{
	ClearControls();
	_Inst = NULL;
}

BOOL COptionDlg::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();

	if(InitFromRootNode(m_pRootNode) == FALSE)
	{
		this->PostMessage(WM_CLOSE,0,0);
	}

	return bRetVal;
}


BOOL COptionDlg::InitFromRootNode( COptionNode* pRootNode )
{
	ClearControls();

	CATCodeMgr::GetInstance()->MigrateOption(pRootNode);

	// �⺻���� ������ �ʱ�ȭ
	int nRes = InitMainPage(pRootNode);
	if(nRes < 0)
	{
		MessageBox(_T("���� ���� ȭ���� ������ �� �����ϴ�."));
		return FALSE;
	}

	// ��ŷ���� ������ �ʱ�ȭ
	int cnt = pRootNode->GetChildCount();
	for(int i=0; i<cnt; i++)
	{
		COptionNode* pNode = pRootNode->GetChild(i);
		CString strValue = pNode->GetValue().MakeUpper();

		// HOOK ���
		if(strValue == _T("HOOK"))
		{
			nRes = AddHookPage(pNode);
			if(nRes < 0)
			{
				MessageBox(_T("���� ��ŷ�ڵ忡 ���� ���� ȭ���� ������ �� �����ϴ�.\r\n\n") + pNode->ToString());
				return FALSE;
			}
		}
	}

	// ���� ���̴� �������� �⺻ ��������
	ShowPage(0);

	return TRUE;
}


void COptionDlg::ClearControls()
{
	int cnt = (int)m_arrPage.GetCount();

	for(int i=0; i<cnt; i++)
	{
		if(0==i)
		{
			delete ((CPageMain*)m_arrPage[i]);
		}
		else
		{
			delete ((CPageHook*)m_arrPage[i]);
		}
	}

	m_arrPage.RemoveAll();
	
	if(::IsWindow(m_tabMain.m_hWnd)) m_tabMain.DeleteAllItems();

}


// COptionDlg message handlers

void COptionDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nItem = m_tabMain.GetCurSel();
	ShowPage(nItem);

	*pResult = 0;
}


void COptionDlg::ShowPage( int nPageIdx )
{
	m_tabMain.SetCurSel(nPageIdx);
	
	int cnt = m_tabMain.GetItemCount();
	for( int i=0; i<cnt; i++)
	{
		CDialog* pDlg = m_arrPage[i];
		if( i != nPageIdx )
		{
			pDlg->ShowWindow( SW_HIDE );
		}
		else
		{
			pDlg->ShowWindow( SW_SHOW );
		}
	}

}

void COptionDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void COptionDlg::OnBnClickedBtnCreateShortcut()
{
	// TODO: Add your control notification handler code here
}

//////////////////////////////////////////////////////////////////////////
//
// ���� �ɼ� ��� ����
//
//////////////////////////////////////////////////////////////////////////
void COptionDlg::SetRootOptionNode( COptionNode* pRootNode )
{
	m_pRootNode = pRootNode;
}



//////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////
int COptionDlg::InitMainPage(COptionNode* pRootNode)
{
	int nRetVal = -1;

	CPageMain* pMainpage = new CPageMain(&m_tabMain);
	pMainpage->Create( IDD_PAGE_MAIN, &m_tabMain );
	pMainpage->ShowWindow(SW_SHOW);
	//pMainpage->MoveWindow( 5, 25, 310, 290, FALSE);
	CRect rcTab;
	m_tabMain.GetWindowRect(rcTab);
	pMainpage->MoveWindow( 3, 25, rcTab.Width()-10, rcTab.Height()-40, FALSE);
	
	if(pMainpage->InitFromRootNode(pRootNode))
	{
		m_tabMain.InsertItem( 0, _T("����") );
		m_arrPage.Add( (CDialog*)pMainpage );
		nRetVal = 0;
	}
	else
	{
		pMainpage->DestroyWindow();
		delete pMainpage;
		pMainpage = NULL;
	}
	

	return nRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////
int COptionDlg::AddHookPage(COptionNode* pHookNode)
{
	int nRetVal = -1;

	CPageHook* pHookPage = new CPageHook(&m_tabMain);
	pHookPage->Create( IDD_PAGE_HOOK, &m_tabMain );
	pHookPage->ShowWindow( SW_HIDE );
	//pHookPage->MoveWindow( 5, 25, 310, 290, FALSE);
	CRect rcTab;
	m_tabMain.GetWindowRect(rcTab);
	pHookPage->MoveWindow( 3, 25, rcTab.Width()-10, rcTab.Height()-40, FALSE);
	
	if(pHookPage->InitFromHookNode(pHookNode))
	{
		nRetVal = m_tabMain.GetItemCount();
		m_tabMain.InsertItem( nRetVal, pHookNode->GetChild(0)->GetValue() );
		m_arrPage.Add( (CDialog*)pHookPage );
	}
	else
	{
		pHookPage->DestroyWindow();
		delete pHookPage;
		pHookPage = NULL;
	}

	return nRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////
void COptionDlg::OnBnClickedApply()
{
	BOOL bRes = CATCodeMgr::GetInstance()->ApplyOption(m_pRootNode);
	if(bRes) m_btnApply.EnableWindow(FALSE);
}



//////////////////////////////////////////////////////////////////////////
//
// ��ŷ�ڵ� �߰�
//
//////////////////////////////////////////////////////////////////////////
void COptionDlg::OnBnClickedBtnAddHook()
{
	// ���� ��ŷ ��ġ ����
	CNewHookDlg newHookDlg;

	if(newHookDlg.DoModal() == IDOK)
	{
		
		try
		{
			HMODULE hModule = newHookDlg.m_hModule;
			CString strModuleName = newHookDlg.m_strModuleName;

			CString strFormatedAddr = FormatAddress(newHookDlg.m_strHookAddr);
			if(strFormatedAddr.IsEmpty() || NULL == m_pRootNode)
			{
				this->MessageBox(_T("�߸��� �ּ� �����Դϴ�."), _T("����"));
				throw -1;
			}

			if(hModule)
			{
				strFormatedAddr = strModuleName + _T("!") + strFormatedAddr;
			}

			
			// HOOK ��� ����
			COptionNode* pNode = m_pRootNode->CreateChild();
			if(NULL == pNode) throw -2;
			pNode->SetValue(_T("HOOK"));

			// �ּ� ����
			COptionNode* pAddrNode = pNode->CreateChild();
			if(NULL == pAddrNode) throw -3;
			pAddrNode->SetValue(strFormatedAddr);

			// UI ������ ����
			int nIdx = AddHookPage(pNode);
			if(nIdx < 0) throw -4;

			ShowPage(nIdx);

		}
		catch (int nErrCode)
		{
			nErrCode = nErrCode;
		}
	}
	
}



//////////////////////////////////////////////////////////////////////////
//
// �־��� �ּҸ� ����ȭ�Ͽ� ��ȯ
//
//////////////////////////////////////////////////////////////////////////
CString COptionDlg::FormatAddress( LPCTSTR cszAddr )
{
	CString strRetVal = _T("");

	try
	{
		if(NULL == cszAddr) throw -1;

		CString strSrc = cszAddr;
		strSrc = strSrc.MakeUpper();

		// ��ȿ�� �˻�
		int cnt = strSrc.GetLength();
		for(int i=0; i<cnt; i++)
		{
			if( (strSrc[i] < _T('A') || strSrc[i] > _T('F')) 
				&& (strSrc[i] < _T('0') || strSrc[i] > _T('9')) 
				&& strSrc[i] != _T('X') ) throw -2;
		}

		// ����ȭ
		UINT_PTR pAddr;
		_stscanf(strSrc, _T("%x"), &pAddr);
		if(NULL == pAddr) throw -3;

		strRetVal.Format(_T("0x%p"), pAddr);


	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
	}

	return strRetVal;

}


LRESULT COptionDlg::OnDeleteHook( WPARAM wParam, LPARAM lParam )
{
	try
	{
		if(0 == wParam) throw -1;

		// �ǿ��� ����
		int nIdx = m_tabMain.GetCurSel();
		if(nIdx < 0 || nIdx >= m_tabMain.GetItemCount()) throw -2;
		m_tabMain.DeleteItem(nIdx);

		// ������ ����
		if(nIdx >= m_arrPage.GetCount()) throw -3;
		delete ((CPageHook*)m_arrPage[nIdx]);
		m_arrPage.RemoveAt(nIdx);


		// Ʈ������ ����
		m_pRootNode->DeleteChild((COptionNode*)wParam);


		// ���� ���õ� �� �ε����� �� ������ ���̱�
		ShowPage(nIdx-1);

	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
	}

	return 0;
}


void COptionDlg::OnBnClickedBtnInputString()
{
	// ���� �ɼ��� ��� ����޾� ���´�.
	CString strPrevOptions = m_pRootNode->ChildrenToString();

	// �����Է� ���̾�α� ����
	COptionInputDlg input_dlg(this);
	input_dlg.m_strInputString = strPrevOptions;
	
	if(input_dlg.DoModal() == IDOK)
	{
		try
		{
			//COptionNode tmpRootNode;
			
			BOOL bRes = m_pRootNode->ParseChildren(input_dlg.m_strInputString);
			if(FALSE == bRes) throw _T("�ɼ� ���ڿ� �ؼ��� ������ �ֽ��ϴ�.");

			bRes = InitFromRootNode(m_pRootNode);
			if(FALSE == bRes) throw _T("������ �ùٸ��� UI���� �� ������ �߻��߽��ϴ�.");

			//m_pRootNode->ParseChildren(input_dlg.m_strInputString);
			m_btnApply.EnableWindow(TRUE);
		}
		catch (LPCTSTR strErrMsg)
		{
			this->MessageBox(strErrMsg, _T("Invalid option string"));			
			// ���������Ƿ� ���� �ɼ����� ����
			m_pRootNode->ParseChildren(strPrevOptions);
			InitFromRootNode(m_pRootNode);
		}

	}

}

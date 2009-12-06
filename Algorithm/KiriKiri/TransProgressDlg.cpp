// TransProgressDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Kirikiri.h"
#include "TransProgressDlg.h"
#include <process.h>

extern CKirikiriApp theApp;

// CTransProgressDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CTransProgressDlg, CDialog)

CTransProgressDlg::CTransProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransProgressDlg::IDD, pParent), m_bCancel(FALSE), m_pThread(NULL), m_bShowUI(FALSE)
{
	
}

CTransProgressDlg::~CTransProgressDlg()
{

}



void CTransProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progTrans);
	DDX_Control(pDX, IDC_ANI_GIF, m_picAniGif);
}


BEGIN_MESSAGE_MAP(CTransProgressDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CTransProgressDlg::OnBnClickedCancel)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CTransProgressDlg::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	CenterWindow();

	// ���� ������ ������ ����
	m_pThread = ::AfxBeginThread(MainThread, this);
	
	// ���α׷������� ������ �� ���� ����
	m_progTrans.SendMessage(PBM_SETBKCOLOR, (WPARAM)0, (LPARAM)(COLORREF)RGB(230,230,230) );
	
	m_progTrans.SendMessage(PBM_SETBARCOLOR, (WPARAM)0, (LPARAM)(COLORREF)RGB(130,190,255) );

	// �ִϸ��̼� GIF
	if (m_picAniGif.Load(MAKEINTRESOURCE(IDR_GIF_1), _T("GIF")))
		m_picAniGif.Draw();


	this->SetTimer(927, 1500, NULL);

	return bRetVal;  // return TRUE  unless you set the focus to a control	
}


UINT CTransProgressDlg::MainThread(void *lParam)
{
	UINT nRetVal = 0;

	CTransProgressDlg* pThisDlg = (CTransProgressDlg*)lParam;

	try
	{
		CKAGScriptMgr* pScriptMgr = theApp.GetKAGScriptMgr();
		LPCWSTR orig_script = theApp.GetOriginalScript();
		LPWSTR buf = theApp.GetScriptBuffer();
		if(NULL == pScriptMgr) throw -1;
		if(NULL == orig_script) throw -2;
		if(NULL == buf) throw -3;

		if(orig_script[0] == L'\0')
		{
			buf[0] = L'\0';
		}
		else
		{

			// �ݹ� �Լ� ���
			pScriptMgr->SetProgressCallback(pThisDlg, ProgressCallback);

			// ����
			BOOL bTrans = FALSE;
			for(int i=0; i<5; i++)
			{
				bTrans = pScriptMgr->TranslateScript(orig_script, buf);
				if(bTrans) break;
				Sleep(500);
			}

			if(FALSE == bTrans) throw -4;
		}

		pThisDlg->PostMessage(WM_COMMAND, IDOK);

	}
	catch (int nErrCode)
	{
		TRACE(_T("[aral1] TranslationThread catched exception %d"), nErrCode);
		nRetVal = nErrCode;
		pThisDlg->PostMessage(WM_COMMAND, IDCANCEL);
	}

	return nRetVal;
}

int CALLBACK CTransProgressDlg::ProgressCallback( void* pContext, int nTransed, int nTotal )
{
	CTransProgressDlg* pThisDlg = (CTransProgressDlg*)pContext;
	int nRetVal = 1;
	
	// ��ҵǾ��ٸ�
	if(pThisDlg->m_bCancel)
	{
		nRetVal = 0;
	}
	// �������̶��
	else
	{
		// ù ȣ���̸� ���α׷��� ���� ����
		if(0 == nTransed)
		{
			pThisDlg->m_progTrans.SetRange(0, nTotal);

			CWnd* pWnd = pThisDlg->GetDlgItem(IDC_STATIC_HASH);
			if(pWnd)
			{
				CString strHash;
				strHash.Format(_T("%p"), theApp.GetKAGScriptMgr()->GetCurrentScriptHash());
				pWnd->SetWindowText(strHash);
			}

		}


		CWnd* pWnd = pThisDlg->GetDlgItem(IDC_STATIC_SIZE);
		if(pWnd)
		{
			CString strSize;
			strSize.Format(_T("%d/%d Lines"), nTransed, nTotal);
			pWnd->SetWindowText(strSize);
		}

		pThisDlg->m_progTrans.SetPos(nTransed);

	}

	return nRetVal;
}

// CTransProgressDlg �޽��� ó�����Դϴ�.

void CTransProgressDlg::OnBnClickedCancel()
{
	if( MessageBox(_T("������ �ߴ��Ͻðڽ��ϱ�?"), _T("Confirm"), MB_YESNO) == IDYES )
	{
		m_bCancel = TRUE;
		TRACE(_T("[aral1] m_bCancel = TRUE;"));
	}
}

//////////////////////////////////////////////////////////////////////////
//
// ������ �Ⱥ��̰� ����
//
//////////////////////////////////////////////////////////////////////////
void CTransProgressDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{

	if(m_bShowUI)
	{
		lpwndpos->flags |= SWP_SHOWWINDOW;
	}
	else
	{
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}

	CDialog::OnWindowPosChanging(lpwndpos);
}

void CTransProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if(nIDEvent == 927)
	{
		this->KillTimer(nIDEvent);
		m_bShowUI = TRUE;
		this->ShowWindow(SW_NORMAL);
	}

	CDialog::OnTimer(nIDEvent);
}

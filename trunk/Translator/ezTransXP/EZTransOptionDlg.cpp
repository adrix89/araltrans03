// EZTransOptionDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "ezTransXP.h"
#include "EZTransOptionDlg.h"


// CEZTransOptionDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CEZTransOptionDlg, CDialog)

CEZTransOptionDlg::CEZTransOptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEZTransOptionDlg::IDD, pParent)
	, m_bRemoveTrace(FALSE)
	, m_bRemoveDupSpace(FALSE)
{

}

CEZTransOptionDlg::~CEZTransOptionDlg()
{
}

void CEZTransOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHK_ALLOW_TRACE, m_bRemoveTrace);
	DDX_Check(pDX, IDC_CHK_ALLOW_DUPSPACE, m_bRemoveDupSpace);
}


BEGIN_MESSAGE_MAP(CEZTransOptionDlg, CDialog)
END_MESSAGE_MAP()


// CEZTransOptionDlg �޽��� ó�����Դϴ�.

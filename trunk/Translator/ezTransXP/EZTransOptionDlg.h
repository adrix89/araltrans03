#pragma once
#include "afxwin.h"


// CEZTransOptionDlg ��ȭ �����Դϴ�.

class CEZTransOptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CEZTransOptionDlg)

public:
	CEZTransOptionDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CEZTransOptionDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_OPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bRemoveTrace;
	BOOL m_bRemoveDupSpace;
};

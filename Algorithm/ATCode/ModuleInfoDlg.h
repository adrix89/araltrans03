#pragma once
#include "afxwin.h"


// CModuleInfoDlg ��ȭ �����Դϴ�.

class CModuleInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CModuleInfoDlg)

public:
	CString m_strHookAddr;
	CModuleInfoDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CModuleInfoDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_MOD_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editModName;
	CEdit m_editModRange;
};

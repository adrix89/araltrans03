#pragma once
#include "afxwin.h"


// CKirikiriOptionDlg ��ȭ �����Դϴ�.

class CKirikiriOptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CKirikiriOptionDlg)

public:
	CKirikiriOptionDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CKirikiriOptionDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DLG_OPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	int m_nCacheMode;
	BOOL m_bAlsoSrc;
	afx_msg void OnBnClickedBtnClearCache();
	afx_msg void OnBnClickedCheckUseCodepoint2();
	BOOL m_bUseCP2;
	virtual BOOL OnInitDialog();
	int m_nCP2Type;
	CComboBox m_comboCP2Type;
	afx_msg void OnCbnSelchangeComboCodepointtype();
	afx_msg void OnBnClickedOk();
};

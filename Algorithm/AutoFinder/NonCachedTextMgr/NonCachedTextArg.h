#pragma once

#define MAX_TEXT_LENGTH 1024

class CNonCachedTextArg
{
	friend class CNonCachedTextArgMgr;

private:
	int		m_nNextTestIdx;						// ���ڿ��� ���� ��ġ
	int		m_nNextKorIdx;						// ���� ��ȯ�� ����
	int		m_nHitCnt;							// ��Ʈ ī��Ʈ
	int		m_nDupCnt;							// �ߺ� ī��Ʈ
	int		m_nFaultCnt;						// ���� ī��Ʈ
	wchar_t	m_wszJapaneseText[MAX_TEXT_LENGTH];	// �Ϻ��� �ؽ�Ʈ ����
	size_t	m_nJapaneseLen;						// �Ϻ��� �ؽ�Ʈ ����
	wchar_t	m_wszKoreanText[MAX_TEXT_LENGTH];	// �ѱ��� �ؽ�Ʈ ����
	size_t	m_nKoreanLen;						// �ѱ��� �ؽ�Ʈ ����

	inline BOOL IsControlCharacter(wchar_t wch) const;
		
public:
	CNonCachedTextArg(void);
	~CNonCachedTextArg(void);

	BOOL	SetNonCachedTextArg(LPCWSTR wszJapaneseText);
	void	ResetNonCachedTextArg();

	int		TestCharacter(wchar_t wch, UINT nPreperedDupCnt);
	int		GetHitCount();
	wchar_t	GetTranslatedCharacter();

	int		TestString(LPCWSTR wszJapaneseText, int nLen);
	int		GetTranslatedString(LPWSTR wszKoreanBuf, int nBufSize, int &nOrigLen);
};

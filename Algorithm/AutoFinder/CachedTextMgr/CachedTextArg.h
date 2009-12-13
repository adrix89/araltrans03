#pragma once
#include <set>

#define MAX_TEXT_LENGTH 1024

using namespace std;

class ITranslator;

class CCachedText
{
	friend class CCachedTextArgMgr;

private:
	BOOL		m_bWideChar;
	BOOL		m_bTranslated;
	BOOL		m_bTranslatedTestMode;
	int			m_nNextTestIdx;						// ���ڿ��� ���� ��ġ
	int			m_nHitCnt;							// ��Ʈ ī��Ʈ
	int			m_nFaultCnt;						// ���� ī��Ʈ
	set<void*>	m_setSourcePtr;
	set<pair<UINT_PTR,size_t>> m_setFuncArg;		// ��� ȣ��� �Լ� �����Ϳ� ESP�κ����� �Ÿ�

	char		m_szJapaneseText[MAX_TEXT_LENGTH];	// �Ϻ��� �ؽ�Ʈ ���� (��Ƽ����Ʈ)
	wchar_t		m_wszJapaneseText[MAX_TEXT_LENGTH];	// �Ϻ��� �ؽ�Ʈ ���� (�����ڵ�)
	size_t		m_nJapaneseLen;						// �Ϻ��� �ؽ�Ʈ ����

	char		m_szKoreanText[MAX_TEXT_LENGTH];	// �ѱ��� �ؽ�Ʈ ���� (��Ƽ����Ʈ)
	wchar_t		m_wszKoreanText[MAX_TEXT_LENGTH];	// �ѱ��� �ؽ�Ʈ ���� (�����ڵ�)
	size_t		m_nKoreanLen;						// �ѱ��� �ؽ�Ʈ ����

	inline BOOL IsControlCharacter(wchar_t ch) const;
	BOOL		OverwriteTranslatedText(LPVOID pDest);
	BOOL		IsJapaneseTextA(LPCSTR szJpnText);
	BOOL		IsJapaneseTextW(LPCWSTR wszJpnText);

public:
	CCachedText(void);
	~CCachedText(void);

	BOOL	SetCachedText(LPVOID pSource, BOOL bWideChar);
	void	ResetCachedText();

	BOOL	IsWideCharacter();
	BOOL	IsTranslatable();
	void	Translate();
	
	//////////////////////////////////////////////////////////////////////////
	//
	// <��ȯ��>
	// 0 : ���� ������ ����
	// ��� : �ߺ��� ���ڿ�. ������ ó���� �Ͽ���.
	int		TestText(LPVOID pSource);

	//////////////////////////////////////////////////////////////////////////
	//
	// <��ȯ��>
	// 0 : ���ߵ��� ����
	// 1 : ����
	//////////////////////////////////////////////////////////////////////////
	int		TestCharacter(wchar_t wch);
	
	//////////////////////////////////////////////////////////////////////////
	//
	// <��ȯ��>
	// ��� : ���� ���ߵ� ī��Ʈ
	// ���� : ���� ������ ī��Ʈ
	//////////////////////////////////////////////////////////////////////////
	int		GetHitCount();

	//////////////////////////////////////////////////////////////////////////
	//
	// <��ȯ��>
	// �����Ȳ���� ���� ������ ���� ��ȯ
	//////////////////////////////////////////////////////////////////////////
	wchar_t GetBestTranslatedCharacter();
};

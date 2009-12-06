
#include "stdafx.h"
#include "ATTextArg.h"
#include "ATTextArgMgr.h"

#pragma warning(disable:4996)


CATText::CATText(void)
{
	ResetATText();
}

CATText::~CATText(void)
{
}

void CATText::ResetATText()
{
	m_bWideChar = FALSE;
	m_bTranslated = FALSE;
	m_bTranslatedTestMode = FALSE;
	m_nNextTestIdx = 0;						// ���ڿ��� ���� ��ġ
	m_nHitCnt = 0;							// ��Ʈ ī��Ʈ
	m_nFaultCnt = 0;						// ���� ī��Ʈ
	m_setSourcePtr.clear();
	m_setFuncArg.clear();

	m_nJapaneseLen = 0;						// �Ϻ��� �ؽ�Ʈ ����
	m_nKoreanLen = 0;						// �ѱ��� �ؽ�Ʈ ����

	ZeroMemory(m_szJapaneseText, MAX_TEXT_LENGTH);					// �Ϻ��� �ؽ�Ʈ ���� (��Ƽ����Ʈ)
	ZeroMemory(m_wszJapaneseText, MAX_TEXT_LENGTH*sizeof(wchar_t));	// �Ϻ��� �ؽ�Ʈ ���� (�����ڵ�)
	ZeroMemory(m_szKoreanText, MAX_TEXT_LENGTH);					// �ѱ��� �ؽ�Ʈ ���� (��Ƽ����Ʈ)
	ZeroMemory(m_wszKoreanText, MAX_TEXT_LENGTH*sizeof(wchar_t));	// �ѱ��� �ؽ�Ʈ ���� (�����ڵ�)
}


//////////////////////////////////////////////////////////////////////////
//
// ��ȿ�� �Ϻ��� �ؽ�Ʈ�ΰ� �� (��Ƽ����Ʈ��)
//
//////////////////////////////////////////////////////////////////////////
BOOL CATText::IsJapaneseTextA(LPCSTR szJpnText)
{
	BOOL bRetVal = FALSE;

	if( IsBadStringPtrA(szJpnText, 1024*1024*1024) == FALSE )
	{
		// ���� ��ȿ�� �˻�
		int nLen = (int)strlen(szJpnText);
		if(1 < nLen && nLen < MAX_TEXT_LENGTH)
		{
			// �����ڵ� ��ȿ�� �˻�
			int score = 0;
			for(int i=0; i<nLen; i++)
			{
				if((BYTE)szJpnText[i]>=0x80)
				{
					if(szJpnText[i+1]==0)
					{
						break;
					}
					else if( 0x81 <= (BYTE)szJpnText[i] && (BYTE)szJpnText[i] <= 0x9F )
					{
						score+=2;
					}

					i++;
				}
			}

			// ���̻� �Ϻ��� �ڵ�� �Ϻ���� ����
			if( score > nLen/2 ) bRetVal = TRUE;
		}
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
//
// ��ȿ�� �Ϻ��� �ؽ�Ʈ�ΰ� �� (�����ڵ��)
//
//////////////////////////////////////////////////////////////////////////
BOOL CATText::IsJapaneseTextW(LPCWSTR wszJpnText)
{
	BOOL bRetVal = FALSE;

	if( IsBadStringPtrW(wszJpnText, 1024*1024*1024) == FALSE )
	{
		size_t nLen = wcslen(wszJpnText);
		if(1 < nLen && nLen < MAX_TEXT_LENGTH)
		{
			bRetVal = TRUE;
		}
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// �ؽ�Ʈ ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CATText::SetATText(LPVOID pSource, BOOL bWideChar)
{
	BOOL bRetVal = FALSE;

	ResetATText();

	if( pSource )
	{	
		BOOL bIsJpnText = FALSE;
		int  nSrcLen = 0;
		
		// ��ȿ�� �˻� : �����ڵ� ���ڿ��� �Ѿ�� ���
		if( bWideChar )
		{
			bIsJpnText = IsJapaneseTextW((LPCWSTR)pSource);

			if( bIsJpnText )
			{
				m_nJapaneseLen = wcslen((LPCWSTR)pSource);
				wcscpy(m_wszJapaneseText, (LPCWSTR)pSource);
				nSrcLen = MyWideCharToMultiByte(932, 0, m_wszJapaneseText, -1, m_szJapaneseText, MAX_TEXT_LENGTH*2-1, NULL, NULL);
			}
		}
		// ��ȿ�� �˻� : ��Ƽ����Ʈ ���ڿ��� �Ѿ�� ���
		else
		{
			bIsJpnText = IsJapaneseTextA((LPCSTR)pSource);
			
			if(bIsJpnText)
			{
				m_nJapaneseLen = strlen((LPCSTR)pSource);
				nSrcLen = m_nJapaneseLen + 1;
				strcpy(m_szJapaneseText, (LPCSTR)pSource);
				MyMultiByteToWideChar(932, 0, m_szJapaneseText, -1, m_wszJapaneseText, MAX_TEXT_LENGTH-1 );
			}
		}


		// �Ϻ��� �ؽ�Ʈ�� ���� ó��
		if( bIsJpnText )
		{			
			// �����⿡ ����־��
			BOOL bTrans = CATTextArgMgr::GetInstance()->m_sATCTNR3.procTranslateUsingCtx(L"DebuggingATCode", m_szJapaneseText, nSrcLen, m_szKoreanText, MAX_TEXT_LENGTH*2);
			if( bTrans )
			{
				MyMultiByteToWideChar(949, 0, m_szKoreanText, -1, m_wszKoreanText, MAX_TEXT_LENGTH-1 );
												
				if(bWideChar) m_nKoreanLen = wcslen(m_wszKoreanText);
				else m_nKoreanLen = strlen(m_szKoreanText);
			}

			m_setSourcePtr.insert(pSource);
			m_bWideChar = bWideChar;
			
			bRetVal = TRUE;

		}
	}

	if( FALSE == bRetVal) ResetATText();

	return bRetVal;
}


int CATText::TestText(LPVOID pSource)
{
	if(NULL==pSource) return 0;

	int iRetVal = 0;
	
	if(m_bWideChar)
	{
		LPWSTR wszSource = (LPWSTR)pSource;
		
		// �Ϻ��� �ؽ�Ʈ�� ��ġ�Ѵٸ�
		if( wcscmp(wszSource, m_wszJapaneseText) == 0 )
		{
			iRetVal = 1;
		}
	}
	else
	{
		LPSTR szSource = (LPSTR)pSource;

		// �Ϻ��� �ؽ�Ʈ�� ��ġ�Ѵٸ�
		if( strcmp(szSource, m_szJapaneseText) == 0 )
		{
			iRetVal = 1;
		}
	}

	if(iRetVal)
	{
		m_setSourcePtr.insert(pSource);
	}

	return iRetVal;
}


int CATText::TestCharacter(wchar_t wch)
{
	int nRetVal = 0;

	if( L'\0' != wch )
	{
		
		size_t tmpIdx = 0;
		
		// �Ϻ��� �˻�
		if( m_bTranslatedTestMode == FALSE )
		{
			if( 0 < m_nNextTestIdx && m_wszJapaneseText[m_nNextTestIdx-1] == wch )
			{
				nRetVal = 3;
			}
			else
			{
				tmpIdx = m_nNextTestIdx;
				while(m_wszJapaneseText[tmpIdx])
				{
					if( m_wszJapaneseText[tmpIdx] == wch )
					{
						m_nNextTestIdx = (int)tmpIdx+1;
						m_nHitCnt++;
						m_nFaultCnt = 0;
						nRetVal = 1;
						break;
					}
					tmpIdx++;
				}
			}
		}

		// �ѱ��� �˻�
		if( 0==nRetVal)
		{
			if( 0 < m_nNextTestIdx && m_wszKoreanText[m_nNextTestIdx-1] == wch )
			{
				nRetVal = 3;
			}
			else
			{
				tmpIdx = m_nNextTestIdx;
				while(m_wszKoreanText[tmpIdx])
				{
					if( m_wszKoreanText[tmpIdx] == wch )
					{
						m_nNextTestIdx = (int)tmpIdx+1;
						m_nHitCnt++;
						m_nFaultCnt = 0;
						m_bTranslatedTestMode = TRUE;
						nRetVal = 1;
						break;
					}
					tmpIdx++;
				}

			}

		}


		//if(0==nRetVal && m_nFaultCnt<50)
		//{
		//	nRetVal = 4;		// ����
		//	m_nHitCnt = 0;
		//	m_nFaultCnt++;
		//}

		if(0==nRetVal)
		{
			m_nHitCnt = 0;
			m_nFaultCnt++;
		}

		// ������
		//if(0==nRetVal && m_nNextTestIdx>2)
		//{
		//	wchar_t dbg[1024];
		//	swprintf(dbg, L"[ aral1 ] Ż�� : %d ��°���� '%c'ã��õ�, \"%s\"->\"%s\"  \n", m_nNextTestIdx, wch, m_wszJapaneseText, m_wszKoreanText);
		//	OutputDebugStringW(dbg);

		//}


	}

	return nRetVal;
}

int CATText::GetHitCount()
{
	return m_nHitCnt;
}

BOOL CATText::IsWideCharacter() 
{
	return m_bWideChar;
}

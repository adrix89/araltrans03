
#include "../stdafx.h"
#include "../AutoFinder.h"
#include "NonCachedTextArg.h"
#include "NonCachedTextArgMgr.h"
#include "Misc.h"

#pragma warning(disable:4996)

CNonCachedTextArg::CNonCachedTextArg(void)
{
	ResetNonCachedTextArg();
}

CNonCachedTextArg::~CNonCachedTextArg(void)
{
}


BOOL CNonCachedTextArg::SetNonCachedTextArg(LPCWSTR wszJapaneseText)
{
	BOOL bRetVal = FALSE;

	ResetNonCachedTextArg();

	if( wszJapaneseText && MAINAPP && MAINAPP->m_sContainerFunc.procTranslateUsingCtx )
	{
		size_t nSrcLen = wcslen(wszJapaneseText);

		if( nSrcLen < MAX_TEXT_LENGTH )
		{
			// �Ϻ��� �ؽ�Ʈ ����			
			//wcscpy(m_wszJapaneseText, wszJapaneseText);
			//m_nJapaneseLen = nSrcLen;
			try
			{

				m_nJapaneseLen = 0;
				for(size_t i=0; i<nSrcLen; i++)
				{
					if( !IsControlCharacter(wszJapaneseText[i]) )
					{
						m_wszJapaneseText[m_nJapaneseLen] = wszJapaneseText[i];
						m_nJapaneseLen++;
					}
				}
				m_wszJapaneseText[m_nJapaneseLen] = L'\0';

				if (!IsJapaneseW(m_wszJapaneseText, m_nJapaneseLen))
					throw -1;

				// �����⿡ ����־��
				char szJapanese[MAX_TEXT_LENGTH*2];

				int nSrcLen = MyWideCharToMultiByte(932, 0, m_wszJapaneseText, -1, szJapanese, MAX_TEXT_LENGTH*2, NULL, NULL);
				if (nSrcLen == 0)
					throw -2;

				char szKorean[MAX_TEXT_LENGTH*2];
				BOOL bTrans = MAINAPP->m_sContainerFunc.procTranslateUsingCtx(
					L"AutoFinder",
					szJapanese,
					nSrcLen,
					szKorean, 
					MAX_TEXT_LENGTH*2
					);
				
				if (!bTrans)
					throw -3;


				ZeroMemory(m_wszKoreanText, MAX_TEXT_LENGTH);
				if (!MyMultiByteToWideChar(949, 0, szKorean, -1, m_wszKoreanText, MAX_TEXT_LENGTH-1 ))
					throw -4;

				CString strKorean = m_wszKoreanText;
				strKorean.Replace(L" ", L"");

				wcscpy_s(m_wszKoreanText, MAX_TEXT_LENGTH, (LPCWSTR)strKorean);
				m_nKoreanLen = wcslen(m_wszKoreanText);

				while(m_nKoreanLen<m_nJapaneseLen)
				{
					m_wszKoreanText[m_nKoreanLen] = L' ';
					m_nKoreanLen++;
				}

				m_nNextTestIdx = 0;
				m_nNextKorIdx = 0;
				m_nHitCnt = 1;
				m_nFaultCnt = 0;

				bRetVal = TRUE;

			}
			catch(int e)
			{
				e = e;
				bRetVal = FALSE;
			}

			//wchar_t dbg[1024];
			//swprintf(dbg, L"[ aral1 ] '%s'->'%s' \n", m_wszJapaneseText, m_wszKoreanText);
			//OutputDebugStringW(dbg);

		}
	}

	if( FALSE == bRetVal) ResetNonCachedTextArg();

	return bRetVal;
}

void CNonCachedTextArg::ResetNonCachedTextArg()
{
	m_nNextTestIdx = 0;								// ���ڿ��� ���� ��ġ
	m_nNextKorIdx = 0;
	m_nHitCnt = 0;
	m_nDupCnt = 0;
	m_nFaultCnt = 0;
	m_nJapaneseLen = 0;								// �Ϻ��� �ؽ�Ʈ ����
	m_nKoreanLen = 0;								// �ѱ��� �ؽ�Ʈ ����
	ZeroMemory(m_wszJapaneseText, MAX_TEXT_LENGTH);	// �Ϻ��� �ؽ�Ʈ ����
	ZeroMemory(m_wszKoreanText, MAX_TEXT_LENGTH);	// �ѱ��� �ؽ�Ʈ ����
}

inline BOOL CNonCachedTextArg::IsControlCharacter(wchar_t wch) const
{
	return ( ( wch==0x0A || wch==0x0D ) ? TRUE : FALSE );
}

int CNonCachedTextArg::TestCharacter(wchar_t wch, UINT nPreperedDupCnt)
{
	int nRetVal = 0;

	if (m_nNextTestIdx == 0)
		m_nNextTestIdx = 1;

	if( L'\0' != wch )
	{
		// �ߺ� ����� ���ɼ��� �����ϸ�
		if( nPreperedDupCnt && (UINT)m_nDupCnt<nPreperedDupCnt && m_wszJapaneseText[m_nNextTestIdx-1] == wch )
		{
			m_nDupCnt++;
			nRetVal = 3;			
		}
		// �Ϲ� ���߽�
		else if( m_wszJapaneseText[m_nNextTestIdx] == wch )
		{
			m_nNextKorIdx++;
			m_nNextTestIdx++;
			m_nHitCnt++;
			m_nDupCnt = 0;
			m_nFaultCnt = 0;
			nRetVal = 1;
		}
		// �ߺ������ ��쵵 �����Ƿ�
		else if( m_wszJapaneseText[m_nNextTestIdx-1] == wch )
		{
			//m_nHitCnt = 0;
			//m_nFaultCnt++;
			m_nDupCnt++;
			nRetVal = 2;
		}
		// �ǳʶپ� ����� �� �����Ƿ�
		else if( m_wszJapaneseText[m_nNextTestIdx] != L'\0' )
		{
			size_t tmpIdx = m_nNextTestIdx;
			while(m_wszJapaneseText[tmpIdx])
			{
				if( m_wszJapaneseText[tmpIdx] == wch )
				{
					m_nNextKorIdx++;
					m_nNextTestIdx = (int)tmpIdx+1;
					m_nHitCnt++;
					m_nDupCnt = 0;
					m_nFaultCnt = 0;
					nRetVal = 1;
					break;
				}
				tmpIdx++;
			}

			if(0==nRetVal && m_nFaultCnt<50)
			{
				nRetVal = 4;		// ����
				m_nHitCnt = 0;
				m_nFaultCnt++;
			}
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

int CNonCachedTextArg::GetHitCount()
{
	return m_nHitCnt;
}

wchar_t CNonCachedTextArg::GetTranslatedCharacter()
{
	wchar_t wchRetVal = L'\0';

	int idx = m_nNextKorIdx;

	if( 0 <= idx && (unsigned)idx < m_nKoreanLen)
	{
		wchRetVal = m_wszKoreanText[idx];

		// ������
		// wchar_t tmp[2];
		// tmp[0] = m_wszKoreanText[idx];
		// tmp[1] = L'\0';
		// wchar_t dbg[MAX_PATH];
		// swprintf(dbg, L"[ aral1 ] Best : '(%x)%s' , '%s'[%d] \n", 
		// 	tmp[0], tmp, m_wszKoreanText, m_nNextKorIdx);
		// OutputDebugStringW(dbg);
	}
	else if( 0 <= idx && (unsigned)idx < m_nJapaneseLen)
	{
		wchRetVal = m_wszJapaneseText[idx];
	}

	//wchar_t dbg[MAX_PATH];
	//swprintf(dbg, L"[ aral1 ] Best : '%c' , '%s'[%d] -> '%s'[%d] \n", 
	//	wchRetVal, m_wszJapaneseText, m_nNextTestIdx-1, m_wszKoreanText, m_nNextKorIdx);
	//OutputDebugStringW(dbg);

	return wchRetVal;
}

int	CNonCachedTextArg::TestString(LPCWSTR wszJapaneseText, int nLen)
{
	int nRetVal = 0;

	char szTemp[1024]= {0, };

	if (wszJapaneseText)
	{
		if (nLen < 0) nLen = lstrlenW(wszJapaneseText);

		// nLen �� 1�� ��� �ѹ��� ��¹���� ���� �ְ�
		// ���� Ư���� ���� ���� �׽�Ʈ������ ù���ڸ� ������ ��찡 �־�
		// Ư��ó���� �� ��� �Ѵ�..
		if (nLen == 1)
		{
			if (wszJapaneseText[0] == m_wszJapaneseText[0])
			{
				// ù���ڰ� ������ �� �����Ƿ� �׳� Hit ��Ű���� m_nTestIdx �� �׳� ���д�.
				m_nHitCnt++;
				m_nDupCnt++;

				return 3;
			}
		}


		// 1. ������ ��µ� �� �������� �׽�Ʈ
		if (!wcsncmp(m_wszJapaneseText + m_nNextTestIdx, wszJapaneseText, nLen))
		{
			// �ι�° ������ ��ġ(Hit!)
			// ������ ��° ������ ���� ù ���
			m_nNextKorIdx = m_nNextTestIdx;
			m_nNextTestIdx += nLen;
			m_nHitCnt++;
			m_nDupCnt = 0;
			m_nFaultCnt = 0;
			nRetVal = 1;

		}
		// 2. ������ ��µ� �� �׽�Ʈ
		else if (!wcsncmp(m_wszJapaneseText + m_nNextKorIdx, wszJapaneseText, nLen))
		{
			// �ι�° ������ �ߺ�
			m_nDupCnt++;
			m_nNextTestIdx = m_nNextKorIdx+nLen;
			nRetVal=2;

		}
		// 3. ������ ó������ �׽�Ʈ
		else if (!wcsncmp(m_wszJapaneseText, wszJapaneseText, nLen))
		{

			// �ߺ����
			m_nNextKorIdx = 0;
			m_nNextTestIdx = nLen;
			m_nDupCnt++;
			nRetVal = 2;

		}

		else
		{
			// ��򰡿����� �ߺ�
			// Ȥ�� ���� ������� ����

			int i;

			// ��򰡿��� �ߺ��Ǿ����� üũ
			for (i = m_nNextKorIdx; i < (int)m_nJapaneseLen; i+=nLen)
			{
				if (!wcsncmp(m_wszJapaneseText + i, wszJapaneseText, nLen))
				{
					m_nNextKorIdx = i;
					m_nNextTestIdx = i+nLen;
					m_nHitCnt++;
					nRetVal=1;

					break;
				}

			}
			if (i >= (int)m_nJapaneseLen)
			{
				for (i = 0; i < m_nNextKorIdx; i+=nLen)
				{
					m_nNextKorIdx = i;
					m_nNextTestIdx = i+nLen;
					m_nDupCnt++;
					nRetVal = 3;

					break;
				}
			}
		}

		if (nRetVal == 0 && m_nFaultCnt < 50)
		{
			// ����
			nRetVal = 4;
			m_nHitCnt = 0;
			m_nFaultCnt ++;
		}
	}

	return nRetVal;
}

int	CNonCachedTextArg::GetTranslatedString(LPWSTR wszKoreanBuf, int nBufSize, int &nOrigLen)
{
	if (m_nKoreanLen < (size_t)m_nNextKorIdx)
	{
		// ���̻� ����� �ѱ��� ����.
		wszKoreanBuf[0]=L'\0';
		return 0;
	}

	// MATCHLEN �� FALSE �̰� ������ �����̸� ������ �ִ� ���ڸ� ���
	if (!CNonCachedTextArgMgr::GetInstance()->IsMatchLen() && (m_nNextKorIdx+nOrigLen >= (int)m_nJapaneseLen))
		nOrigLen = nBufSize - 1;


	if (nOrigLen > nBufSize)
		nOrigLen = nBufSize - 1;

	lstrcpynW(wszKoreanBuf, m_wszKoreanText+m_nNextKorIdx, nOrigLen + 1);

	nOrigLen = lstrlenW(wszKoreanBuf);

	return nOrigLen;
}

#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include "../stdafx.h"
#include "CachedTextArgMgr.h"
#include "CachedTextArg.h"
#include "Function.h"
#include "../AutoFinder.h"

#define TEXT_ARG_POOL_SIZE 100

extern CCachedTextArgMgr g_objCachedTextArgMgr;
CCachedTextArgMgr*	CCachedTextArgMgr::_Inst = NULL;


CCachedTextArgMgr::CCachedTextArgMgr(void)
	: m_distBest(0)
{
	_Inst = this;
}


CCachedTextArgMgr::~CCachedTextArgMgr(void)
{
	_Inst = NULL;
	Close();
}

// �ʱ�ȭ
BOOL CCachedTextArgMgr::Init() 
{
	// �ؽ�Ʈ ���� Ǯ ����
	for(int i=0; i<TEXT_ARG_POOL_SIZE; i++)
	{
		m_setInactivatedArgs.insert(new CCachedText());
	}

	return TRUE;
}

// ����ȭ
void CCachedTextArgMgr::Close()
{

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ����
	for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CCachedText* pCachedText = *(iter);
		delete pCachedText;
	}
	m_setActivatedArgs.clear();
	
	// ��Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ����
	for(CCachedTextSet::iterator iter = m_setInactivatedArgs.begin();
		iter != m_setInactivatedArgs.end();
		iter++)
	{
		CCachedText* pCachedText = *(iter);
		delete pCachedText;
	}
	m_setInactivatedArgs.clear();

	// ������ �Ÿ��� ����
	m_mapHitDist.clear();
	m_distBest = 0;

	// �Լ� ��� ����
	for( CFunctionMap::iterator iter2 = m_mapFunc.begin();
		iter2 != m_mapFunc.end();
		iter2++)
	{
		CFunction* pFunc = iter2->second;
		delete pFunc;
	}
	m_mapFunc.clear();

	// ��ŷ�� �Լ��� ����
	for( CArgInfoMap::iterator iter3 = m_mapArgInfoA.begin();
		iter3 != m_mapArgInfoA.end();
		iter3++)
	{
		MAINAPP->m_sContainerFunc.procUnhookCodePoint( (LPVOID)iter3->first, ModifyValueA );
	}
	m_mapArgInfoA.clear();

	for( CArgInfoMap::iterator iter4 = m_mapArgInfoW.begin();
		iter4 != m_mapArgInfoW.end();
		iter4++)
	{
		MAINAPP->m_sContainerFunc.procUnhookCodePoint( (LPVOID)iter4->first, ModifyValueW );
	}
	m_mapArgInfoW.clear();
	

	// ��ŷ����迭 ����
	m_setReservedHooks.clear();

}



// ���ο� ���ڿ� �ĺ��� �߰��Ѵ�
int CCachedTextArgMgr::AddTextArg(LPVOID pText, BOOL bWideChar, BOOL bAutoTrans, UINT_PTR ptrFunc, size_t dist)
{
	BOOL nRetVal = 0;

	CCachedText* pCachedText = NULL;
	
	// ������ ����ִ� ���ڿ����� �˻� (Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ)
	for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CCachedText* pTmpCachedText = *(iter);

		if( pTmpCachedText->IsWideCharacter() == bWideChar && pTmpCachedText->TestText(pText) )
		{
			pCachedText = pTmpCachedText;
			nRetVal = 2;
		}		
	}		


	// ������ �ʿ��ϸ� ���� �Ǵ� �־��� ��带 ������ �߰��Ѵ�
	if( NULL == pCachedText)
	{	
		if(!m_setInactivatedArgs.empty()) 
		{
			CCachedTextSet::iterator iter = m_setInactivatedArgs.begin();
			pCachedText = ( *iter );
			m_setInactivatedArgs.erase(pCachedText);
			m_setActivatedArgs.insert(pCachedText);
		}
		else
		{
			// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ
			for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
				iter != m_setActivatedArgs.end();
				iter++)
			{
				CCachedText* pTmpCachedText = *(iter);

				if( NULL == pCachedText || (pCachedText->GetHitCount() > pTmpCachedText->GetHitCount()) )
				{
					pCachedText = pTmpCachedText;
				}		
			}		
		}
			

		if( pCachedText->SetCachedText(pText, bWideChar) )
		{
			nRetVal = 1;
		}
		else
		{
			m_setActivatedArgs.erase( pCachedText );
			m_setInactivatedArgs.insert(pCachedText);
			pCachedText = NULL;
		}
	}

	// ����� �����̸� 
	if( nRetVal!=0 )
	{
		// ��� ���� ���� �߰�
		if( ptrFunc && dist )
		{
			pCachedText->m_setFuncArg.insert( pair<UINT_PTR,size_t>(ptrFunc,dist) );
		}

		// ������ �ʿ��ϸ� ����
		if( bAutoTrans )
		{
			pCachedText->Translate();
		}

		wchar_t dbg[2048];
		swprintf(dbg, L"[ aral1 ] %c[0x%p] '%s'('%s') \n",
			(pCachedText->m_bTranslated ? L'��' : L'��'),
			pText,
			pCachedText->m_wszJapaneseText, 
			pCachedText->m_wszKoreanText
			);
		//OutputDebugStringW(dbg);

	}

	return nRetVal;
}

// ���ڿ� �ĺ��� ��ü�� �׽�Ʈ�Ѵ�. (���̻� ��ġ���� �ʴ� �ĺ��� �ٷ� ����)
BOOL CCachedTextArgMgr::TestCharacter(wchar_t wch, void* baseESP)
{
	BOOL bRetVal = FALSE;

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ��� ��� ��ȸ
	for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();)
	{
		CCachedText* pCachedText = (*iter);
		iter++;
		
		// �˻� ����
		int nRes = pCachedText->TestCharacter(wch);
		
		// ��Ʈ(����)��
		if( 0 != nRes )
		{
			if( nRes & 0x01 )
			{
				ModifyHitMap(pCachedText, baseESP, +1);
				bRetVal = TRUE;

				for(set<pair<UINT_PTR,size_t>>::iterator iter = pCachedText->m_setFuncArg.begin();
					iter != pCachedText->m_setFuncArg.end();
					iter++)
				{
					CFunction* pFunc = m_mapFunc[iter->first];
					size_t distArg = iter->second;
					
					if(pFunc && distArg)
					{
						pFunc->m_mapDistScores[distArg]++;
						
						// ���� Ư����ġ �̻� ���ߵǾ����� �̺κ� ��ŷ
						if( pFunc->m_mapDistScores[distArg] > 30 
							&& MAINAPP->m_sContainerFunc.procHookCodePoint
							&& m_mapArgInfoA.find(pFunc->m_ptrFunction) == m_mapArgInfoA.end()
							&& m_mapArgInfoW.find(pFunc->m_ptrFunction) == m_mapArgInfoW.end() )
						{
							HMODULE hExeMod = GetModuleHandle(NULL);
							HMODULE hHookMod = NULL;

							if( GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)pFunc->m_ptrFunction, &hHookMod)
								&& hExeMod == hHookMod )
							{
								/*
								PRESERVED_HOOK_POINT pRHP = new RESERVED_HOOK_POINT;
								pRHP->bWideChar	= pCachedText->m_bWideChar;
								pRHP->nArgDist	= distArg;
								pRHP->pHookPoint	= pFunc->m_ptrFunction;
								m_setReservedHooks.insert(pRHP);
								TRACE("[ aral1 ] Function 0x%p(+%d) was reserved for hook \n", pFunc->m_ptrFunction, distArg);
								*/
							}
						}

					}
				}
			}
		}
		// ����
		else
		{
			//// ���� ���� ����
			//for(set<void*>::iterator iter = pCachedText->m_setSourcePtr.begin();
			//	iter != pCachedText->m_setSourcePtr.end();
			//	iter++)
			//{
			//	void* pSource = (*iter);
			//	size_t dist = (size_t)pSource - (size_t)baseESP;
			//	ModifyHitMap(dist, -1);
			//}
			for(set<pair<UINT_PTR,size_t>>::iterator iter = pCachedText->m_setFuncArg.begin();
				iter != pCachedText->m_setFuncArg.end();
				iter++)
			{
				CFunction* pFunc = m_mapFunc[iter->first];
				size_t distArg = iter->second;

				if(pFunc && distArg)
				{
					pFunc->m_mapDistScores[distArg]--;
				}
			}

			ModifyHitMap(pCachedText, baseESP, -1);
			m_setActivatedArgs.erase(pCachedText);
			m_setInactivatedArgs.insert(pCachedText);
		}
	}	

	return bRetVal;
}


UINT_PTR CCachedTextArgMgr::GetFuncAddrFromReturnAddr(UINT_PTR pAddr)
{
	UINT_PTR funcAddr = NULL;

	__try
	{
		if( !IsBadReadPtr( (void*)pAddr, sizeof(void*) ) )
		{
			// �Լ��� ������� �˻�
			BYTE* pRetAddr = (BYTE*)pAddr;
			if( 0xE8 == *(pRetAddr-5) )	// call �ڵ� case 1
			{
				UINT_PTR func_dist = *( (UINT_PTR*)(pRetAddr-4) );		// �̵��Ÿ� ���ϱ�
				funcAddr = pAddr + func_dist;							// ������ �Լ��ּ�
			}
			else if( 0xFF == *(pRetAddr-6) && 0x15 == *(pRetAddr-5) )	// call �ڵ� case 2
			{
				funcAddr = **( (UINT_PTR**)(pRetAddr-4) );				// ������ �Լ��ּ�
			}


			// �Լ� ������ ��ȿ�� �˻�
			if( funcAddr && IsBadCodePtr((FARPROC)funcAddr) )
			{
				funcAddr = NULL;
			}

		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}

	return funcAddr;

}


int CCachedTextArgMgr::SearchStringA(INT_PTR ptrBegin, char ch1, char ch2)
{
	int iRetVal = 0;

	// ���ڰŸ��� ����
	//FindBestDistAndClearHitMap();
	
	size_t dist = 0;
	size_t arg_dist = 0;
	CFunction* pCurFunc = NULL;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPSTR* ppText = (LPSTR*)(ptrBegin+dist);
		LPVOID found = NULL;

		if( IsBadReadPtr(*ppText, sizeof(LPSTR))==FALSE && IsBadStringPtrA(*ppText, 1024)==FALSE )
		{
			size_t nLen = strnlen(*ppText, 1024);
			if(nLen < 1024)
			{
				if('\0'==ch2)
				{
					found = (LPVOID)strchr(*ppText, ch1);
				}
				else
				{
					char tmp[4] = {0,};
					tmp[0] = ch1;
					tmp[1] = ch2;
					found = (LPVOID)strstr(*ppText, tmp);
				}

			}
		}

		// ���ڿ��̶��
		if(found)
		{
			int nAddRes = AddTextArg( *ppText, FALSE, IsAutoTransPoint(dist), (pCurFunc?pCurFunc->m_ptrFunction:NULL), arg_dist );
			if( nAddRes )
			{
				// ���� ���� ���
				m_mapHitDist.insert( pair<size_t, int>(dist, 0) );
				iRetVal++;

				//// �Լ� ���� ����
				//if( pCurFunc )
				//{
				//	pCurFunc->m_mapDistScores[arg_dist]++;
				//}

			}
		}
		else
		{
			UINT_PTR funcAddr = GetFuncAddrFromReturnAddr( *((UINT_PTR*)(ptrBegin+dist)) );

			// �Լ� �����ּҶ�� ���� ������ �Լ��� ��ü�Ѵ�
			if( funcAddr )
			{
				CFunctionMap::iterator iter = m_mapFunc.find( funcAddr );
				// ���� ����Ʈ�� �����ϸ�
				if( iter != m_mapFunc.end() )
				{
					pCurFunc = iter->second;
				}
				// ������ ���� ���� & �߰�
				else
				{
					pCurFunc = new CFunction(funcAddr);
					m_mapFunc[funcAddr] = pCurFunc;
				}
				
				arg_dist = 0;
			}
		}

		dist += sizeof(void*);
		arg_dist += sizeof(void*);
	}
	
	//char dbg[1024];
	//sprintf(dbg, " [ aral1 ] ã���Ÿ�:0x%p~0x%p (%d bytes) \n", ptrBegin, ptrBegin+dist, dist); OutputDebugStringA(dbg);

	return iRetVal;
}

int CCachedTextArgMgr::SearchStringW(INT_PTR ptrBegin, wchar_t wch)
{
	int iRetVal = 0;

	// ���ڰŸ��� ����
	//FindBestDistAndClearHitMap();

	size_t dist = 0;
	size_t arg_dist = 0;
	CFunction* pCurFunc = NULL;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPWSTR* ppText = (LPWSTR*)(ptrBegin+dist);

		// ���ڿ��̶��
		if( IsBadReadPtr(*ppText, sizeof(LPWSTR))==FALSE && IsBadStringPtrW(*ppText, 1024)==FALSE && wcsnlen(*ppText,1024)==1024 && NULL!=wcschr(*ppText, wch) )
		{

			int nAddRes = AddTextArg( *ppText, TRUE, IsAutoTransPoint(dist), pCurFunc->m_ptrFunction, arg_dist );
			if( nAddRes )
			{
				// ���� ���� ���
				m_mapHitDist.insert( pair<size_t, int>(dist, 0) );
				iRetVal++;

				//// �Լ� ���� ����
				//if( pCurFunc )
				//{
				//	pCurFunc->m_mapDistScores[arg_dist]++;
				//}

			}
		}
		else
		{
			UINT_PTR funcAddr = GetFuncAddrFromReturnAddr( *((UINT_PTR*)(ptrBegin+dist)) );

			// �Լ� �����ּҶ�� ���� ������ �Լ��� ��ü�Ѵ�
			if( funcAddr )
			{
				CFunctionMap::iterator iter = m_mapFunc.find( funcAddr );
				// ���� ����Ʈ�� �����ϸ�
				if( iter != m_mapFunc.end() )
				{
					pCurFunc = iter->second;
				}
				// ������ ���� ���� & �߰�
				else
				{
					pCurFunc = new CFunction(funcAddr);
					m_mapFunc[funcAddr] = pCurFunc;
				}

				arg_dist = 0;
			}
		}

		dist += sizeof(void*);
		arg_dist += sizeof(void*);

	}	

	return iRetVal;
}



// �ְ�� Ȯ���� ���� ���� ���ڸ� ��ȯ
wchar_t CCachedTextArgMgr::GetBestTranslatedCharacter() 
{
	wchar_t wchRetVal = L'\0';
	CCachedText* pBestArg = NULL;

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ
	for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CCachedText* pCachedTextArg = *(iter);

		if( NULL == pBestArg )
		{
			if( 0 < pCachedTextArg->GetHitCount() ) pBestArg = pCachedTextArg;
		}
		else if( pBestArg->m_bTranslated < pCachedTextArg->m_bTranslated )
		{
			pBestArg = pCachedTextArg;
		}
		else if( pBestArg->m_nHitCnt < pCachedTextArg->m_nHitCnt )
		{
			pBestArg = pCachedTextArg;
		}
		else if( pBestArg->m_nHitCnt == pCachedTextArg->m_nHitCnt )
		{
			size_t nBestRef = pBestArg->m_setSourcePtr.size();
			size_t nTempRef = pCachedTextArg->m_setSourcePtr.size();
			if( nBestRef < nTempRef )
			{
				pBestArg = pCachedTextArg;				
			}
			else if( nBestRef == nTempRef && pBestArg->m_nJapaneseLen < pCachedTextArg->m_nJapaneseLen )
			{
				pBestArg = pCachedTextArg;
			}
		}
	}

	if(pBestArg)
	{
		wchRetVal = pBestArg->GetBestTranslatedCharacter();
		//wchar_t dbg[1024];
		//swprintf(dbg, L"[ aral1 ] GetBestTranslatedCharacter() returned '%c' ('%s'[%d]) \n", (wchRetVal ? wchRetVal:L'0'), pBestArg->m_wszKoreanText, pBestArg->m_nNextTestIdx-1);
		//OutputDebugStringW(dbg);
	}

	return wchRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// ���� ���� ���� ó��
//
//////////////////////////////////////////////////////////////////////////
void CCachedTextArgMgr::ModifyHitMap( CCachedText* pCachedText, void* baseESP, int increment ) 
{
	// ���� ���� ����
	for(map<size_t,int>::iterator iter = m_mapHitDist.begin();
		iter != m_mapHitDist.end();)
	{
		void** ppSource = (void**)( (UINT_PTR)baseESP + iter->first );
		
		// �ؽ�Ʈ���ڿ� �ش� ���ڿ� �����Ͱ� ������
		if( IsBadReadPtr(ppSource, sizeof(void*)) == FALSE
			 && pCachedText->m_setSourcePtr.find(*ppSource) != pCachedText->m_setSourcePtr.end() )
		{
			iter->second += increment;
		}

		if( iter->second > 10 )
		{
			iter->second = 10;
			iter++;
		}
		// ���̻� �ʿ������ ����
		else if( iter->second < 0 )
		{
			size_t key = iter->first;
			iter++;
			m_mapHitDist.erase(key);
		}
		// ���� �����ؾ� �Ѵٸ�
		else
		{
			iter++;
		}
	}


	//map<size_t,int>::iterator iter = m_mapHitDist.find(dist);
	//
	//// �ű� �߰��� �ʿ��ϸ�
	//if( iter == m_mapHitDist.end() )
	//{
	//	if( increment > 0 ) m_mapHitDist.insert( pair<size_t,int>(dist, increment) );
	//}
	//// ���� �� ���۽�
	//else
	//{
	//	iter->second += increment;
	//	if( iter->second <= 0 )
	//	{
	//		m_mapHitDist.erase(iter);
	//	}
	//}
}


void CCachedTextArgMgr::FindBestDistAndClearHitMap()
{
	int nBestVal = 0;

	// �ְ� ���� ����
	for(map<size_t,int>::iterator iter = m_mapHitDist.begin();
		iter != m_mapHitDist.end();
		iter++)
	{
		if( iter->second > nBestVal && iter->second > 2 )
		{
			m_distBest = iter->first;
			nBestVal = iter->second;
		}
	}

	m_mapHitDist.clear();

}

//////////////////////////////////////////////////////////////////////////
//
// �ڵ� ������������ �Ǵ��Ѵ�
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::IsAutoTransPoint( size_t dist ) 
{
	BOOL bRetVal = FALSE;

	map<size_t,int>::iterator iter = m_mapHitDist.find( dist );
	if( iter != m_mapHitDist.end() && iter->second > 3 )
	{
		bRetVal = TRUE;
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// ����� ������ ��ŷ�Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
void CCachedTextArgMgr::HookAllReservedPoints()
{
	for(CReservedHooks::iterator iter = m_setReservedHooks.begin();
		iter != m_setReservedHooks.end();
		iter++)
	{

		PRESERVED_HOOK_POINT pRHP = (*iter);

		// �����ڵ��� ���
		if(pRHP->bWideChar)
		{
			BOOL bHooked = MAINAPP->m_sContainerFunc.procHookCodePoint( (LPVOID)pRHP->pHookPoint, ModifyValueW, 5 );

			if( bHooked )
			{
				m_mapArgInfoW.insert( CArgInfo(pRHP->pHookPoint, pRHP->nArgDist) );		
				TRACE("[ aral1 ] Function 0x%p(+%d) was hooked as unicode text \n", pRHP->pHookPoint, pRHP->nArgDist);
			}
		}
		// ��Ƽ����Ʈ�� ���
		else
		{
			BOOL bHooked = MAINAPP->m_sContainerFunc.procHookCodePoint( (LPVOID)pRHP->pHookPoint, ModifyValueA, 5 );

			if( bHooked )
			{
				m_mapArgInfoA.insert( CArgInfo(pRHP->pHookPoint, pRHP->nArgDist) );		
				TRACE("[ aral1 ] Function 0x%p(+%d) was hooked as multibyte text \n", pRHP->pHookPoint, pRHP->nArgDist);
			}
		}

		delete pRHP;
	}

	m_setReservedHooks.clear();


}

//////////////////////////////////////////////////////////////////////////
//
// Ư������ ��ŷ�� �ݹ� �Լ� (��Ƽ����Ʈ��)
//
//////////////////////////////////////////////////////////////////////////
void CCachedTextArgMgr::ModifyValueA(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters)
{
	if(NULL==CCachedTextArgMgr::_Inst) return;

	// �������� ���̺��� ���� ���ϱ�
	CArgInfoMap::iterator iter = CCachedTextArgMgr::_Inst->m_mapArgInfoA.find((UINT_PTR)pHookedPoint);
	if( iter != CCachedTextArgMgr::_Inst->m_mapArgInfoA.end() )
	{
		size_t dist = iter->second;
		LPSTR* pArgText = (LPSTR*)( pRegisters->_ESP + dist );		// �ؽ�Ʈ ���� ������ ���ϱ�
		if( CCachedTextArgMgr::_Inst && IsBadReadPtr(pArgText, sizeof(LPSTR)) == FALSE 
			&& IsBadStringPtrA(*pArgText, 1024*1024*1024) == FALSE )
		{
			LPSTR pText = *pArgText;
			char ch1 = pText[0];
			char ch2 = pText[1];
			int nRes = CCachedTextArgMgr::_Inst->AddTextArg( pText, FALSE, TRUE, NULL, NULL );
			if( nRes == 1 )
			{
				CCachedTextArgMgr::_Inst->SearchStringA((INT_PTR)pRegisters->_ESP, ch1, ch2 );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Ư������ ��ŷ�� �ݹ� �Լ� (�����ڵ��)
//
//////////////////////////////////////////////////////////////////////////
void CCachedTextArgMgr::ModifyValueW(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters)
{
	if(NULL==CCachedTextArgMgr::_Inst) return;

	// �������� ���̺��� ���� ���ϱ�
	CArgInfoMap::iterator iter = CCachedTextArgMgr::_Inst->m_mapArgInfoW.find((UINT_PTR)pHookedPoint);
	if( iter != CCachedTextArgMgr::_Inst->m_mapArgInfoW.end() )
	{
		size_t dist = iter->second;
		LPWSTR* pArgText = (LPWSTR*)( pRegisters->_ESP + dist );		// �ؽ�Ʈ ���� ������ ���ϱ�
		if( CCachedTextArgMgr::_Inst && IsBadReadPtr(pArgText, sizeof(LPWSTR)) == FALSE 
			&& IsBadStringPtrW(*pArgText, 1024*1024*1024) == FALSE )
		{
			LPWSTR pText = *pArgText;
			wchar_t wch = pText[0];
			int nRes = CCachedTextArgMgr::_Inst->AddTextArg( pText, TRUE, TRUE, NULL, NULL );
			if( nRes == 1 )
			{
				CCachedTextArgMgr::_Inst->SearchStringW((INT_PTR)pRegisters->_ESP, wch );
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CCachedTextArgMgr::NewGetGlyphOutlineA(
	  HDC hdc,             // handle to device context
	  UINT uChar,          // character to query
	  UINT uFormat,        // format of data to return
	  LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	  DWORD cbBuffer,      // size of buffer for data
	  LPVOID lpvBuffer,    // pointer to buffer for data
	  CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	  )
{

	char chArray[10] = {0,};
	char tmpbuf[10]  = {0,};
	wchar_t wchArray[10] = {0,};
	wchar_t wch = L'\0';

	// �ٷ� �� �� EBP�� ����
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	// char �迭�� ���� ����
	size_t i,j;
	j = 0;
	for(i=sizeof(UINT); i>0; i--)
	{
		char one_ch = *( ((char*)&uChar) + i - 1 );
		if(one_ch)
		{
			chArray[j] = one_ch;
			j++;
		}
	}

	MyMultiByteToWideChar(932, 0, chArray, sizeof(UINT), wchArray, 10 );
	wch = wchArray[0];

	// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
	BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

	// ���ߵ� �����Ͱ� ���ٸ� �˻�
	if( FALSE == bHitOnce && wch > 0x80)
	{			
		// ����� ��ŷ�۾� ����
		HookAllReservedPoints();

		// �˻�
		int iRes = SearchStringA(_CUR_EBP, chArray[0], chArray[1]);
		if(iRes)
		{
			TestCharacter(wch, (void*)_CUR_EBP);
		}
	}

	//wchar_t dbg[MAX_PATH];
	//swprintf(dbg, L"[ aral1 ] NewTextOutA���� ĳ�� : '%c'( %2X %2X ) \n", wch, lpString[0], lpString[1]);
	//OutputDebugStringW(dbg);

	DWORD dwRetVal = 0;
	wchar_t wtmp[2] = {0,};
	wtmp[0] = GetBestTranslatedCharacter();
	if(wtmp[0])
	{
		uChar = wtmp[0];

		if(uChar<=0x20)
		{
			uFormat = GGO_NATIVE;
		}

		dwRetVal = GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
	else
	{
		// �����Լ� ȣ��
		dwRetVal = GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CCachedTextArgMgr::NewGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	/*
	// �ٷ� �� �� EBP�� ����
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	wchar_t wch = (wchar_t)uChar;

	// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
	BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

	// ���ߵ� �����Ͱ� ���ٸ� �˻�
	if( FALSE == bHitOnce && wch > 0x80 )
	{			
		// ����� ��ŷ�۾� ����
		HookAllReservedPoints();

		// �˻�
		int iRes = SearchStringW(_CUR_EBP, wch);
		if(iRes)
		{
			TestCharacter(wch, (void*)_CUR_EBP);
		}
		TRACE("[ aral1 ] ============== �� �˻� ��� : %d ================= \n", iRes );			
	}

	//wchar_t dbg[MAX_PATH];
	//swprintf(dbg, L"[ aral1 ] NewTextOutA���� ĳ�� : '%c'( %2X %2X ) \n", wch, lpString[0], lpString[1]);
	//OutputDebugStringW(dbg);

	wch = GetBestTranslatedCharacter();
	if(wch)
	{
		//wchar_t dbg[MAX_PATH];
		//swprintf(dbg, L"[ aral1 ] MyGetGlyphOutlineW : '%c'->'%c' \n", (wchar_t)uChar, wch);
		//OutputDebugStringW(dbg);
		uChar = (UINT)wch;
	}
	else
	{
		uChar = 0x20;
	}

	// �����Լ� ȣ��
	DWORD dwRetVal = 0;

	if( m_sTextFunc.pfnGetGlyphOutlineW )
	{
		if(uChar<=0x80)
		{
			uFormat = GGO_NATIVE;
		}

		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
	*/

	return GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::NewTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = FALSE;

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	if( cbString<=2 || strlen(lpString)<=2 )
	{
		wchar_t wchArray[10] = {0,};
		MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );		
		wchar_t wch = wchArray[0];

		// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
		BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

		// ���ߵ� �����Ͱ� ���ٸ� �˻�
		if( FALSE == bHitOnce && wch > 0x80 )
		{			
			// ����� ��ŷ�۾� ����
			HookAllReservedPoints();
			
			// �˻�
			TRACE("[ aral1 ] \n");			
			TRACE("[ aral1 ] \n");						
			int iRes = SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
			if(iRes)
			{
				TestCharacter(wch, (void*)_CUR_EBP);
			}
			TRACE("[ aral1 ] ============== �� �˻� ��� : (%d/%d) ================= \n", m_setActivatedArgs.size(), iRes );
		}


		// ���� ��ȯ
		wchar_t wtmp[2] = {0,};
		wtmp[0] = GetBestTranslatedCharacter();
		if( L'\0' == wtmp[0] ) wtmp[0] = L' ';

		TextOutW(hdc, nXStart, nYStart, wtmp, 1);
	}
	else
	{
		// �����Լ� ȣ��
		TextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::NewTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	return TextOutW(hdc, nXStart, nYStart, lpString, cbString);
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::NewExtTextOutA(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	return ExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::NewExtTextOutW(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCWSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	return ExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
int CCachedTextArgMgr::NewDrawTextA(
						 HDC hDC,          // handle to DC
						 LPCSTR lpString,  // text to draw
						 int nCount,       // text length
						 LPRECT lpRect,    // formatting dimensions
						 UINT uFormat      // text-drawing options
						 )
{
	return DrawTextA(hDC, lpString, nCount, lpRect, uFormat);
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
int CCachedTextArgMgr::NewDrawTextW(
						 HDC hDC,          // handle to DC
						 LPCWSTR lpString, // text to draw
						 int nCount,       // text length
						 LPRECT lpRect,    // formatting dimensions
						 UINT uFormat      // text-drawing options
						 )
{
	return DrawTextW(hDC, lpString, nCount, lpRect, uFormat);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
int CCachedTextArgMgr::NewDrawTextExA(
						   HDC hdc,                     // handle to DC
						   LPSTR lpchText,              // text to draw
						   int cchText,                 // length of text to draw
						   LPRECT lprc,                 // rectangle coordinates
						   UINT dwDTFormat,             // formatting options
						   LPDRAWTEXTPARAMS lpDTParams  // more formatting options
						   )
{
	return DrawTextExA(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
int CCachedTextArgMgr::NewDrawTextExW(
						   HDC hdc,                     // handle to DC
						   LPWSTR lpchText,              // text to draw
						   int cchText,                 // length of text to draw
						   LPRECT lprc,                 // rectangle coordinates
						   UINT dwDTFormat,             // formatting options
						   LPDRAWTEXTPARAMS lpDTParams  // more formatting options
						   )
{
	return DrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
}

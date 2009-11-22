
#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include <process.h>
#include "stdafx.h"
#include "ATCodeMgr.h"
#include "HookPoint.h"
//#include "RegistryMgr/cRegistryMgr.h"
//#include "CharacterMapper.h"
#include "OptionDlg.h"

CATCodeMgr*	CATCodeMgr::_Inst = NULL;

CATCodeMgr* CATCodeMgr::GetInstance()
{
	return _Inst;
}


CATCodeMgr::CATCodeMgr(void)
  : m_hContainer(NULL), 
	m_hContainerWnd(NULL), 
	m_wszOptionString(NULL), 
	m_bRunClipboardThread(FALSE),
	m_hClipboardThread(NULL),
	m_hClipTextChangeEvent(NULL)
	//m_pfnLoadLibraryA(NULL),
	//m_pfnLoadLibraryW(NULL)
{
	_Inst = this;
	ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_PROC_ENTRY));
	InitializeCriticalSection(&m_csClipText);
}

CATCodeMgr::~CATCodeMgr(void)
{
	DeleteCriticalSection(&m_csClipText);
	Close();
	_Inst = NULL;
}


BOOL CATCodeMgr::Init( HWND hAralWnd, LPWSTR wszPluginOption )
{
	Close();

	BOOL bRetVal = FALSE;

	// �θ� ������ �ڵ� ����
	if(NULL==hAralWnd) return FALSE;
	m_hContainerWnd = hAralWnd;

	// �����̳� �Լ� ������ ������
	m_hContainer = GetModuleHandle(_T("ATCTNR3.DLL"));
	if(m_hContainer && INVALID_HANDLE_VALUE != m_hContainer)
	{
		ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_PROC_ENTRY));
		m_sContainerFunc.procHookWin32Api		= (PROC_HookWin32Api) GetProcAddress(m_hContainer, "HookWin32Api");
		m_sContainerFunc.procUnhookWin32Api		= (PROC_UnhookWin32Api) GetProcAddress(m_hContainer, "UnhookWin32Api");
		m_sContainerFunc.procHookCodePoint		= (PROC_HookCodePoint) GetProcAddress(m_hContainer, "HookCodePoint");
		m_sContainerFunc.procUnhookCodePoint	= (PROC_UnhookCodePoint) GetProcAddress(m_hContainer, "UnhookCodePoint");
		m_sContainerFunc.procCreateTransCtx		= (PROC_CreateTransCtx) GetProcAddress(m_hContainer, "CreateTransCtx");
		m_sContainerFunc.procDeleteTransCtx		= (PROC_DeleteTransCtx) GetProcAddress(m_hContainer, "DeleteTransCtx");
		m_sContainerFunc.procTranslateUsingCtx	= (PROC_TranslateUsingCtx) GetProcAddress(m_hContainer, "TranslateUsingCtx");
		m_sContainerFunc.procIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress(m_hContainer, "IsAppLocaleLoaded");
		m_sContainerFunc.procSuspendAllThread	= (PROC_SuspendAllThread) GetProcAddress(m_hContainer, "SuspendAllThread");
		m_sContainerFunc.procResumeAllThread	= (PROC_ResumeAllThread) GetProcAddress(m_hContainer, "ResumeAllThread");
		m_sContainerFunc.procIsAllThreadSuspended = (PROC_IsAllThreadSuspended) GetProcAddress(m_hContainer, "IsAllThreadSuspended");
	}

	if( m_sContainerFunc.procHookWin32Api && m_sContainerFunc.procUnhookWin32Api
		&& m_sContainerFunc.procHookCodePoint && m_sContainerFunc.procUnhookCodePoint
		&& m_sContainerFunc.procCreateTransCtx && m_sContainerFunc.procDeleteTransCtx
		&& m_sContainerFunc.procTranslateUsingCtx && m_sContainerFunc.procIsAppLocaleLoaded
		&& m_sContainerFunc.procSuspendAllThread && m_sContainerFunc.procResumeAllThread
		&& m_sContainerFunc.procIsAllThreadSuspended)
	{
		// ��� ������ ����
		m_sContainerFunc.procSuspendAllThread();

		// LoadLibrary �Լ� ��ŷ	 
		if( m_sContainerFunc.procHookWin32Api( L"kernel32.dll", L"LoadLibraryA", NewLoadLibraryA, 1 ) )
		{
			//m_pfnLoadLibraryA = (PROC_LoadLibrary) m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "LoadLibraryA");
		}

		if( m_sContainerFunc.procHookWin32Api( L"kernel32.dll", L"LoadLibraryW", NewLoadLibraryW, 1 ) )
		{
			//m_pfnLoadLibraryW = (PROC_LoadLibrary) m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "LoadLibraryW");
		}

		// Ŭ������ ����
		m_bRunClipboardThread = TRUE;
		m_hClipTextChangeEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hClipboardThread = (HANDLE)_beginthreadex(NULL, 0, ClipboardThreadFunc, NULL, 0, NULL);

		// �ɼ� ��Ʈ�� �Ľ�
		m_wszOptionString = wszPluginOption;

		if(m_wszOptionString == NULL)
		{
			m_wszOptionString = new wchar_t[MAX_OPTION_LEN];
			ZeroMemory(m_wszOptionString, MAX_OPTION_LEN);
		}

#ifdef UNICODE
		CString strOptionString	= m_wszOptionString;		
#else
		CString strOptionString;
		WideCharToMultiByte(CP_ACP, 0, m_wszOptionString, -1, strOptionString.GetBufferSetLength(MAX_OPTION_LEN), MAX_OPTION_LEN, NULL, NULL);
#endif

		if( m_optionRoot.ParseChildren(strOptionString) )
		{
			bRetVal = TRUE;
		}
			
		// ��� ������ �簡��
		m_sContainerFunc.procResumeAllThread();

	}

	if( bRetVal == TRUE )
	{
		// �ɼ� ����
		AdjustOption(&m_optionRoot);
	}
	else
	{
		Close();
	}

	return bRetVal;
}


BOOL CATCodeMgr::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

	// Ŭ������ ������ ����
	m_bRunClipboardThread = FALSE;
	if(m_hClipboardThread && m_hClipTextChangeEvent)
	{
		::SetEvent(m_hClipTextChangeEvent);
		//::WaitForSingleObject(m_hClipboardThread, 3000);
		::CloseHandle(m_hClipboardThread);
		::CloseHandle(m_hClipTextChangeEvent);
	}

	// ��� ������ ����
	m_sContainerFunc.procSuspendAllThread();

	ResetOption();

	m_hClipboardThread = NULL;
	m_hClipTextChangeEvent = NULL;
	
	// �ɼ� ��ü �ʱ�ȭ
	m_optionRoot.ClearChildren();
	
	// LoadLibrary ����
	TRACE(_T("kernel32.DLL!LoadLibraryA Unhook... \n"));
	m_sContainerFunc.procUnhookWin32Api( L"kernel32.dll", L"LoadLibraryA", NewLoadLibraryA );
	TRACE(_T("kernel32.DLL!LoadLibraryW Unhook... \n"));
	m_sContainerFunc.procUnhookWin32Api( L"kernel32.dll", L"LoadLibraryW", NewLoadLibraryW );
	
	// ��Ÿ ���� ����
	m_hContainerWnd = NULL;
	m_wszOptionString = NULL;

	// ��� ������ �簡��
	m_sContainerFunc.procResumeAllThread();

	return TRUE;
}

BOOL CATCodeMgr::Option()
{
	BOOL bRetVal = TRUE;

	CString strCurOptionString = m_optionRoot.ChildrenToString();
	
	COptionNode tmpRoot;
	if( tmpRoot.ParseChildren(strCurOptionString) == FALSE ) return FALSE;

	COptionDlg od;
	od.SetRootOptionNode(&tmpRoot);
	if( od.DoModal() == IDOK )
	{
		ApplyOption(&tmpRoot);
	}

	return bRetVal;
}

BOOL CATCodeMgr::ApplyOption( COptionNode* pRootNode )
{
	BOOL bRetVal = FALSE;
	
	CString strCurOptionString = m_optionRoot.ChildrenToString();

	// ������Ѻ���
	if( AdjustOption(pRootNode) == FALSE || m_optionRoot.ParseChildren( pRootNode->ChildrenToString() ) == FALSE )
	{
		// ���и� ���󺹱�
		m_optionRoot.ParseChildren( strCurOptionString );
		AdjustOption(&m_optionRoot);
	}
	else
	{
		// ���� �����̸�
		CString strOptionString = m_optionRoot.ChildrenToString();

#ifdef UNICODE
		wcscpy_s(m_wszOptionString, MAX_OPTION_LEN, strOptionString);
#else
		CString strOptionString;
		WideCharToMultiByte(CP_ACP, 0, strOptionString, -1, m_wszOptionString, MAX_OPTION_LEN, NULL, NULL);
#endif

		bRetVal = TRUE;
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// �� �ɼ� ���� ������ ���� �ɼǵ��� ��� �ʱ�ȭ���ִ� �Լ�
//
//////////////////////////////////////////////////////////////////////////
void CATCodeMgr::ResetOption()
{
	// ��ŷ ��õ� ���� ����� Ŭ����
	m_listRetryHook.clear();
	
	// ��ŷ�� ATCode�� ����
	for(list<CHookPoint*>::iterator iter = m_listHookPoint.begin();
		iter != m_listHookPoint.end();
		iter++)
	{
		CHookPoint* pPoint = (*iter);
		delete pPoint;
	}
	m_listHookPoint.clear();
}


//////////////////////////////////////////////////////////////////////////
//
// ���� �������� ȣȯ�� ���� �ɼ� ���̱׷��̼�
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::MigrateOption(COptionNode* pRootNode)
{
	if(NULL == pRootNode) return FALSE;

	BOOL bRetVal = TRUE;
	BOOL bNeedMigration = FALSE;

	BOOL bPtrCheat = FALSE;
	BOOL bSOW = FALSE;
	BOOL bRemoveSpace = FALSE;
	BOOL bTwoByte = FALSE;

	int cnt = pRootNode->GetChildCount();
	for(int i=0; i<cnt; i++)
	{
		COptionNode* pNode = pRootNode->GetChild(i);
		CString strValue = pNode->GetValue().MakeUpper();

		// FORCEFONT �ɼ�
		if(strValue == _T("FORCEFONT"))
		{
			COptionNode* pLevelNode = pNode->GetChild(0);

			// ���� ���İ� ȣȯ�� ����
			if(NULL==pLevelNode)
			{
				pLevelNode = pNode->CreateChild();
				pLevelNode->SetValue(_T("10"));
			}
		}
		
		// PTRCHEAT �ɼ�
		else if(strValue == _T("PTRCHEAT"))
		{
			bNeedMigration = TRUE;
			bPtrCheat = TRUE;
		}
		// SOW �ɼ�
		else if(strValue == _T("SOW"))
		{
			bNeedMigration = TRUE;
			bSOW = TRUE;
		}
		// REMOVESPACE �ɼ�
		else if(strValue == _T("REMOVESPACE"))
		{
			bNeedMigration = TRUE;
			bRemoveSpace = TRUE;
		}
		// TWOBYTE �ɼ�
		else if(strValue == _T("TWOBYTE"))
		{
			bNeedMigration = TRUE;
			bTwoByte = TRUE;
		}
	}	

	// ���̱׷��̼��� �ʿ��ϸ�
	if(bNeedMigration)
	{
		// �ؽ�Ʈ ���� ��� ����
		CString strTransMethod;
		if(bPtrCheat) strTransMethod = _T("PTRCHEAT");
		else if (bSOW) strTransMethod = _T("SOW");
		else strTransMethod = _T("OVERWRITE");

		// �ʿ���� ��� ����
		pRootNode->DeleteChild(_T("PTRCHEAT"));
		pRootNode->DeleteChild(_T("SOW"));
		pRootNode->DeleteChild(_T("REMOVESPACE"));
		pRootNode->DeleteChild(_T("TWOBYTE"));
		
		// ������ ���� ��� HOOK ��忡 ����
		cnt = pRootNode->GetChildCount();
		for(int i=0; i<cnt; i++)
		{
			COptionNode* pNode = pRootNode->GetChild(i);
			CString strValue = pNode->GetValue().MakeUpper();

			// HOOK ���
			if(strValue == _T("HOOK"))
			{
				int cnt2 = pNode->GetChildCount();
				for(int j=0; j<cnt2; j++)
				{
					COptionNode* pTransNode = pNode->GetChild(j);
					strValue = pTransNode->GetValue().MakeUpper();

					// TRANS ���
					if(strValue == _T("TRANS"))
					{
						pTransNode->DeleteChild(_T("NOP"));
						pTransNode->DeleteChild(_T("PTRCHEAT"));
						pTransNode->DeleteChild(_T("OVERWRITE"));
						pTransNode->DeleteChild(_T("SOW"));
						pTransNode->DeleteChild(_T("REMOVESPACE"));
						pTransNode->DeleteChild(_T("TWOBYTE"));

						COptionNode* pChildNode = NULL;

						// �����۾� ���
						pChildNode = pTransNode->CreateChild();
						pChildNode->SetValue(strTransMethod);
						
						// ��������
						if(bRemoveSpace)
						{
							pChildNode = pTransNode->CreateChild();
							pChildNode->SetValue(_T("REMOVESPACE"));
						}

						// 1����Ʈ�� 2����Ʈ�� ��ȯ
						if(bTwoByte)
						{
							pChildNode = pTransNode->CreateChild();
							pChildNode->SetValue(_T("TWOBYTE"));
						}

					}

				}
				
			} // HOOK ��� ��
		}	
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// �ɼ��� ���� ���α׷��� ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::AdjustOption(COptionNode* pRootNode)
{
	if(NULL == pRootNode) return FALSE;
	
	ResetOption();
	MigrateOption(pRootNode);

	//FORCEFONT,HOOK(0x00434343,TRANS([ESP+0x4],ANSI,ALLSAMETEXT))
	BOOL bRetVal = TRUE;

	int cnt = pRootNode->GetChildCount();
	for(int i=0; i<cnt; i++)
	{
		COptionNode* pNode = pRootNode->GetChild(i);
		CString strValue = pNode->GetValue().MakeUpper();
		
		// HOOK ���
		if(strValue == _T("HOOK"))
		{
			BOOL bHookRes = HookFromOptionNode(pNode);
			if(FALSE == bHookRes) m_listRetryHook.push_back(pNode);
		} // HOOK ��� ��
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// ���� AdjustOption ���� HOOK ��带 ������� �� �Լ��� ȣ���ؼ� �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::HookFromOptionNode(COptionNode* pNode)
{
	BOOL bRetVal = FALSE;
	
	try
	{
		// ��ŷ�� �ּ�
		COptionNode* pAddrNode = pNode->GetChild(0);
		if(pAddrNode==NULL) throw -1;

		CHookPoint* pHookPoint = CHookPoint::CreateInstance(pAddrNode->GetValue());
		if(pHookPoint==NULL)
		{
			//MessageBox(m_hContainerWnd, _T("������ �ּҸ� ��ŷ�ϴµ� �����߽��ϴ� : ") + pAddrNode->GetValue(), _T("Hook error"), MB_OK);
			//continue;
			 throw -2;
		}
		m_listHookPoint.push_back(pHookPoint);

		// �� �ּҿ� ���� ��ŷ ��ɵ� ����
		int cnt2 = pNode->GetChildCount();
		for(int j=1; j<cnt2; j++)
		{
			COptionNode* pNode2 = pNode->GetChild(j);
			CString strHookValue = pNode2->GetValue();

			// ���� ���
			if(strHookValue.CompareNoCase(_T("TRANS"))==0)
			{
				// ���� �Ÿ�
				COptionNode* pDistNode = pNode2->GetChild(0);
				if(pDistNode==NULL) continue;

				/*
				int nDistFromESP = 0;
				CString strStorage = pDistNode->GetValue().MakeUpper();

				if(strStorage==_T("[ESP]")) nDistFromESP = 0x0;
				else if(strStorage==_T("EAX")) nDistFromESP = -0x4;
				else if(strStorage==_T("ECX")) nDistFromESP = -0x8;
				else if(strStorage==_T("EDX")) nDistFromESP = -0xC;
				else if(strStorage==_T("EBX")) nDistFromESP = -0x10;
				else if(strStorage==_T("ESP")) nDistFromESP = -0x14;
				else if(strStorage==_T("EBP")) nDistFromESP = -0x18;
				else if(strStorage==_T("ESI")) nDistFromESP = -0x1C;
				else if(strStorage==_T("EDI")) nDistFromESP = -0x20;
				else
				{

					_stscanf((LPCTSTR)strStorage, _T("[ESP+%x]"), &nDistFromESP);
					if(nDistFromESP == 0) continue;
				}

				CTransCommand* pTransCmd = pHookPoint->AddTransCmd(nDistFromESP);
				*/

				CTransCommand* pTransCmd = pHookPoint->AddTransCmd(pDistNode->GetValue());
				
				// ���� �ɼǵ� ����
				int cnt3 = pNode2->GetChildCount();
				for(int k=1; k<cnt3; k++)
				{
					COptionNode* pNode3 = pNode2->GetChild(k);
					CString strTransOption = pNode3->GetValue().MakeUpper();

					// �������
					if(strTransOption == _T("NOP"))
					{
						pTransCmd->SetTransMethod(0);
					}
					else if(strTransOption == _T("PTRCHEAT"))
					{
						pTransCmd->SetTransMethod(1);
					}
					else if(strTransOption == _T("OVERWRITE"))
					{
						pTransCmd->SetTransMethod(2);
						if(pNode3->GetChild(_T("IGNORE")) != NULL)
						{
							pTransCmd->SetIgnoreBufLen(TRUE);
						}
					}
					else if(strTransOption == _T("SOW"))
					{
						pTransCmd->SetTransMethod(3);
					}
					
					// ��Ƽ����Ʈ / �����ڵ� ����
					else if(strTransOption == _T("ANSI"))
					{
						pTransCmd->SetUnicode(FALSE);
					}
					else if(strTransOption == _T("UNICODE"))
					{
						pTransCmd->SetUnicode(TRUE);
					}

					// ��� ��ġ�ϴ� �ؽ�Ʈ ����
					else if(strTransOption == _T("ALLSAMETEXT"))
					{
						pTransCmd->SetAllSameText(TRUE);
					}

					// CLIPKOR �ɼ�
					else if(strTransOption == _T("CLIPKOR"))
					{
						pTransCmd->SetClipKor(TRUE);
					}

					// CLIPJPN �ɼ�
					else if(strTransOption == _T("CLIPJPN"))
					{
						pTransCmd->SetClipJpn(TRUE);
					}

				}
			}

		}	// end of for ( �� �ּҿ� ���� ��ŷ ��ɵ� ���� )

		bRetVal = TRUE;
	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode; 
	}


	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// Ŭ�����忡 �ؽ�Ʈ ���� �۾��� �ϴ� ������
//
//////////////////////////////////////////////////////////////////////////
UINT __stdcall CATCodeMgr::ClipboardThreadFunc(LPVOID pParam)
{
	while(_Inst && _Inst->m_bRunClipboardThread)
	{
		DWORD dwRes = WaitForSingleObject(_Inst->m_hClipTextChangeEvent, 300);

		// ��ٷ��� ���� ���� ��
		if(WAIT_TIMEOUT == dwRes)
		{
			EnterCriticalSection(&_Inst->m_csClipText);

			// Ŭ������� ������ �� �����Ͱ� �ִٸ�
			if(_Inst->m_strClipText.IsEmpty() == FALSE)
			{
				HGLOBAL hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (_Inst->m_strClipText.GetLength() + 1) * sizeof(TCHAR));

				LPTSTR pGlobal = (LPTSTR)GlobalLock(hGlobal);
				
				if(pGlobal)
				{
					_tcscpy(pGlobal, (LPCTSTR)_Inst->m_strClipText);
					GlobalUnlock(hGlobal);

					OpenClipboard(NULL);
					EmptyClipboard();

#ifdef UNICODE
					SetClipboardData(CF_UNICODETEXT, hGlobal);
#else
					SetClipboardData(CF_TEXT, hGlobal);
#endif
					
					CloseClipboard();

					//GlobalFree(hGlobal);
				}

				_Inst->m_strClipText.Empty();
			}

			LeaveCriticalSection(&_Inst->m_csClipText);
		}
		// ��ٸ��� �� �ؽ�Ʈ�� �߰� �Ǿ��� ��
		else if(WAIT_OBJECT_0 == dwRes)
		{
			
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Ŭ�����忡 �ؽ�Ʈ ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::SetClipboardText(LPCTSTR cszText)
{
	BOOL bRetVal = FALSE;

	EnterCriticalSection(&m_csClipText);

	m_strClipText += cszText;

	LeaveCriticalSection(&m_csClipText);

	::SetEvent(m_hClipTextChangeEvent);

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// ���ο� �ε� ���̺귯�� �Լ� (A)
// AT�ڵ尡 DLL ������ ���, �� DLL�� ���� �ε�� �� �𸣹Ƿ�
// �̰����� �����ϰ� �ִٰ� �ε�Ǵ� ���� ��ŷ�Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
HMODULE __stdcall CATCodeMgr::NewLoadLibraryA(LPCSTR lpFileName)
{
	wchar_t wszTmp[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, wszTmp, MAX_PATH);
	TRACE(_T("[aral1] NewLoadLibraryA('%s') \n"), wszTmp);

	HMODULE hModule = NULL;

	if(CATCodeMgr::_Inst)
	{
		hModule = LoadLibraryA(lpFileName);

		// �����ߴ� ��ŷ���� ��õ�
		for(list<COptionNode*>::iterator iter = _Inst->m_listRetryHook.begin();
			iter != _Inst->m_listRetryHook.end();)
		{
			BOOL bHookRes = _Inst->HookFromOptionNode( (*iter) );
			
			if(bHookRes) iter = _Inst->m_listRetryHook.erase(iter);
			else iter++;
		}

	}

	return hModule;
}


//////////////////////////////////////////////////////////////////////////
//
// ���ο� �ε� ���̺귯�� �Լ� (W)
// AT�ڵ尡 DLL ������ ���, �� DLL�� ���� �ε�� �� �𸣹Ƿ�
// �̰����� �����ϰ� �ִٰ� �ε�Ǵ� ���� ��ŷ�Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
HMODULE __stdcall CATCodeMgr::NewLoadLibraryW(LPCWSTR lpFileName)
{
	TRACE(_T("[aral1] NewLoadLibraryW('%s') \n"), lpFileName);

	HMODULE hModule = NULL;

	if(CATCodeMgr::_Inst)
	{
		hModule = LoadLibraryW(lpFileName);

		// �����ߴ� ��ŷ���� ��õ�
		for(list<COptionNode*>::iterator iter = _Inst->m_listRetryHook.begin();
			iter != _Inst->m_listRetryHook.end();)
		{
			BOOL bHookRes = _Inst->HookFromOptionNode( (*iter) );

			if(bHookRes) iter = _Inst->m_listRetryHook.erase(iter);
			else iter++;
		}

	}

	return hModule;
}


int CATCodeMgr::GetAllLoadedModules( PMODULEENTRY32 pRetBuf, int maxCnt )
{
	int curCnt = 0;

	// ��ȯ ���� �ʱ�ȭ
	ZeroMemory(pRetBuf, sizeof(PMODULEENTRY32)*maxCnt);
	
	// ���μ��� ������ �ڵ��� ����
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if(INVALID_HANDLE_VALUE == hModuleSnap) return 0;

	pRetBuf[curCnt].dwSize = sizeof(MODULEENTRY32);
	BOOL bExist = Module32First(hModuleSnap, &pRetBuf[curCnt]);

	while( bExist == TRUE && curCnt < maxCnt )
	{
		curCnt++;
		pRetBuf[curCnt].dwSize = sizeof(MODULEENTRY32);
		bExist = Module32Next(hModuleSnap, &pRetBuf[curCnt]);
	}

	CloseHandle (hModuleSnap);

	return curCnt;
}


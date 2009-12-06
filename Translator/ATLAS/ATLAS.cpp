// ATLAS.cpp : �ش� DLL�� �ʱ�ȭ ��ƾ�� �����մϴ�.
//
#pragma warning(disable:4996)

#include "stdafx.h"
#include "ATLAS.h"
#include "hash.hpp"
#include <stdio.h>
#include <process.h>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TRANS_BUF_SIZE 4096


BEGIN_MESSAGE_MAP(CATLASApp, CWinApp)
END_MESSAGE_MAP()

HWND	g_hSettingWnd = NULL;
LPWSTR	g_pOptStrBuf = NULL;

// ������ CATLASApp ��ü�Դϴ�.
CATLASApp theApp;


// ���� ������
#pragma code_seg()

#pragma data_seg(".ATLASG")
HHOOK sg_hATLASHook = NULL;
#pragma data_seg()
#pragma comment(linker, " /SECTION:.ATLASG,RWS") 



//////////////////////////////////////////////////////////////////////////
//
// Export Functions
//
//////////////////////////////////////////////////////////////////////////

// GetPluginInfo
extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo)
{
	BOOL bRetVal = FALSE;

	if(sizeof(PLUGIN_INFO) <= pPluginInfo->cch)
	{
		// Set Plugin Icon ID
		pPluginInfo->nIconID = IDI_ICON1;
		// Set Plugin Name
		wcscpy_s(pPluginInfo->wszPluginName, 64, L"ATLAS v13");
		// Set Plugin Type (Algorithm, Filter, Translator)
		wcscpy_s(pPluginInfo->wszPluginType, 16, L"Translator");
		// Set Download URL
		wcscpy_s(pPluginInfo->wszDownloadUrl, 256, L"http://www.aralgood.com/update_files_AT3/Plugin/Translator/ATLAS.zip");

		bRetVal = TRUE;
	}

	return bRetVal;
}

// OnPluginInit
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption)
{
	return theApp.OnPluginInit(hAralWnd, wszPluginOption);
}

// OnPluginClose
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose()
{
	return theApp.OnPluginClose();
}

// OnPluginOption
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption()
{
	return theApp.OnPluginOption();
}

// OnObjectInit
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectInit(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.OnObjectInit(pTransObj);
}

// OnObjectClose
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectClose(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.OnObjectClose(pTransObj);	
}

// OnObjectMove
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectMove(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.OnObjectMove(pTransObj);
}

// OnObjectOption
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectOption(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.OnObjectOption(pTransObj);
}

// Main Translation Function
BOOL __stdcall Translate(TRANSLATION_OBJECT* pTransObject)
{
	return theApp.TranslateJ2E(pTransObject);
}



//---------------------------------------------------------------------------
// WH_GETMESSAGE�� ��ŷ�� ���ν����� ����
//---------------------------------------------------------------------------
LRESULT CALLBACK GetMsgProc(
							int code,       // hook code
							WPARAM wParam,  // removal option
							LPARAM lParam   // message
							)
{
	// �׳� �ؽ�Ʈ �Ÿ� �ҷ��ش�
	return ::CallNextHookEx(sg_hATLASHook, code, wParam, lParam);
}





// CATLASApp ������
CATLASApp::CATLASApp()
: m_bRemoveTrace(FALSE)
, m_hATLCLIPProcess(NULL)
, m_hRequestEvent(NULL)
, m_hResponseEvent(NULL)
, m_hATLASHook(NULL)
{
	m_strProcessName = _T("");
	m_strHomeDir = _T("");
	m_strErrorMsg = _T("");
	m_CacheHead.pNextLink = &m_CacheTail;
	m_CacheTail.pPrevLink = &m_CacheHead;
	m_mapCache.clear();
}


// CATLASApp �ʱ�ȭ
BOOL CATLASApp::InitInstance()
{

	::DisableThreadLibraryCalls( this->m_hInstance );

	// Get current process name
	TCHAR szModuleName[MAX_PATH] = {0,};
	::GetModuleFileName(NULL, szModuleName, MAX_PATH);
	TCHAR* pNameStart = _tcsrchr(szModuleName, _T('\\') );
	
	if(pNameStart) m_strProcessName = pNameStart+1;
	else m_strProcessName = szModuleName;

	// This process name is 'ATLCLIP.exe', then enable subclass of the ATLCLIP window
	if(m_strProcessName.CompareNoCase(_T("ATLCLIP.EXE")) == 0)
	{
		//MessageBox(NULL, _T("I'm going"), _T("I'm going"), MB_OK);
		m_ATALSMgr.InitATLASMgr();
	}

	CWinApp::InitInstance();

	return TRUE;
}

// When destroy Instance
int CATLASApp::ExitInstance()
{
	// This process name is 'ATLCLIP.exe', then disable subclassing of the ATLCLIP window
	if(m_strProcessName.CompareNoCase(_T("ATLCLIP.EXE")) == 0)
	{
		m_ATALSMgr.CloseATLASMgr();

		// ATLCLIP.exe program exit.
		TerminateProcess(GetCurrentProcess(), 0);
		/*
		CWnd* pMainWnd = ::AfxGetMainWnd();
		if(pMainWnd && ::IsWindow(pMainWnd->m_hWnd))
		{
			::PostMessage(pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
		}
		*/
	}
	else
	{
		CloseATLAS();
	}

	return CWinApp::ExitInstance();
}


BOOL CATLASApp::OnPluginInit( HWND hAralWnd, LPWSTR wszPluginOption )
{
	TRACE(_T("CATLASApp::OnPluginInit() begin \n"));

	BOOL bRetVal = FALSE;

	// �����̳� ������ ����
	g_hSettingWnd = hAralWnd;

	// �ɼ� ��Ʈ�� ���� ����
	g_pOptStrBuf = wszPluginOption;

	// �ɼ� ����
	if(g_pOptStrBuf[0] == '0') m_bRemoveTrace = FALSE;
	else m_bRemoveTrace = TRUE;

	//if(g_pOptStrBuf[1] == '0') m_bRemoveDupSpace = FALSE;
	//else m_bRemoveDupSpace = TRUE;

	// Initialize ATLAS
	if(InitATLAS() == TRUE)
	{
		bRetVal = TRUE;
	}
	else
	{
		MessageBox(NULL, m_strErrorMsg, _T("ezTransXP Initialize Error"), MB_OK | MB_TOPMOST);		
	}


	TRACE(_T("CATLASApp::OnPluginInit() end \n"));
	return bRetVal;
}


BOOL CATLASApp::OnPluginClose()
{
	CloseATLAS();

	return TRUE;
}

// Check all charactors is ASCII.
BOOL IsASCIIOnly(LPCTSTR cszString)
{
	if(NULL == cszString) return FALSE;

	size_t i = 0;
	while(cszString[i]) 
	{
		if( ((BYTE*)cszString)[i++] >= 0x80 ) return FALSE;
	}

	return TRUE;
}


BOOL CATLASApp::InitATLAS()
{
	BOOL bRetVal = FALSE;

	try
	{
		// ����ȭ ��ü �ʱ�ȭ
		InitializeCriticalSection(&m_csTrans);

		// Find ATLAS directory
		while(m_strHomeDir.IsEmpty())
		{
			m_strHomeDir = GetATLASHomeDir();
			if( m_strHomeDir.IsEmpty() )
			{
				if( ::MessageBox(
					NULL, 
					_T("Can not find the ATLAS directory.\r\nDo you want to browse, now?"), 
					_T("ATLAS Plugin"), 
					MB_YESNO | MB_TOPMOST) == IDYES )
				{
					ITEMIDLIST *pidlBrowse; 
					TCHAR pszPathname[MAX_PATH] = {0,}; 
					BROWSEINFO BrInfo;
					BrInfo.hwndOwner = NULL; 
					BrInfo.pidlRoot = NULL;

					memset(&BrInfo, 0, sizeof(BrInfo));
					BrInfo.pszDisplayName = pszPathname;
					BrInfo.lpszTitle = _T("ATLAS directory");
					BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;

					pidlBrowse = ::SHBrowseForFolder(&BrInfo);

					if( pidlBrowse != NULL)
					{
						BOOL bModalRes = ::SHGetPathFromIDList(pidlBrowse, pszPathname);
						if(bModalRes)
						{
							m_strHomeDir = pszPathname;
						}
					}

				}
				else
				{
					throw _T("Can not find the ATLAS directory.");
				}
			}
		}

		// ���� ������� �˻�
		/*
		if(IsASCIIOnly(m_strHomeDir) == FALSE)
		{
			if( ::MessageBox(
				NULL, 
				_T("ATLAS directory includes non-ASCII charactors.\r\nDo you want to continue?"), 
				_T("ATLAS Plugin"), 
				MB_YESNO) == IDNO )
			{
				throw _T("Please, re-install ATLAS to english directory and try again.");
			}			
		}
		*/

		// Execute ATLCLIP.exe
		CString strATLCLIP = m_strHomeDir + _T("ATLCLIP.exe");
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		si.cb = sizeof(STARTUPINFO);

		BOOL bExecRes = CreateProcess( 
			NULL, (LPTSTR)(LPCTSTR)strATLCLIP, 
			NULL, NULL, FALSE, 0, NULL, 
			(LPTSTR)(LPCTSTR)m_strHomeDir, &si, &pi );

		if(FALSE == bExecRes || 0 == pi.dwThreadId) throw _T("Failed to execute ATLCLIP.exe");
		m_hATLCLIPProcess = pi.hProcess;

		// Wait for ATLCLIP is Idle Status.
		if( WaitForInputIdle(pi.hProcess, 5000) != 0 ) throw _T("ATLCLIP.exe timeout");
		
		// DLL Injection.
		sg_hATLASHook = ::SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)(GetMsgProc), theApp.m_hInstance, pi.dwThreadId );
		if(NULL == sg_hATLASHook) throw _T("Failed to hook ATLCLIP.exe");
		::PostThreadMessage(pi.dwThreadId, WM_NULL, 0, 0);
		
		// ����ȭ ��ü�� �����Ǿ����� ��ٸ�
		for(int r=0; 
			(r<10000) && (NULL==m_hTransReadyEvent || NULL==m_hMapFile || NULL==m_hRequestEvent || NULL==m_hResponseEvent ); 
			r++)
		{
			Sleep(500);

			if(NULL == m_hMapFile)
				m_hMapFile = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("ATBufferATLCLIP"));

			if(NULL == m_hTransReadyEvent)
				m_hTransReadyEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("ATReadyATLCLIP"));

			if(NULL == m_hRequestEvent) 
				m_hRequestEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("ATRequestATLCLIP"));

			if(NULL == m_hResponseEvent) 
				m_hResponseEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("ATResponseTLCLIP"));

		}
		if(NULL==m_hMapFile 
			|| NULL==m_hTransReadyEvent 
			|| NULL==m_hRequestEvent 
			|| NULL==m_hResponseEvent) throw _T("Failed to open event objects.");

		bRetVal = TRUE;
	}
	catch (LPCTSTR strErr)
	{
		m_strErrorMsg = strErr;
		CloseATLAS();
		//MessageBox(hSettingWnd, strErr, _T("ezTransXP Initialize Error"), MB_OK);
	}

	return bRetVal;
}

void CATLASApp::CloseATLAS()
{
	if(NULL != g_hSettingWnd)
	{

		// ����ȭ ��ü ����ȭ
		if(m_hMapFile)
		{
			::CloseHandle(m_hMapFile);
			m_hMapFile = NULL;
		}	
		if(m_hTransReadyEvent)
		{
			::CloseHandle(m_hTransReadyEvent);
			m_hTransReadyEvent = NULL;
		}	
		if(m_hRequestEvent)
		{
			::CloseHandle(m_hRequestEvent);
			m_hRequestEvent = NULL;
		}	
		if(m_hResponseEvent)
		{
			::CloseHandle(m_hResponseEvent);
			m_hResponseEvent = NULL;
		}

		// �����ؽ�Ʈ ĳ��
		m_CacheHead.pNextLink = &m_CacheTail;
		m_CacheTail.pPrevLink = &m_CacheHead;

		// ��Ÿ ����
		m_strErrorMsg = _T("");
		m_strHomeDir = _T("");

		// ���� ���ҵ� ����
		TRACE(_T("[aral1] Map : %d"), m_mapCache.size());
		for(map<UINT, CTextElement*>::iterator iter = m_mapCache.begin();
			iter != m_mapCache.end();
			iter++)
		{
			CTextElement* pTextElem = iter->second;
			if(pTextElem) delete pTextElem;
		}
		m_mapCache.clear();

		// DLL ����� ����
		if(sg_hATLASHook)
		{
			::UnhookWindowsHookEx(sg_hATLASHook);
			sg_hATLASHook = NULL;
		}

		// ����ȭ ��ü ����ȭ
		DeleteCriticalSection(&m_csTrans);

		g_hSettingWnd = NULL;
		g_pOptStrBuf = NULL;
	
	}
}


BOOL CATLASApp::OnPluginOption()
{
	/*
	if(g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		CEZTransOptionDlg opt_dlg;
		opt_dlg.m_bRemoveTrace = m_bRemoveTrace;

		if(opt_dlg.DoModal() == IDOK)
		{
			// �ɼ� ����
			m_bRemoveTrace = opt_dlg.m_bRemoveTrace;

			g_pOptStrBuf[0] = '0' + m_bRemoveTrace;
			g_pOptStrBuf[1] = '\0';

			// �ɼ� ���� ����?
			if(m_bRemoveTrace == TRUE) g_pOptStrBuf[0] = '\0';

		}
	}
	*/
	return TRUE;
}


BOOL CATLASApp::OnObjectInit(TRANSLATION_OBJECT* pTransObj)
{
	if(NULL == pTransObj) return FALSE;

	// Set the translation function pointer
	pTransObj->procTranslate = Translate;

	return TRUE;
}

BOOL CATLASApp::OnObjectClose(TRANSLATION_OBJECT* pTransObj)
{

	if(NULL == pTransObj) return FALSE;

	// Delete Pre-translation buffer
	if(pTransObj->pPreTransBuf)
	{
		delete [] (char*)pTransObj->pPreTransBuf;
		pTransObj->pPreTransBuf = NULL;
	}
	pTransObj->nPreTransBufLen = 0;

	// Delete Post-translation buffer
	if(pTransObj->pPostTransBuf)
	{
		delete [] (char*)pTransObj->pPostTransBuf;
		pTransObj->pPostTransBuf = NULL;
	}
	pTransObj->nPostTransBufLen = 0;

	// Reset the translation function pointer
	pTransObj->procTranslate = NULL;

	return TRUE;
}

BOOL CATLASApp::OnObjectMove(TRANSLATION_OBJECT* pTransObj)
{
	return TRUE;
}

BOOL CATLASApp::OnObjectOption(TRANSLATION_OBJECT* pTransObj)
{

	/*
	if(m_hAralWnd && IsWindow(m_hAralWnd))
	{
		CEZTransOptionDlg opt_dlg;
		opt_dlg.m_bRemoveTrace		= (pTransObj->wszObjectOption[0] && pTransObj->wszObjectOption[0] == L'1') ? TRUE : FALSE;
		opt_dlg.m_bRemoveDupSpace	= (pTransObj->wszObjectOption[0] && pTransObj->wszObjectOption[1] == L'1') ? TRUE : FALSE;

		if(opt_dlg.DoModal() == IDOK)
		{
			// �ɼ� ����
			pTransObj->wszObjectOption[0] = (opt_dlg.m_bRemoveTrace    ? L'1' : L'0');
			pTransObj->wszObjectOption[1] = (opt_dlg.m_bRemoveDupSpace ? L'1' : L'0');
			pTransObj->wszObjectOption[2] = L'\0';

			// �ɼ� ���� ����?
			if(opt_dlg.m_bRemoveTrace == TRUE && opt_dlg.m_bRemoveDupSpace == FALSE) 
				pTransObj->wszObjectOption[0] = L'\0';

		}
	}
	*/

	return TRUE;
}





BOOL CATLASApp::TranslateJ2E(TRANSLATION_OBJECT* pTransObject)
{
	BOOL bRetVal = FALSE;

	// Check translation object is NULL
	if(NULL == pTransObject) return FALSE;

	TRANSLATION_OBJECT* pPrevObject = pTransObject->pPrevObject;
	TRANSLATION_OBJECT* pNextObject = pTransObject->pNextObject;

	// Check the previous object is NULL
	if(NULL == pPrevObject) return FALSE;
	// Check pre-translation buffer of the previous object is NULL
	if(NULL == pPrevObject->pPreTransBuf || 0 == pPrevObject->nPreTransBufLen) return FALSE;

	// Create text buffer as double size of pre-translation buffer
	if(pTransObject->pPreTransBuf)
	{
		delete [] (char*)pTransObject->pPreTransBuf;
		pTransObject->pPreTransBuf = NULL;
		pTransObject->nPreTransBufLen = 0;
	}

	pTransObject->nPreTransBufLen = strlen((LPCSTR)pPrevObject->pPreTransBuf) * 2 + 1024;
	pTransObject->pPreTransBuf = new char[pTransObject->nPreTransBufLen];

	//////////////////////////////////////////////////////////////////////////
	//
	// Pre-translation
	//
	ZeroMemory(pTransObject->pPreTransBuf, pTransObject->nPreTransBufLen);

	EnterCriticalSection(&m_csTrans);

	if( ((LPCSTR)pPrevObject->pPreTransBuf)[0] && pPrevObject->nPreTransBufLen > 0)
	{
		// �� ���� �ؽ�Ʈ�� �ؽ� ���ϱ�
		UINT dwTextHash = MakeStringHash((LPCSTR)pPrevObject->pPreTransBuf);

		// ĳ�ÿ��� ã�ƺ���
		map<UINT, CTextElement*>::iterator iter = m_mapCache.find(dwTextHash);
		CTextElement* pTextElem = NULL;

		// ĳ�ÿ� ������
		if(m_mapCache.end() != iter)
		{
			pTextElem = iter->second;

			// ����Ʈ���� ����������
			pTextElem->pPrevLink->pNextLink = pTextElem->pNextLink;
			pTextElem->pNextLink->pPrevLink = pTextElem->pPrevLink;

		}
		// ĳ�ÿ� ������ ����Ʈ������ ����
		else
		{
			LPCSTR cszJpnText = (LPCSTR)pPrevObject->pPreTransBuf;
			char pBuf1[TRANS_BUF_SIZE] = {0,};
			char pBuf2[TRANS_BUF_SIZE] = {0,};

			// ��ȣ ���� ���ڵ�
			if(m_bRemoveTrace)
			{
				EncodeTrace(cszJpnText, pBuf1);
				cszJpnText = pBuf1;
			}

			// Translate!
			if(TranslateUsingATLAS(cszJpnText, (LPSTR)pTransObject->pPreTransBuf, pTransObject->nPreTransBufLen))
			{
				LPCSTR cszEngText = (LPCSTR)pTransObject->pPreTransBuf;

				// ��ȣ ���� ���ڵ�
				if(m_bRemoveTrace)
				{
					FilterTrace(cszEngText, pBuf2);
					DecodeTrace(pBuf2, pBuf1);
					cszEngText = pBuf1;
				}

				pTextElem = new CTextElement();
				pTextElem->dwHash = dwTextHash;
				pTextElem->strTranslatedText = cszEngText;

				// ĳ�ð� �� á�ٸ� ���ڸ� Ȯ���� ����
				if(m_mapCache.size() >= 10000)
				{
					CTextElement* pDelElem = m_CacheTail.pPrevLink;
					m_CacheTail.pPrevLink = pDelElem->pPrevLink;

					m_mapCache.erase(pDelElem->dwHash);
					delete pDelElem;
				}

				// ĳ�ÿ� ����
				m_mapCache.insert(pair<UINT, CTextElement*>(dwTextHash, pTextElem));
			}

		}


		if(pTextElem)
		{
			int nLen = (int)pTextElem->strTranslatedText.length();

			if( nLen > (pTransObject->nPreTransBufLen-1) )
			{
				strncpy((LPSTR)pTransObject->pPreTransBuf, pTextElem->strTranslatedText.c_str(), pTransObject->nPreTransBufLen-1);
				((LPSTR)pTransObject->pPreTransBuf)[pTransObject->nPreTransBufLen-1] = '\0';
			}
			else
			{
				strcpy((LPSTR)pTransObject->pPreTransBuf, pTextElem->strTranslatedText.c_str());
			}

			// ��� �������� ����
			pTextElem->pPrevLink = &m_CacheHead;
			pTextElem->pNextLink = m_CacheHead.pNextLink;

			m_CacheHead.pNextLink->pPrevLink = pTextElem;
			m_CacheHead.pNextLink = pTextElem;

		}

	}// end of else of if('\0' == pThis->m_pJpnText[0])

	pTransObject->nPreTransBufLen = strlen((LPCSTR)pTransObject->pPreTransBuf) + 1;

	LeaveCriticalSection(&m_csTrans);
	//
	// End of Pre-translation
	//
	//////////////////////////////////////////////////////////////////////////


	// Check the next object is NULL
	if(NULL == pNextObject) return FALSE;
	// Check the next function is available
	if(NULL == pNextObject->procTranslate) return FALSE;

	// * Call next translation function
	BOOL bRes = pNextObject->procTranslate(pTransObject->pNextObject);
	if(FALSE == bRes) return FALSE;

	// Check post-translation buffer of the next object is NULL
	if(NULL == pNextObject->pPostTransBuf || 0 == pNextObject->nPostTransBufLen) return FALSE;

	// Create text buffer as double size of pre-translation buffer
	if(pTransObject->pPostTransBuf)
	{
		delete [] (char*)pTransObject->pPostTransBuf;
		pTransObject->pPostTransBuf = NULL;
		pTransObject->nPostTransBufLen = 0;
	}

	pTransObject->pPostTransBuf = new char[pNextObject->nPostTransBufLen];
	pTransObject->nPostTransBufLen = pNextObject->nPreTransBufLen;

	//////////////////////////////////////////////////////////////////////////
	//
	// Post-translation
	//
	ZeroMemory(pTransObject->pPostTransBuf, pTransObject->nPostTransBufLen);

	memcpy_s(pTransObject->pPostTransBuf, 
		pTransObject->nPostTransBufLen, 
		pNextObject->pPostTransBuf,
		pNextObject->nPostTransBufLen);
	//
	// End of Post-translation
	//
	//////////////////////////////////////////////////////////////////////////


	return TRUE;

}


BOOL CATLASApp::TranslateUsingATLAS( LPCSTR cszJapanese, LPSTR szKorean, int nBufSize )
{
	BOOL bRetVal = FALSE;

	// Wait the Ready event
	if(::WaitForSingleObject(m_hTransReadyEvent, INFINITE) == WAIT_OBJECT_0)
	{


		// Open shared memory
		ATLAS_TRANS_BUFFER* pTransBuf = 
			(ATLAS_TRANS_BUFFER*) MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ATLAS_TRANS_BUFFER));

		if(pTransBuf)
		{
			// Write original text into shared memory.
			pTransBuf->bResult = FALSE;
			strcpy(pTransBuf->szOriginalText, cszJapanese);
			ZeroMemory(pTransBuf->szTranslatedText, TRANS_BUF_SIZE);
			UnmapViewOfFile(pTransBuf);		

			// Set the request event
			::SetEvent(m_hRequestEvent);

			// Wait the response event
			if(::WaitForSingleObject(m_hResponseEvent, INFINITE) == WAIT_OBJECT_0)
			{
				// Open shared memory
				pTransBuf = (ATLAS_TRANS_BUFFER*) MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ATLAS_TRANS_BUFFER));

				if(pTransBuf)
				{
					strcpy(szKorean, pTransBuf->szTranslatedText);
					bRetVal = pTransBuf->bResult;
					UnmapViewOfFile(pTransBuf);		
				}
			}

		} // end of if(pTransBuf)
	}

	return bRetVal;
}


// Get ATLAS directory
CString CATLASApp::GetATLASHomeDir()
{
	CString strRetVal = _T("");

	TCHAR szHomeDir[MAX_PATH];
	HKEY hKey;
	DWORD dwLen = MAX_PATH;

	try
	{
		
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Fujitsu\\ATLAS\\V13.0\\SYSTEM INFO"), 0,
			KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) throw -1;


		if (RegQueryValueEx(hKey, _T("SYSTEM DIR"), NULL, NULL, (LPBYTE)szHomeDir,
			&dwLen) != ERROR_SUCCESS) throw -2;

		RegCloseKey(hKey);

		if ( _tcslen(szHomeDir) == 0 ) throw -3;

		strRetVal = szHomeDir;

	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
	}

	return strRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// �����Ⱑ ��ȣ�� ������� �ٿ������� ��� �Ϻ� ���ӿ��� ���� ����
// ���� �̷� ��� ����
// ex) "��(��)��⸦ �Ծ���." -> "�Ұ�⸦ �Ծ���."
//
//////////////////////////////////////////////////////////////////////////
void CATLASApp::FilterTrace(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar) return;

	size_t len = strlen(cszKorSrc);	
	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		// 2����Ʈ ���ڸ�
		if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0')
		{
			if((BYTE)cszKorSrc[i] == 0xA1 && (BYTE)cszKorSrc[i+1] == 0xA1)
			{
				i += 2;
				szKorTar[j++] = ' ';
			}
			else
			{
				szKorTar[j++] = cszKorSrc[i++];
				szKorTar[j++] = cszKorSrc[i++];
			}
		}
		// 1����Ʈ ���ڸ�
		else
		{
			if(cszKorSrc[i] == '(')
			{
				while(cszKorSrc[i] != ')' && i<len)
				{
					if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0') i++;
					i++;
				}

				if(i<len)
				{
					i++;
					if(cszKorSrc[i]==' ') i++;
				}
			}
			else if(cszKorSrc[i] == '{')
			{
				while(cszKorSrc[i] != '}' && i<len)
				{
					if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0') i++;
					i++;
				}

				if(i<len)
				{
					i++;
					if(cszKorSrc[i]==' ') i++;
				}
			}
			else if(cszKorSrc[i] == '[')
			{
				// AT ���� �����̸�
				if( strncmp(&cszKorSrc[i], "[\\at", 4) == 0 )
				{
					do
					{
						szKorTar[j++] = cszKorSrc[i++];
					}while(cszKorSrc[i-1] != ']' && i<len);
				}
				// �ƴ϶� �׳� '(' ��ȣ �̸�
				else
				{
					while(cszKorSrc[i] != ']' && i<len)
					{
						if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0') i++;
						i++;
					}

					if(i<len)
					{
						i++;
						if(cszKorSrc[i]==' ') i++;
					}
				}
			}
			else if(cszKorSrc[i] == '<')
			{
				while(cszKorSrc[i] != '>' && i<len)
				{
					if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0') i++;
					i++;
				}

				if(i<len)
				{
					i++;
					if(cszKorSrc[i]==' ') i++;
					//if(j>0 && ':' == szKorText[j-1]) szKorText[j-1] = ' ';
				}
			}
			else if(cszKorSrc[i] == '\r' || cszKorSrc[i] == '\n')
			{
				i++;
			}
			else if(i+5 < len && strncmp(&cszKorSrc[i], "&nbsp;", 6) == 0)
			{
				i+= 6;
			}
			else
			{
				szKorTar[j++] = cszKorSrc[i++];
			}
		}
	}

	// Trim
	//do 
	//{
	//	szKorText[j] = '\0';
	//	j--;
	//} while (j>0 && ' ' == szKorText[j]);
	szKorTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<FilterTrace Start>");
	OutputDebugStringA(cszKorSrc);
	OutputDebugStringA(szKorTar);
	OutputDebugStringA("<FilterTrace End>");
#endif
}


//////////////////////////////////////////////////////////////////////////
//
// �ڵ� �����Ǵ� ��ȣ�� Ȯ���� �Ǻ��ϱ� ����
// ���� �ִ� ��ȣ�� ��ȣȭ
// ex) "[�檫]" -> "(\at5B)�檫(\at5D)"
//
//////////////////////////////////////////////////////////////////////////
void CATLASApp::EncodeTrace(LPCSTR cszJpnSrc, LPSTR szJpnTar)
{
	if(NULL==cszJpnSrc || NULL==szJpnTar) return;

	if('\0'==cszJpnSrc[0])
	{
		szJpnTar[0] = '\0';
		return;
	}

	size_t len = strlen(cszJpnSrc);

	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{

		// 2����Ʈ ������ ���
		if((BYTE)cszJpnSrc[i] >= 0x80 && (BYTE)cszJpnSrc[i+1] != '\0')
		{
			szJpnTar[j++] = cszJpnSrc[i++];
			szJpnTar[j++] = cszJpnSrc[i++];
		}
		// 1����Ʈ ������ ���
		else
		{
			// ���ڰ� ��ȣ��
			if(cszJpnSrc[i] == '(' || cszJpnSrc[i] == ')'
				|| cszJpnSrc[i] == '[' || cszJpnSrc[i] == ']'
				|| cszJpnSrc[i] == '{' || cszJpnSrc[i] == '}'
				|| cszJpnSrc[i] == '<' || cszJpnSrc[i] == '>'
				|| cszJpnSrc[i] == '\r' || cszJpnSrc[i] == '\n')
			{
				//j += sprintf(&szJpnTar[j], "_&%03u&_", (BYTE)cszJpnSrc[i++]);
				j += sprintf(&szJpnTar[j], "[\\at%.2X]", (BYTE)cszJpnSrc[i++]);
			}
			// �Ϲ� ���ڸ�
			else
			{
				szJpnTar[j++] = cszJpnSrc[i++];
			}
		}
	}

	szJpnTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<EncodeTrace Start>");
	OutputDebugStringA(cszJpnSrc);
	OutputDebugStringA(szJpnTar);
	OutputDebugStringA("<EncodeTrace End>");
#endif
}




//////////////////////////////////////////////////////////////////////////
//
// ����Ʈ���� ���� �� ��ȯ���״� ��ȣ���� ����
// ex) "(\at5B)�檫(\at5D)" -> "[�檫]"
//
//////////////////////////////////////////////////////////////////////////
void CATLASApp::DecodeTrace(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar) return;

	size_t len = strlen(cszKorSrc);
	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		size_t nCopyLen = len-i;
		//const char* pEncPtr = strstr(&cszKorSrc[i], "_&");
		const char* pEncPtr = strstr(&cszKorSrc[i], "[\\at");

		// "_&" �����Ƚ��� ã�Ҵٸ�
		if(NULL != pEncPtr)
		{
			// ��ȯ��ų �����Ͱ� Ȯ���Ѱ�?
			//if(	(UINT_PTR)pEncPtr - (UINT_PTR)cszKorSrc + 6 < len
			//	&& '&' == *(pEncPtr+5)
			//	&& '_' == *(pEncPtr+6) )
			//{
			//	nCopyLen = (UINT_PTR)pEncPtr - (UINT_PTR)(&cszKorSrc[i]);
			//}
			//// ��ȯ��Ű�� �ȵǴ� _&���
			//else
			//{
			//	pEncPtr = NULL;
			//	nCopyLen = 2;
			//}

			// ��ȯ��ų �����Ͱ� Ȯ���Ѱ�?
			if(	(UINT_PTR)pEncPtr - (UINT_PTR)cszKorSrc + 6 < len
				&& ']' == *(pEncPtr+6) )
			{
				nCopyLen = (UINT_PTR)pEncPtr - (UINT_PTR)(&cszKorSrc[i]);
			}
			// ��ȯ��Ű�� �ȵǴ� _&���
			else
			{
				pEncPtr = NULL;
				nCopyLen = 4;
			}
		}
		else
		{
			pEncPtr = NULL;
		}

		// �Ϲ� ���ڿ� ����
		memcpy(&szKorTar[j], &cszKorSrc[i], nCopyLen);
		i += nCopyLen;
		j += nCopyLen;

		if(pEncPtr)
		{
			// ��ȣ ���� ���ڵ�
			int val1;
			//sscanf(pEncPtr+2, "%03u", &val1);
			sscanf(pEncPtr+4, "%2x", &val1);
			((BYTE*)szKorTar)[j] = (BYTE)val1;

			i += 7;
			j += 1;
		}

	}

	szKorTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<DncodeTrace Start>");
	OutputDebugStringA(cszKorSrc);
	OutputDebugStringA(szKorTar);
	OutputDebugStringA("<DncodeTrace End>");
#endif
}

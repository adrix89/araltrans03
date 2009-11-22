// ezTransXP.cpp : Defines the initialization routines for the DLL.
//
#pragma warning(disable:4996)

#include "stdafx.h"
#include "ezTransXP.h"
#include "hash.hpp"
#include "EZTransOptionDlg.h"
#include <stdio.h>
#include <process.h>
#include <map>

#define TRANS_BUF_SIZE 4096

//   ���� 2Byte :    KS5601 ���� ��ȣ �ڵ�
// ������ 2Byte : SHIFT-JIS ���� ��ȣ �ڵ�
const char _SHIFT_JIS_CHAR_MAP[][4] = {
	{'\xA1', '\xCD', '\x81', '\x8F'},	// '\\'
	{'\xA3', '\xC0', '\x81', '\x97'},	// '@'
	{'\xA3', '\xA8', '\x81', '\x69'},	// '('
	{'\xA3', '\xA9', '\x81', '\x6A'},	// ')'
	{'\xA3', '\xBC', '\x81', '\x83'},	// '<'
	{'\xA3', '\xBE', '\x81', '\x84'},	// '>'
	{'\xA3', '\xDB', '\x81', '\x6D'},	// '['
	{'\xA3', '\xDD', '\x81', '\x6E'},	// ']'
	{'\xA3', '\xFB', '\x81', '\x6F'},	// '{'
	{'\xA3', '\xFD', '\x81', '\x70'},	// '}'
	{'\xA2', '\xC8', '\x81', '\x4A'},	// ��
	{'\xA2', '\xC8', '\x81', '\x4B'},	// ��
	{'\xA2', '\xC8', '\x81', '\x8D'},	// ��
	{'\xA2', '\xC7', '\x81', '\x4C'},	// ��
	{'\xA2', '\xC7', '\x81', '\x4D'},	// ��
	{'\xA2', '\xC7', '\x81', '\x8C'},	// ��
	{'\xA1', '\xAE', '\x81', '\x65'},	// ��
	{'\xA1', '\xAF', '\x81', '\x66'},	// ��
	{'\xA1', '\xB0', '\x81', '\x67'},	// ��
	{'\xA1', '\xB1', '\x81', '\x68'},	// ��

	{'\x00', '\x00', '\x00', '\x00'}
};


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only CezTransXPApp object

CezTransXPApp theApp;


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
		wcscpy_s(pPluginInfo->wszPluginName, 64, L"ezTrans XP");
		// Set Plugin Type (Algorithm, Filter, Translator)
		wcscpy_s(pPluginInfo->wszPluginType, 16, L"Translator");
		// Set Download URL
		wcscpy_s(pPluginInfo->wszDownloadUrl, 256, L"http://www.aralgood.com/update_files_AT3/Plugin/Translator/ezTransXP.zip");

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
	return theApp.TranslateJ2K(pTransObject);
}



//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// CezTransXPApp

BEGIN_MESSAGE_MAP(CezTransXPApp, CWinApp)
END_MESSAGE_MAP()


// CezTransXPApp construction

CezTransXPApp::CezTransXPApp()
: m_hTransThread(NULL)
, m_hRequestEvent(NULL)
, m_hResponseEvent(NULL)
, m_pCurTransObj(NULL)
{
	ZeroMemory(&m_sATCTNR3, sizeof(CONTAINER_PROC_ENTRY));
}




// CezTransXPApp initialization

BOOL CezTransXPApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

BOOL CezTransXPApp::OnPluginInit( HWND hAralWnd, LPWSTR wszPluginOption )
{
	TRACE(_T("CezTransXPApp::OnPluginInit() begin \n"));
	BOOL bRetVal = FALSE;

	// Initializing Code
	//::MessageBox(NULL, _T("CezTransXPApp"), _T("CezTransXPApp"), MB_OK | MB_TOPMOST);

	// �����̳� ������ ����
	m_hAralWnd = hAralWnd;
	// �ɼ� ��Ʈ�� ���� ����
	m_wszPluginOption = wszPluginOption;
	// �����̳� �Լ�
	HMODULE hATCTNR3 = GetModuleHandle(_T("ATCTNR3.DLL"));
	if(hATCTNR3 && INVALID_HANDLE_VALUE != hATCTNR3)
	{
		ZeroMemory(&m_sATCTNR3, sizeof(CONTAINER_PROC_ENTRY));
		m_sATCTNR3.procHookWin32Api			= (PROC_HookWin32Api) GetProcAddress(hATCTNR3, "HookWin32Api");
		m_sATCTNR3.procUnhookWin32Api		= (PROC_UnhookWin32Api) GetProcAddress(hATCTNR3, "UnhookWin32Api");
		m_sATCTNR3.procHookCodePoint		= (PROC_HookCodePoint) GetProcAddress(hATCTNR3, "HookCodePoint");
		m_sATCTNR3.procUnhookCodePoint		= (PROC_UnhookCodePoint) GetProcAddress(hATCTNR3, "UnhookCodePoint");
		m_sATCTNR3.procCreateTransCtx		= (PROC_CreateTransCtx) GetProcAddress(hATCTNR3, "CreateTransCtx");
		m_sATCTNR3.procDeleteTransCtx		= (PROC_DeleteTransCtx) GetProcAddress(hATCTNR3, "DeleteTransCtx");
		m_sATCTNR3.procTranslateUsingCtx	= (PROC_TranslateUsingCtx) GetProcAddress(hATCTNR3, "TranslateUsingCtx");
		m_sATCTNR3.procIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress(hATCTNR3, "IsAppLocaleLoaded");
		m_sATCTNR3.procSuspendAllThread		= (PROC_SuspendAllThread) GetProcAddress(hATCTNR3, "SuspendAllThread");
		m_sATCTNR3.procResumeAllThread		= (PROC_ResumeAllThread) GetProcAddress(hATCTNR3, "ResumeAllThread");
		m_sATCTNR3.procIsAllThreadSuspended = (PROC_IsAllThreadSuspended) GetProcAddress(hATCTNR3, "IsAllThreadSuspended");
	}

	// ����ȭ ��ü �ʱ�ȭ
	InitializeCriticalSection(&m_csTrans);
	m_hRequestEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hResponseEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	/*
	BOOL bInitRes = FALSE;
	for(int i=0; i<100; i++)
	{
		if(::WaitForSingleObject(m_hResponseEvent, 0) == WAIT_OBJECT_0)
		{
			bInitRes = TRUE;
			break;
		}

		::Sleep(50);
	}
	if(FALSE == bInitRes) m_strErrorMsg = _T("Initializing Time-out!");
	*/

	//::WaitForSingleObject(m_hResponseEvent, INFINITE);

	InitEZTrans();

	// Check ezTransXP was Initialized
	if(!m_strErrorMsg.IsEmpty())
	{
		MessageBox(NULL, m_strErrorMsg, _T("ezTransXP Initialize Error"), MB_OK | MB_TOPMOST);
	}
	else
	{
		// ����Ʈ���� �����带 ����
		UINT dwTransThreadID = 0;
		m_hTransThread = (HANDLE)_beginthreadex(NULL, 0, TransThreadFunc, this, 0, &dwTransThreadID);
		
		if(m_hTransThread)
		{
			bRetVal = TRUE;
		}
		else
		{
			MessageBox(NULL, _T("����Ʈ���� �����带 ������ �� �����ϴ�."), _T("ezTransXP Initialize Error"), MB_OK | MB_TOPMOST);
		}

	}

	TRACE(_T("CezTransXPApp::OnPluginInit() end \n"));
	return bRetVal;
}

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


void CezTransXPApp::InitEZTrans()
{
	m_strErrorMsg = _T("");
	m_strHomeDir = _T("");

	try
	{
		while(m_strHomeDir.IsEmpty())
		{
			m_strHomeDir = GetEZTransHomeDir();
			if( m_strHomeDir.IsEmpty() )
			{
				if( ::MessageBox(
					NULL, 
					_T("EZ Trans XP�� ��ġ�� ��θ� �� �����ϴ�.\r\n��θ� ���� ã���ðڽ��ϱ�?"), 
					_T("ezTransXP Plugin"), 
					MB_YESNO | MB_TOPMOST) == IDYES )
				{
					ITEMIDLIST *pidlBrowse; 
					TCHAR pszPathname[MAX_PATH] = {0,}; 
					BROWSEINFO BrInfo;
					BrInfo.hwndOwner = NULL; 
					BrInfo.pidlRoot = NULL;

					memset(&BrInfo, 0, sizeof(BrInfo));
					BrInfo.pszDisplayName = pszPathname;
					BrInfo.lpszTitle = _T("ez Trans-XP ��ġ ����");
					BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;

					pidlBrowse = ::SHBrowseForFolder(&BrInfo);

					if( pidlBrowse != NULL)
					{
						BOOL bModalRes = ::SHGetPathFromIDList(pidlBrowse, pszPathname);
						if(bModalRes)
						{
							m_strHomeDir.Format(_T("%s\\Dat"), pszPathname);
						}
					}

				}
				else
				{
					throw _T("EZ Trans XP �� ��ġ�� ��θ� ã�� �� �����ϴ�.");
				}
			}
		}

		// ���� ������� �˻�
		if(IsASCIIOnly(m_strHomeDir) == FALSE)
		{
			if( ::MessageBox(
				NULL, 
				_T("EZ Trans XP ��ο� �ѱ��� ���� ���� ��� ����� �������� ���� �� �ֽ��ϴ�.\r\n�׷��� ��� �����Ͻðڽ��ϱ�?"), 
				_T("ezTransXP Plugin"), 
				MB_YESNO) == IDNO )
			{
				throw _T("EZ Trans XP�� ���� ������ ��ġ�Ͻ� �� �ٽ� �õ��� �ֽʽÿ�.");				
			}			
		}

		// ����Ʈ���� �Լ���� ����ü �ʱ�ȭ
		ZeroMemory(&m_EZDLL, sizeof(m_EZDLL));

		// J2K ���� ��� �ε�
		CString strJ2KDllPath = m_strHomeDir + _T("\\..\\J2KEngine.dll");
		m_EZDLL.hJKMod = LoadLibrary(strJ2KDllPath);	
		if (m_EZDLL.hJKMod == NULL) throw _T("J2KEngine.dll�� �ε��� �� �����ϴ�.");

		// DLL�κ��� �Լ� ������ ���
		FARPROC pFuncAddr = NULL;

		pFuncAddr = GetProcAddress(m_EZDLL.hJKMod, "J2K_InitializeEx"); 
		memcpy( &m_EZDLL.J2K_InitializeEx, &pFuncAddr, sizeof(FARPROC) );

		pFuncAddr = GetProcAddress(m_EZDLL.hJKMod, "J2K_TranslateMMNT"); 
		memcpy( &m_EZDLL.J2K_TranslateMMNT, &pFuncAddr, sizeof(FARPROC) );

		pFuncAddr = GetProcAddress(m_EZDLL.hJKMod, "J2K_FreeMem"); 
		memcpy( &m_EZDLL.J2K_FreeMem, &pFuncAddr, sizeof(FARPROC) );

		pFuncAddr = GetProcAddress(m_EZDLL.hJKMod, "J2K_StopTranslation"); 
		memcpy( &m_EZDLL.J2K_StopTranslation, &pFuncAddr, sizeof(FARPROC) );

		pFuncAddr = GetProcAddress(m_EZDLL.hJKMod, "J2K_Terminate"); 
		memcpy( &m_EZDLL.J2K_Terminate, &pFuncAddr, sizeof(FARPROC) );


		if (m_EZDLL.J2K_InitializeEx == NULL || m_EZDLL.J2K_TranslateMMNT == NULL ||
			m_EZDLL.J2K_FreeMem == NULL || m_EZDLL.J2K_StopTranslation == NULL ||
			m_EZDLL.J2K_Terminate == NULL)  throw _T("�߸��� J2KEngine.dll �����Դϴ�.");

	}
	catch (LPCTSTR strErr)
	{
		m_strErrorMsg = strErr;
	}


	return;
}



void CezTransXPApp::CloseEZTrans()
{
	if (m_EZDLL.J2K_StopTranslation && m_EZDLL.J2K_Terminate)
	{
		m_EZDLL.J2K_StopTranslation(0);		/*    m_EZDLL.J2K_Terminate(); */
	}

	if (m_EZDLL.hJKMod)
	{
		/*    FreeLibrary(m_EZDLL.hJKMod); */
	}

	ZeroMemory(&m_EZDLL, sizeof(m_EZDLL));

	m_hAralWnd = NULL;

}

BOOL CezTransXPApp::OnPluginOption()
{
	return TRUE;
}


BOOL CezTransXPApp::OnPluginClose()
{
	if(m_hTransThread)
	{
		HANDLE hTmp = m_hTransThread;
		m_hTransThread = NULL;
		::SetEvent(m_hRequestEvent);
		::WaitForSingleObject(hTmp, 3000);
		::CloseHandle(m_hTransThread);
	}

	CloseEZTrans();

	// ����ȭ ��ü ����ȭ
	::CloseHandle(m_hRequestEvent);
	::CloseHandle(m_hResponseEvent);
	m_hRequestEvent = NULL;
	m_hResponseEvent = NULL;
	DeleteCriticalSection(&m_csTrans);

	return TRUE;
}

BOOL CezTransXPApp::OnObjectInit(TRANSLATION_OBJECT* pTransObj)
{
	if(NULL == pTransObj) return FALSE;

	/*
	// Create Pre-translation buffer
	pTransObj->pPreTransBuf = malloc(TRANS_BUF_LEN);
	pTransObj->nPreTransBufLen = TRANS_BUF_LEN;
	ZeroMemory(pTransObj->pPreTransBuf, pTransObj->nPreTransBufLen);

	// Create Post-translation buffer
	pTransObj->pPostTransBuf = malloc(TRANS_BUF_LEN);
	pTransObj->nPostTransBufLen = TRANS_BUF_LEN;
	ZeroMemory(pTransObj->pPostTransBuf, pTransObj->nPostTransBufLen);
	*/

	// Set the translation function pointer
	pTransObj->procTranslate = Translate;

	return TRUE;
}

BOOL CezTransXPApp::OnObjectClose(TRANSLATION_OBJECT* pTransObj)
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

BOOL CezTransXPApp::OnObjectMove(TRANSLATION_OBJECT* pTransObj)
{
	return TRUE;
}

BOOL CezTransXPApp::OnObjectOption(TRANSLATION_OBJECT* pTransObj)
{

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

	return TRUE;
}


BOOL CezTransXPApp::TranslateJ2K(TRANSLATION_OBJECT* pTransObject)
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

	pTransObject->nPreTransBufLen = strlen((LPCSTR)pPrevObject->pPreTransBuf) * 2 + 10;
	pTransObject->pPreTransBuf = new char[pTransObject->nPreTransBufLen];

	//////////////////////////////////////////////////////////////////////////
	//
	// Pre-translation
	//
	ZeroMemory(pTransObject->pPreTransBuf, pTransObject->nPreTransBufLen);
	
	EnterCriticalSection(&m_csTrans);

	m_pCurTransObj = pTransObject;

	ResetEvent(m_hResponseEvent);
	BOOL bSetEventResult = ::SetEvent(m_hRequestEvent);
	TRACE(_T("[aral1] Request Trans Event ON!! (bSetEventResult : %d) \n"), bSetEventResult);

	DWORD dwTransWait = ::WaitForSingleObject(m_hResponseEvent, INFINITE);
	TRACE(_T("[aral1] Received Response of Trans Event (dwTransWait : %d) \n"), dwTransWait);
	//if(dwTransWait == WAIT_OBJECT_0) bRetVal = TRUE;
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



// ����Ʈ���� Ȩ ���丮 �ʱ�ȭ
CString CezTransXPApp::GetEZTransHomeDir()
{
	CString strRetVal = _T("");

	TCHAR szHomeDir[MAX_PATH];
	HKEY hKey;
	DWORD dwLen = MAX_PATH;

	try
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\ChangShin\\ezTrans"), 0,
			KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) throw -1;


		if (RegQueryValueEx(hKey, _T("FilePath"), NULL, NULL, (LPBYTE)szHomeDir,
			&dwLen) != ERROR_SUCCESS) throw -2;

		RegCloseKey(hKey);

		if ( _tcslen(szHomeDir) == 0 ) throw -3;

		strRetVal.Format(_T("%s\\Dat"), szHomeDir);
	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
	}

	return strRetVal;
}



unsigned int __stdcall CezTransXPApp::TransThreadFunc(void* pParam)
{
	TRACE(_T("[aral1] ���� ������ ���� \n"));

	// �����ؽ�Ʈ ĳ��
	map<UINT, CTextElement*> mapCache;
	CTextElement CacheHead;
	CTextElement CacheTail;
	CacheHead.pNextLink = &CacheTail;
	CacheTail.pPrevLink = &CacheHead;

	// ������ ����
	char* pBuf1 = new char[TRANS_BUF_SIZE];
	char* pBuf2 = new char[TRANS_BUF_SIZE];	


	CezTransXPApp* pThis = (CezTransXPApp*)pParam;

	// ����Ʈ���� �ʱ�ȭ�Լ� ȣ��
#ifdef UNICODE
	char szHomeDir[MAX_PATH];
	::WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)pThis->m_strHomeDir, -1, szHomeDir, MAX_PATH, NULL, NULL);
	pThis->m_EZDLL.J2K_InitializeEx(EZTR_INIT_STR, szHomeDir);
#else
	m_EZDLL.J2K_InitializeEx(EZTR_INIT_STR, (LPCSTR)m_strHomeDir);
#endif
	//::SetEvent(pThis->m_hResponseEvent);	// ����Ʈ���� �ʱ�ȭ�� �Ǿ��ٰ� �˷��ִ� �뵵

	while(pThis->m_hTransThread)
	{
		DWORD dwTransWait = WaitForSingleObject(pThis->m_hRequestEvent, INFINITE);
		TRACE(_T("[aral1] Received Request of Trans Event (dwTransWait : %d) \n"), dwTransWait);
		if(pThis->m_hTransThread == NULL)
		{
			TRACE(_T("[aral1] Exit (pThis->m_hTransThread == NULL) \n"));
			break;
		}

		TRANSLATION_OBJECT* pTransObject = pThis->m_pCurTransObj;
		TRANSLATION_OBJECT* pPrevObject = pTransObject->pPrevObject;

		if('\0' == ((LPSTR)pPrevObject->pPreTransBuf)[0])
		{
			((LPSTR)pTransObject->pPreTransBuf)[0] = '\0';
		}
		else
		{
			// �� ���� �ؽ�Ʈ�� �ؽ� ���ϱ�
			UINT dwTextHash = MakeStringHash((LPCSTR)pPrevObject->pPreTransBuf);

			// ĳ�ÿ��� ã�ƺ���
			map<UINT, CTextElement*>::iterator iter = mapCache.find(dwTextHash);
			CTextElement* pTextElem = NULL;

			// ĳ�ÿ� ������
			if(mapCache.end() != iter)
			{
				pTextElem = iter->second;

				// ����Ʈ���� ����������
				pTextElem->pPrevLink->pNextLink = pTextElem->pNextLink;
				pTextElem->pNextLink->pPrevLink = pTextElem->pPrevLink;

			}
			// ĳ�ÿ� ������ ����Ʈ������ ����
			else
			{
				LPCSTR	cszJpnText		= (LPSTR)pPrevObject->pPreTransBuf;
				BOOL	bRemoveTrace	= (pTransObject->wszObjectOption[0] && pTransObject->wszObjectOption[0] == L'1') ? TRUE : FALSE;
				BOOL	bRemoveDupSpace	= (pTransObject->wszObjectOption[0] && pTransObject->wszObjectOption[1] == L'1') ? TRUE : FALSE;

				// ��ȣ ���� ���ڵ� (�ɼ� ����)
				if(bRemoveTrace)
				{
					pThis->EncodeTrace(cszJpnText, pBuf1);
					cszJpnText = pBuf1;
				}

				// ���� ���� ���ڵ�
				pThis->EncodeTwoByte(cszJpnText, pBuf2);
				cszJpnText = pBuf2;

				char* szTrans = pThis->m_EZDLL.J2K_TranslateMMNT(0, cszJpnText);

				if(szTrans)
				{
					LPCSTR cszKorText = szTrans;

					// �ߺ� ���� ���� (�ɼ� ����)
					if(bRemoveDupSpace)
					{
						pThis->FilterDupSpaces(cszKorText, pBuf2);
						cszKorText = pBuf2;
					}

					// ���� ���� ���ڵ�
					pThis->DecodeTwoByte(cszKorText, pBuf1);
					cszKorText = pBuf1;

					// ��ȣ ���� ���ڵ�
					if(bRemoveTrace)
					{
						pThis->FilterTrace(cszKorText, pBuf2);
						pThis->DecodeTrace(pBuf2, pBuf1);
						cszKorText = pBuf1;
					}

					pThis->m_EZDLL.J2K_FreeMem(szTrans);

					pTextElem = new CTextElement();
					pTextElem->dwHash = dwTextHash;
					pTextElem->strTranslatedText = cszKorText;

					// ĳ�ð� �� á�ٸ� ���ڸ� Ȯ���� ����
					if(mapCache.size() >= 10000)
					{
						CTextElement* pDelElem = CacheTail.pPrevLink;
						CacheTail.pPrevLink = pDelElem->pPrevLink;

						mapCache.erase(pDelElem->dwHash);
						delete pDelElem;
					}

					// ĳ�ÿ� ����
					mapCache.insert(pair<UINT, CTextElement*>(dwTextHash, pTextElem));
				}

			}


			if(pTextElem && pTransObject->pPreTransBuf && pTransObject->nPreTransBufLen)
			{
				size_t nLen = pTextElem->strTranslatedText.length();

				if( nLen > (pTransObject->nPreTransBufLen-1) )
				{
					strncpy((LPSTR)pTransObject->pPreTransBuf, pTextElem->strTranslatedText.c_str(), pTransObject->nPreTransBufLen-1);
					((LPSTR)pTransObject->pPreTransBuf)[pTransObject->nPreTransBufLen-1] = _T('\0');
				}
				else
				{
					strcpy((LPSTR)pTransObject->pPreTransBuf, pTextElem->strTranslatedText.c_str());
				}

				// ��� �������� ����
				pTextElem->pPrevLink = &CacheHead;
				pTextElem->pNextLink = CacheHead.pNextLink;

				CacheHead.pNextLink->pPrevLink = pTextElem;
				CacheHead.pNextLink = pTextElem;
			}

		}// end of else of if('\0' == pThis->m_pJpnText[0])

		BOOL bSetEventResult = ::SetEvent(pThis->m_hResponseEvent);
		TRACE(_T("[aral1] Response Trans Event ON!! (result : %d) \n"), bSetEventResult);
	}
	
	// ���� ����
	delete [] pBuf1;
	delete [] pBuf2;
	
	// ���� ���ҵ� ����
	TRACE(_T("[aral1] Map : %d \n"), mapCache.size());
	for(map<UINT, CTextElement*>::iterator iter = mapCache.begin();
		iter != mapCache.end();
		iter++)
	{
		CTextElement* pTextElem = iter->second;
		if(pTextElem) delete pTextElem;
	}

	TRACE(_T("[aral1] Trans Thread Terminated!!!!! \n"));

	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
// �����Ⱑ ��ȣ�� ������� �ٿ������� ��� �Ϻ� ���ӿ��� ���� ����
// ���� �̷� ��� ����
// ex) "��(��)��⸦ �Ծ���." -> "�Ұ�⸦ �Ծ���."
//
//////////////////////////////////////////////////////////////////////////
void CezTransXPApp::FilterTrace(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar || '\0'==cszKorSrc[0] || '\0'==szKorTar[0]) return;

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
				//// AT ���� �����̸�
				//if( strncmp(&cszKorSrc[i], "(\at", 4) == 0 )
				//{
				//	do
				//	{
				//		szKorTar[j++] = cszKorSrc[i++];
				//	}while(cszKorSrc[i-1] != ')' && i<len);
				//}
				//// �ƴ϶� �׳� '(' ��ȣ �̸�
				//else
				//{
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
				//}
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
			else if(cszKorSrc[i] == '<')
			{
				while((cszKorSrc[i] != '>'  && cszKorSrc[i] != ')')&& i<len)
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
			else if(cszKorSrc[i] == ':')
			{
				i++;
			}
			else
			{
				szKorTar[j++] = cszKorSrc[i++];
			}
		}
	}

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
// �����⸦ ��ģ �� ����(' ')�� �ʿ��̻����� �þ�� ��찡 �ִ�.
// �̷� ��� �ϳ��� ����� ��� ����
// ex) "�� , �̷���  ����"   ->   "��, �̷��� ����"
//
//////////////////////////////////////////////////////////////////////////
void CezTransXPApp::FilterDupSpaces(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar || '\0'==cszKorSrc[0] || '\0'==szKorTar[0]) return;

	size_t len = strlen(cszKorSrc);	
	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		// 2����Ʈ ���ڸ�
		if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0')
		{
			if((BYTE)cszKorSrc[i] == 0xA1 &&	//�� �� �� �� 
				0xB8 <= (BYTE)cszKorSrc[i+1] && (BYTE)cszKorSrc[i+1] <= 0xBB)
			{
				if(j>0 && szKorTar[j-1] == ' ') j--;
				szKorTar[j++] = cszKorSrc[i++];
				szKorTar[j++] = cszKorSrc[i++];
				if(cszKorSrc[i] == ' ') i++;
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
			if((cszKorSrc[i] == ' ') && (i+1 < len) && (cszKorSrc[i+1] == ' ' || cszKorSrc[i+1] == ','))
			{
				i++;
			}
			else
			{
				szKorTar[j++] = cszKorSrc[i++];
			}
		}
	}

	szKorTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<FilterDupSpaces Start>");
	OutputDebugStringA(cszKorSrc);
	OutputDebugStringA(szKorTar);
	OutputDebugStringA("<FilterDupSpaces End>");
#endif
}

//////////////////////////////////////////////////////////////////////////
//
// ���� Ư�����ڸ� ����Ʈ������ ������� ASCII�ڵ�� �ٲ������ �̸� ����
// �ϱ����� ���� ��� ��ȯ��Ŵ
// ex) "���檫��" -> "(\atA3BC)�檫(\atA3BE)"
//
//////////////////////////////////////////////////////////////////////////
void CezTransXPApp::EncodeTwoByte(LPCSTR cszJpnSrc, LPSTR szJpnTar)
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
		if((BYTE)cszJpnSrc[i] >= 0x80 && (BYTE)cszJpnSrc[i+1] != '\0')
		{
			int nTarInc = 2;

			szJpnTar[j] = cszJpnSrc[i];
			szJpnTar[j+1] = cszJpnSrc[i+1];

			for(int k=0; _SHIFT_JIS_CHAR_MAP[k][0]; k++)
			{
				if(_SHIFT_JIS_CHAR_MAP[k][2] == cszJpnSrc[i]
					&& _SHIFT_JIS_CHAR_MAP[k][3] == cszJpnSrc[i+1])
				{
					nTarInc = sprintf(&szJpnTar[j], "_&%03u%03u&_", (BYTE)_SHIFT_JIS_CHAR_MAP[k][0], (BYTE)_SHIFT_JIS_CHAR_MAP[k][1]);
				}
			}
			
			i += 2;
			j += nTarInc;
		}
		else
		{
			szJpnTar[j++] = cszJpnSrc[i++];
		}
	}

	szJpnTar[j] = '\0';
	
#ifdef DEBUG
	OutputDebugStringA("<EncodeTwoByte Start>");
	OutputDebugStringA(cszJpnSrc);
	OutputDebugStringA(szJpnTar);
	OutputDebugStringA("<EncodeTwoByte End>");
#endif
}




//////////////////////////////////////////////////////////////////////////
//
// ����Ʈ���� ���� �� ��ȯ���״� ����Ư����ȣ���� ����
// ex)  "(\atA3BC)���ΰ�(\atA3BE)" -> "�����ΰ���"
//
//////////////////////////////////////////////////////////////////////////
void CezTransXPApp::DecodeTwoByte(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar) return;

	if('\0'==cszKorSrc[0])
	{
		szKorTar[0] = '\0';
		return;
	}

	size_t len = strlen(cszKorSrc);

	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{		
		size_t nCopyLen = len-i;
		const char* pEncPtr = strstr(&cszKorSrc[i], "_&");
				
		// "_&" �����Ƚ��� ã�Ҵٸ�
		if(NULL != pEncPtr)
		{
			// ��ȯ��ų �����Ͱ� Ȯ���Ѱ�?
			//if(	(UINT_PTR)pEncPtr - (UINT_PTR)cszKorSrc + 9 < len
			//	&& '&' == *(pEncPtr+8)
			//	&& '_' == *(pEncPtr+9) )
			if(GetEncodedLen(pEncPtr) == 10)
			{
				nCopyLen = (UINT_PTR)pEncPtr - (UINT_PTR)(&cszKorSrc[i]);
			}
			// ��ȯ��Ű�� �ȵǴ� _&���
			else
			{
				pEncPtr = NULL;
				nCopyLen = 2;
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
			// Ư�� ���� ���ڵ�
			int val1, val2;
			sscanf(pEncPtr+2, "%03u%03u", &val1, &val2);
			((BYTE*)szKorTar)[j] = (BYTE)val1;
			((BYTE*)szKorTar)[j+1] = (BYTE)val2;
			
			i += 10;
			j += 2;
		}

	}

	szKorTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<DecodeTwoByte Start>");
	OutputDebugStringA(cszKorSrc);
	OutputDebugStringA(szKorTar);
	OutputDebugStringA("<DecodeTwoByte End>");
#endif
}

//////////////////////////////////////////////////////////////////////////
//
// �ڵ� �����Ǵ� ��ȣ�� Ȯ���� �Ǻ��ϱ� ����
// ���� �ִ� ��ȣ�� ��ȣȭ
// ex) "[�檫]" -> "(\at5B)�檫(\at5D)"
//
//////////////////////////////////////////////////////////////////////////
void CezTransXPApp::EncodeTrace(LPCSTR cszJpnSrc, LPSTR szJpnTar)
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
				|| cszJpnSrc[i] == ':')
			{
				j += sprintf(&szJpnTar[j], "_&%03u&_", (BYTE)cszJpnSrc[i++]);
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
void CezTransXPApp::DecodeTrace(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar) return;

	if('\0'==cszKorSrc[0])
	{
		szKorTar[0] = '\0';
		return;
	}

	size_t len = strlen(cszKorSrc);

	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		size_t nCopyLen = len-i;
		const char* pEncPtr = strstr(&cszKorSrc[i], "_&");

		// "_&" �����Ƚ��� ã�Ҵٸ�
		if(NULL != pEncPtr)
		{
			// ��ȯ��ų �����Ͱ� Ȯ���Ѱ�?
			//if(	(UINT_PTR)pEncPtr - (UINT_PTR)cszKorSrc + 6 < len
			//	&& '&' == *(pEncPtr+5)
			//	&& '_' == *(pEncPtr+6) )
			if(GetEncodedLen(pEncPtr) == 7)
			{
				nCopyLen = (UINT_PTR)pEncPtr - (UINT_PTR)(&cszKorSrc[i]);
			}
			// ��ȯ��Ű�� �ȵǴ� _&���
			else
			{
				pEncPtr = NULL;
				nCopyLen = 2;
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
			sscanf(pEncPtr+2, "%03u", &val1);
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

size_t CezTransXPApp::GetEncodedLen( LPCSTR cszBytes )
{
	size_t nRetVal = 0;

	if(NULL != cszBytes)
	{
		size_t len = strlen(cszBytes);

		if(len >= 7 
			&& '_' == cszBytes[0] 
			&& '&' == cszBytes[1]
			&& isdigit(cszBytes[2])
			&& isdigit(cszBytes[3])
			&& isdigit(cszBytes[4])
			&& '&' == cszBytes[5] 
			&& '_' == cszBytes[6]) nRetVal = 7;
		else if(len >= 10 
			&& '_' == cszBytes[0] 
			&& '&' == cszBytes[1]
			&& isdigit(cszBytes[2])
			&& isdigit(cszBytes[3])
			&& isdigit(cszBytes[4])
			&& isdigit(cszBytes[5])
			&& isdigit(cszBytes[6])
			&& isdigit(cszBytes[7])
			&& '&' == cszBytes[8] 
			&& '_' == cszBytes[9]) nRetVal = 10;
	}

	return nRetVal;
}
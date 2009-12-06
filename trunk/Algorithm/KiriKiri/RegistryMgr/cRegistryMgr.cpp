// CRegistryMgr.cpp: implementation of the CRegistryMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "CRegistryMgr.h"
#include <shlobj.h>

#pragma comment(lib, "shell32.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegistryMgr::CRegistryMgr()
{

}

CRegistryMgr::~CRegistryMgr()
{

}



//---------------------------------------------------------------------------
//	���Լ��� : RegRead
//	���Լ����� : ������Ʈ������ �����͸� �о�´�
//	������ : CString ������Ʈ�����, CString �̸�
//	�ݹ�ȯ�� : ���� ��ο� �̸��� �ش��ϴ� ������Ʈ�� ������(CString ����)
//---------------------------------------------------------------------------
CString CRegistryMgr::RegRead(CString rpath, CString name)
{
	int strindex;
	HKEY hCategoryKey;
	CString temp;
	CString retString = _T("");

	strindex = rpath.Find(_T('\\'));
	if(strindex<0) return retString;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return retString;

	HKEY hKey;
	LONG lRet;
	DWORD type = REG_SZ;
	rpath = rpath.Mid(strindex+1);
	lRet = RegOpenKeyEx(hCategoryKey, rpath, 0, KEY_READ, &hKey);
	
	// Ű�� ���µ� �����ߴٸ�
	if(lRet == ERROR_SUCCESS)
	{
		LPCTSTR pName = NULL;
		if(!name.IsEmpty()) pName = (LPCTSTR)name;

		DWORD size = 0;
		RegQueryValueEx(hKey, name, 0, &type, NULL, &size);

		BYTE* dir = new BYTE[size+2];

		lRet = RegQueryValueEx(hKey, name, 0, &type, dir, &size);
		if(lRet == ERROR_SUCCESS) retString = (LPCTSTR)dir;

		delete dir;

		RegCloseKey(hKey);
	}

	return retString;
}





//---------------------------------------------------------------------------
//	���Լ��� : RegWrite
//	���Լ����� : ������Ʈ���� �����͸� ����
//	������ : CString ������Ʈ�����, CString �̸�, CString ��
//	�ݹ�ȯ�� : ������ TRUE, ���н� FALSE
//---------------------------------------------------------------------------
BOOL CRegistryMgr::RegWrite(CString rpath, CString name, CString value)
{
	int strindex, count;
	CString temp = _T("");
	HKEY hCategoryKey;

	strindex = rpath.Find(_T('\\'));
	if(strindex<0) return FALSE;
	
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return FALSE;

	HKEY hKey;
	LONG lRet;
	DWORD type = REG_SZ;
	rpath = rpath.Mid(strindex+1);
	count = rpath.GetLength();
	if(count>0){				// ������Ʈ�� ���丮 ����
		TCHAR ch;
		temp = _T("");
		for(strindex=0; strindex<count; strindex++)
		{
			ch = rpath.GetAt(strindex);
			
			if(ch==_T('\\') || ch==_T('/'))
			{
				lRet = RegCreateKeyEx(hCategoryKey, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
				if(lRet == ERROR_SUCCESS) RegCloseKey(hKey);
			}

			temp += ch;
		}
	}
	else return FALSE;

	lRet = RegCreateKeyEx(hCategoryKey, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if(lRet != ERROR_SUCCESS) return FALSE;
	if(name.IsEmpty() || name.Compare(_T("0"))==0)
		RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)(LPCTSTR)value, (value.GetLength()+1)*sizeof(TCHAR) );
	else
		RegSetValueEx( hKey, name, 0, REG_SZ, (LPBYTE)(LPCTSTR)value, (value.GetLength()+1)*sizeof(TCHAR) );
	
	RegCloseKey(hKey);
	return TRUE;

}







BOOL CRegistryMgr::RegExist(CString rpath, CString name)
{
	int strindex;
	HKEY hCategoryKey;
	CString temp;
	BOOL retValue = FALSE;
	
	strindex = rpath.Find(_T('\\'));
	if(strindex<0) return retValue;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return retValue;
	
	HKEY hKey;
	LONG lRet;
	DWORD type = REG_DWORD;
	DWORD size = MAX_PATH*2;
	//BYTE dir[MAX_PATH*2];
	rpath = rpath.Mid(strindex+1);
	lRet = RegOpenKeyEx(hCategoryKey, rpath, 0, KEY_READ, &hKey);
	if(lRet == ERROR_SUCCESS){		// Ű�� ���µ� �����ߴٸ�
		if(name.IsEmpty() || name.Compare(_T("0"))==0)
			lRet = RegQueryValueEx(hKey, NULL, 0, NULL, NULL, NULL);
		else
			lRet = RegQueryValueEx(hKey, name, 0, NULL, NULL, NULL);
		
		if(lRet == ERROR_SUCCESS)
		{
			retValue = TRUE;
		}
		RegCloseKey(hKey);
	}
	
	return retValue;
}





//---------------------------------------------------------------------------
//	���Լ��� : RegReadDWORD
//	���Լ����� : ������Ʈ������ DWORD �����͸� �о�´�
//	������ : CString ������Ʈ�����, CString �̸�
//	�ݹ�ȯ�� : ���� ��ο� �̸��� �ش��ϴ� ������Ʈ�� ������(DWORD ����)
//---------------------------------------------------------------------------
DWORD CRegistryMgr::RegReadDWORD(CString rpath, CString name)
{
	int strindex;
	HKEY hCategoryKey;
	CString temp;
	DWORD retValue = 0;

	strindex = rpath.Find(_T('\\'));
	if(strindex<0) return retValue;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return retValue;

	HKEY hKey;
	LONG lRet;
	DWORD type = REG_DWORD;
	DWORD size = MAX_PATH*2;
	BYTE dir[MAX_PATH*2];
	rpath = rpath.Mid(strindex+1);
	lRet = RegOpenKeyEx(hCategoryKey, rpath, 0, KEY_READ, &hKey);
	if(lRet == ERROR_SUCCESS){		// Ű�� ���µ� �����ߴٸ�
		if(name.IsEmpty() || name.Compare(_T("0"))==0)
			lRet = RegQueryValueEx(hKey, NULL, 0, &type, (LPBYTE)&dir, &size);
		else
			lRet = RegQueryValueEx(hKey, name, 0, &type, (LPBYTE)&dir, &size);
		
		if(lRet == ERROR_SUCCESS)
		{
			memcpy( &retValue, &dir, sizeof(DWORD) );
		}
		RegCloseKey(hKey);
	}

	return retValue;
}





//---------------------------------------------------------------------------
//	���Լ��� : RegWriteDWORD
//	���Լ����� : ������Ʈ���� DWORD �����͸� ����
//	������ : CString ������Ʈ�����, CString �̸�, DWORD ��
//	�ݹ�ȯ�� : ������ TRUE, ���н� FALSE
//---------------------------------------------------------------------------
BOOL CRegistryMgr::RegWriteDWORD(CString rpath, CString name, DWORD value)
{
	int strindex, count;
	CString temp = "";
	HKEY hCategoryKey;

	strindex = rpath.Find('\\');
	if(strindex<0) return FALSE;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return FALSE;

	HKEY hKey;
	LONG lRet;
	DWORD type = REG_SZ;
	DWORD size = MAX_PATH;
	rpath = rpath.Mid(strindex+1);
	count = rpath.GetLength();
	if(count>0){				// ������Ʈ�� ���丮 ����
		TCHAR ch;
		temp = "";
		for(strindex=0; strindex<count; strindex++){
			ch = rpath.GetAt(strindex);
			if(ch=='\\' || ch=='/'){
				lRet = RegCreateKeyEx(hCategoryKey, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
				if(lRet == ERROR_SUCCESS) RegCloseKey(hKey);
			}
			temp += ch;
		}
	}
	else return FALSE;

	lRet = RegCreateKeyEx(hCategoryKey, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if(lRet != ERROR_SUCCESS) return FALSE;
	if(name.IsEmpty() || name.Compare(_T("0"))==0)
		RegSetValueEx(hKey, NULL, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
	else
		RegSetValueEx(hKey, name, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
	
	RegCloseKey(hKey);
	return TRUE;

}


















//---------------------------------------------------------------------------
//	���Լ��� : RegDelete
//	���Լ����� : ������Ʈ���� Ư�� �����͸� �����Ѵ�
//	������ : CString ������Ʈ�����, CString �������̸�
//	�������̸��� ���ڿ��̰ų� "0"�� ��� �ش� ��θ� ������ ����
//	����Ű����� ������ �����Ѵ�.
//	�ݹ�ȯ�� : ������ TRUE, ���н� FALSE
//---------------------------------------------------------------------------
BOOL CRegistryMgr::RegDelete(CString rpath, CString name)
{
	int strindex;
	CString temp = "";
	HKEY hCategoryKey;

	strindex = rpath.Find('\\');
	if(strindex<0) return FALSE;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return FALSE;

	
	rpath = rpath.Mid(strindex+1);
	long lRet;
	if(name.IsEmpty() || name.Compare(_T("0"))==0){		// ���� ��α��� ���� �����
		lRet = RegDeleteRecursive(hCategoryKey, rpath);
		if(lRet != ERROR_SUCCESS) return FALSE;
	}
	else{											// Ư��Ű�� �����
		HKEY hKey;
		lRet = RegOpenKeyEx(hCategoryKey, rpath, 0, KEY_ALL_ACCESS, &hKey);
		if(lRet == ERROR_SUCCESS) // ���� �����ϸ�...
		{
			lRet = RegDeleteValue(hKey, name);
			if(lRet != ERROR_SUCCESS) return FALSE;
		}
		else return FALSE;
	}

	return TRUE;
}



// RegDeleteKey() �Լ��� NT���� ����Ű�� ������ ������ �ȵȴ�.
// �׷��� RegDelete�� �������ִ� �Լ��� �������. ���� ������ �ʴ´�.
LONG CRegistryMgr::RegDeleteRecursive(HKEY hKey, LPCTSTR lpSubKey)
{
    HKEY newKey;
    TCHAR newSubKey[MAX_PATH];
    LONG Result;
    DWORD Size;
    FILETIME FileTime;

    RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &newKey);
    Result = ERROR_SUCCESS;
    while(TRUE) {
        Size = MAX_PATH;
        // ��� Ű�� ���� �ǹǷ� dwIndex�� �׻� 0�� �־��־�� �Ѵ�.
        // ���� for������ i�� �����ø� ����ϸ� �ϳ������ �ϳ� �پ�Ѿ� ���� ���´�.
        Result = RegEnumKeyEx(newKey, 0, newSubKey, &Size, NULL, NULL, NULL, &FileTime);
        if (Result != ERROR_SUCCESS) break;
        Result = RegDeleteRecursive(newKey, newSubKey);
        if (Result  != ERROR_SUCCESS) break;
    }
    RegCloseKey(newKey);
 
    return RegDeleteKey(hKey, lpSubKey);
}



//////////////////////////////////////////////////////////////////////////
//
// ������Ʈ���� Ȯ���ڸ� ����ϰ� ���� �뺸
//
BOOL CRegistryMgr::RegistFileType(CString strExt, CString strAppPath, CString strIconPath)
{
	if(strExt.IsEmpty() || strAppPath.IsEmpty()) return FALSE;
	if(strIconPath.IsEmpty()) strIconPath = strAppPath;
	
	CString strKey = strExt + "ClassApp";
	
	// strExt�� '.' ����
	strExt = "." + strExt;

	// Ȯ���� ���
	CRegistryMgr::RegWrite(
		"HKEY_CLASSES_ROOT\\" + strExt, 
		"", 
		strKey
	);

 	// ����Ŀ�ǵ� ���
	CRegistryMgr::RegWrite(
		"HKEY_CLASSES_ROOT\\" + strKey + "\\Shell\\Open\\Command", 
		"", 
		strAppPath + " \"%1\""
	);

 	// Ȯ���ڿ� ���� ������ ���
	CRegistryMgr::RegWrite(
		"HKEY_CLASSES_ROOT\\" + strKey + "\\DefaultIcon", 
		"", 
		strIconPath
	);

 	// ���� �뺸
 	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);

	return TRUE;

}



//////////////////////////////////////////////////////////////////////////
//
// ������Ʈ���� Ȯ���ڸ� ��� �����ϰ� ���� �뺸
//
BOOL CRegistryMgr::DeleteFileType(CString strExt)
{
	if(strExt.IsEmpty()) return FALSE;
	

	CString strKey = strExt + "ClassApp";
	
	// ���� ���α׷� ����
	CRegistryMgr::RegDelete("HKEY_CLASSES_ROOT\\" + strKey, "");

	// strExt�� '.' ����
	strExt = "." + strExt;

	// Ȯ���� ����
	CRegistryMgr::RegDelete("HKEY_CLASSES_ROOT\\" + strExt, "");

 	// ���� �뺸
 	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);

	return TRUE;
}


#pragma once

#include "../ITextManager.h"
#include <set>
#include <map>

using namespace std;

class CNonCachedTextArg;
typedef set<CNonCachedTextArg*> CNonCachedTextArgSet;

//////////////////////////////////////////////////////////////////////////
//
class CNonCachedTextArgMgr : public ITextManager
{
private:
	static CNonCachedTextArgMgr* _inst;

	UINT					m_aDupCntTable[16];			// �ߺ� ī��Ʈ ���̺�
	CNonCachedTextArgSet	m_setActivatedArgs;			// Ȱ��ȭ�� �ؽ�Ʈ ����
	CNonCachedTextArgSet	m_setInactivatedArgs;		// ��Ȱ��ȭ �Ǿ� �ִ� ���ڵ�
	BOOL					m_bMatchLen;

	BOOL	AddTextArg(LPCWSTR wszText);		// ���ο� ���ڿ� �ĺ��� �߰��Ѵ�
	BOOL	TestCharacter(wchar_t wch);					// ���ڿ� �ĺ��� ��ü�� �׽�Ʈ�Ѵ�. (���̻� ��ġ���� �ʴ� �ĺ��� �ٷ� ����)
	BOOL	GetBestTranslatedCharacter(wchar_t* pTransResultBuf);		// �ְ�� Ȯ���� ���� ���� ���ڸ� ��ȯ
	CNonCachedTextArg*	FindString(LPCWSTR pTestString, int nSize = -1);
	BOOL	GetTranslatedStringA(INT_PTR ptrBegin, LPCSTR szOrigString, int nOrigSize, wchar_t *pTransResultBuf, int nBufSize, int &nTransSize);
	BOOL	GetTranslatedStringW(INT_PTR ptrBegin, LPCWSTR wszOrigString, int nOrigSize, wchar_t *pTransResultBuf, int nBufSize, int &nTransSize);
	BOOL	IsEmpty();									// ���� Ȱ��ȭ �� �ؽ�Ʈ ���ڵ��� �ϳ��� ���°�?
	int		SearchStringA(INT_PTR ptrBegin, char ch1, char ch2);
	int		SearchStringW(INT_PTR ptrBegin, wchar_t wch);

public:

	CNonCachedTextArgMgr(void);
	static CNonCachedTextArgMgr* GetInstance(){ return _inst; };
	BOOL IsMatchLen(){ return m_bMatchLen; };

	//////////////////////////////////////////////////////////////////////////
	// Override Functions
	virtual ~CNonCachedTextArgMgr(void);

	virtual BOOL Init();
	virtual void Close();

	virtual DWORD NewGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	virtual DWORD NewGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	virtual BOOL NewTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	virtual BOOL NewTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	virtual BOOL NewExtTextOutA(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	virtual BOOL NewExtTextOutW(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	virtual int NewDrawTextA(
		HDC hDC,          // handle to DC
		LPCSTR lpString,  // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	virtual int NewDrawTextW(
		HDC hDC,          // handle to DC
		LPCWSTR lpString, // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	virtual int NewDrawTextExA(
		HDC hdc,                     // handle to DC
		LPSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	virtual int NewDrawTextExW(
		HDC hdc,                     // handle to DC
		LPWSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);
};

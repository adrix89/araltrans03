#pragma warning(disable:4996)
#pragma warning(disable:4101)

#include "stdafx.h"
#include "BinaryPatternSearcher.h"


CBinaryPatternSearcher::CBinaryPatternSearcher(void)
{
}

CBinaryPatternSearcher::~CBinaryPatternSearcher(void)
{
}


int CBinaryPatternSearcher::Search(LPBYTE pBinData, UINT nBinSize, LPCTSTR cszBinPattern, int nAfter)
{
	int nRetVal = -1;

	LPBYTE pPattern = NULL;
	LPBYTE pPatternMask = NULL;

	try
	{
		if(NULL == cszBinPattern) throw -1;
		if(nAfter < 0) throw -2;
		
		CString strBinPattern = cszBinPattern;
		strBinPattern.Remove(_T(' '));
		strBinPattern = strBinPattern.MakeLower();

		int nPatternSize = strBinPattern.GetLength() / 2;
		if(nPatternSize == 0) throw -3;

		// ���� �޸� �Ҵ�
		pPattern = new BYTE[nPatternSize];
		pPatternMask = new BYTE[nPatternSize];

		// ���Ϲ迭�� ���ϸ���ũ�迭 ����
		for(int i=0; i<nPatternSize; i++)
		{
			CString strHex = strBinPattern.Mid(i*2, 2);
			if(_T('x') == strHex[0])
			{
				pPattern[i] = 0x00;
				pPatternMask[i] = 0x00;
			}
			else
			{
				UINT hex_val = 0;
				_stscanf(strHex, _T("%x"), &hex_val);
				pPattern[i] = (BYTE)hex_val;
				pPatternMask[i] = 0xFF;
			}
		}

		// �˻�
		int cnt = nBinSize - nPatternSize;
		int hit = 0;
		for(int i=nAfter; i<(int)nBinSize; i++)
		{
			
			for(hit=0; hit<nPatternSize; hit++)
			{
				if( (pBinData[i+hit] & pPatternMask[hit]) != pPattern[hit] ) break;
			}
			
			if(hit == nPatternSize)
			{
				nRetVal = i;
				break;
			}
		}

	}
	catch (int nErrCode)
	{
	}

	// ���� �޸� ����
	if(pPattern) delete [] pPattern;
	if(pPatternMask) delete [] pPatternMask;

	return nRetVal;
}

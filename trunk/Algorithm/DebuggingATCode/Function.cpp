#include "stdafx.h"
#include "Function.h"

CFunction::CFunction(UINT_PTR ptrFunction)
{
	m_ptrFunction = ptrFunction;	// �Լ�������
	m_strLastJapaneseText = _T("");		// ��Ʈ�� ������ �Ϻ��� �ؽ�Ʈ
	m_strLastKoreanText = _T("");		// ��Ʈ�� ������ �Ϻ��� �ؽ�Ʈ
}

CFunction::~CFunction(void)
{
}

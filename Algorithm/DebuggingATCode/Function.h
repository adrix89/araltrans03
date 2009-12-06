#pragma once
#include <Windows.h>
#include <map>

//struct FuncPtrLess : public binary_function<const CFunction*, const CFunction*, bool>
//{
//	bool operator()(const CFunction* pFunc1, const CFunction* pFunc2) const
//	{
//		return  ((CFunction*)pFunc1)->GetFuncPtr() < ((CFunction*)pFunc2)->GetFuncPtr();
//	}
//};


using namespace std;

class CFunction
{
	friend class CATTextArgMgr;

private:
	UINT_PTR		m_ptrFunction;				// �Լ�������
	map<size_t,int>	m_mapDistScores;			// ȣ��� ������ ���� ������
	CString			m_strLastJapaneseText;		// ��Ʈ�� ������ �Ϻ��� �ؽ�Ʈ
	CString			m_strLastKoreanText;		// ��Ʈ�� ������ �Ϻ��� �ؽ�Ʈ

public:
	CFunction(UINT_PTR ptrFunction);
	~CFunction(void);

};

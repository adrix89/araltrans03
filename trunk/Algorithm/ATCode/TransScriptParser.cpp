// EffectScript.cpp: implementation of the CScript class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
#pragma warning(disable:4267)

#include "stdafx.h"
#include "TransScriptParser.h"
//#include "EffectScriptCallback.h"
//#include "io.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTransScriptParser::CTransScriptParser()
{
	//m_pCallbackClass = pCallbackObj;		// ��ũ��Ʈ �ݹ� �����̳�
	//m_pCallbackClass->m_pScriptContainer = this;
	
	m_bContinuousMode = true;

	m_pSourcePtr = NULL;					// ��ũ��Ʈ �Ľ� ������
	m_strLastError = L"";					// ������ ���� ����
	nBlockLevel = 0;						// ��� ����
	nQuitLevel = 0;							// ���Ż�� ����
	stackLoopBlock = L"";					// �������� ����

	ExecuteScript(_T("int EAX;int EBX;int ECX;int EDX;int ESI;int EDI;int EBP;int ESP;"));
	//// (0:void, 1:int, 2:string)
}


CTransScriptParser::~CTransScriptParser()
{
	DeleteAllVariables(&m_aVariableTable);		// �������̺��� �޸𸮿��� ����
	DeleteAllFunctions(&m_aFunctionTable);		// �Լ����̺��� �޸𸮿��� ����
	DeleteAllIdentifiers(&m_aReplaceArray);		// #define ������ ����
}

// ��ũ��Ʈ �����͸� �����Ͽ� �����Ѵ�.
void CTransScriptParser::ExecuteScript(LPCTSTR buffer)
{
	m_pSourcePtr = (wchar_t*)buffer;
	vector<CString*> m_aTokens;

	/////////////////// ��ūȭ ///////////////////
	if(Tokenization(&m_aTokens)==FALSE){
		//AfxMessageBox(m_strLastError, MB_OK);
	}
	int a = (int)m_aTokens.size();
	//ShowTokens(&m_aTokens);

	/////////////////// ó�� /////////////////////
	InterpretBlock(&m_aTokens);


	/////////////// �پ� ������ ���� //////////////
	DeleteAllTokens(&m_aTokens);				// ��ū ��Ʈ������ �޸𸮿��� ����

}

// ���� �ҽ������ͷκ��� �Ѷ����� �о� ��ȯ
// �ҽ������͸� ���� �������� �ڵ� ������Ŵ
CString CTransScriptParser::GetLine()
{
	CString returnLine = _T("");
	while(*m_pSourcePtr){
		if(*m_pSourcePtr==0x0A){		// ������ ����Ǿ��ٸ�
			m_pSourcePtr++;
			break;							// ������ ��������
		}
		else if(*m_pSourcePtr==0x0D){
			m_pSourcePtr++;
		}
		else{							// ������ ���� �ȵ�����
			returnLine += *m_pSourcePtr;
			m_pSourcePtr++;
		}
	}
	return returnLine;
}

// Ư�� ��Ʈ���� �޾Ƽ� ��ū���� �������ش�
// tokens��� ������ �迭�� CString���� �ּҸ� ��´�
// ������ ��� TRUE��ȯ
BOOL CTransScriptParser::Tokenize(CString str, vector<CString*> *tokens)
{
	int length = str.GetLength();
	int strindex = 0;
	CString* identifier = new CString;
	*identifier = _T("");
	while(strindex<length){
		wchar_t ch = str[strindex];

		// �����̽�, ��, ���Ͱ� ������ ���
		if(ch==_T(' ') || ch==_T('\t') || ch==0x0A || ch==0x0D){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}
			strindex++;
		}

		// �ֵ���ǥ�� ��������
		else if(ch==0x22){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}
			*identifier += ch;
			strindex++;
			do{
				if(strindex>=length){
					m_strLastError = _T("���ڿ� ��� ������ �߸��Ǿ����ϴ�.");
					return FALSE;
				}
				ch = str[strindex];
				if(ch==_T('\\')){				// Ư�����ڶ��
					strindex++;
					wchar_t tempch = str[strindex];
					switch(tempch){
						case 'n': tempch = '\n'; break;
						case 'r': tempch = '\r'; break;
						case 't': tempch = '\t'; break;
						case '\"': tempch = '\"'; break;
						case 'b': tempch = '\b'; break;
						case '0': tempch = '\0'; break;
					}
					*identifier += tempch;
				}
				else *identifier += ch;		// �Ϲݹ��ڶ��
				strindex++;
			}while(ch!=0x22);
			identifier = AddToken(tokens, identifier);			
		}

		// ���� Ư�����ڰ� ��������
		else if( ch==';' || ch=='(' || ch==')' || ch=='{' || ch=='}' || ch==',' || ch=='[' || ch==']' ){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}
			*identifier = ch;
			identifier = AddToken(tokens, identifier);
			strindex++;
		}

		// �� �����ڰ� ��������
		else if(ch=='&' || ch=='|'){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);	
			}

			if(str[strindex+1]==ch){
				*identifier = ch;
				*identifier += ch;
				identifier = AddToken(tokens, identifier);
				strindex += 2;
			}
			else{
				m_strLastError = L"�߸��� �������� �Դϴ�. &&(o) &(x)";
				return FALSE;
			}
		}

		//  ���� �����ڰ� ��������
		else if((ch=='+' || ch=='-') && ( str.GetLength()>strindex+1 && str[strindex+1]==ch )){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);	
			}
			*identifier = ch;
			*identifier += ch;
			identifier = AddToken(tokens, identifier);
			strindex += 2;
		}

		// �ּ� ���ڰ� ��������
		else if(ch=='/' && str[strindex+1]=='/'){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}
			while(strindex<str.GetLength()){
				if(str[strindex]==0x0A){strindex++; break;}
				strindex++;
			}
		}

		// ���Ա�ȣ�� ��������
		else if(ch=='=' || (ch=='<' || ch=='>' || ch=='!' || ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='%')&&(str.GetLength()>strindex+1)){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}

			*identifier = ch;
			strindex++;
			if(str[strindex]=='='){
				*identifier += '=';
				strindex ++;
			}
			identifier = AddToken(tokens, identifier);
		}

		// ���ڰ� ��������
		else if('0'<= ch && ch<='9'){
			if(identifier->IsEmpty()){
				while(('0'<= ch && ch<='9') || ('A'<= ch && ch<='Z') || ('a'<= ch && ch<='z')){
					*identifier += ch;
					strindex++;
					if(strindex>=str.GetLength()) break;
					ch = str[strindex];					
				}
				identifier = AddToken(tokens, identifier);
			}
			else{
				*identifier += ch;
				strindex++;
			}
		}

		// ���ĺ� �Ǵ� �ѱ� �����϶�
		else if(('A'<=ch && ch<='Z') || ('a'<=ch && ch<='z') || (0xAC00<=ch && ch<=0xD743) || ch=='_'){
			*identifier += ch;
			strindex++;
		}

		// �׿�
		else{
			m_strLastError = ch + _T("�� �˼����� �����Դϴ�.");
			return FALSE;
		}
	}


	if(identifier->IsEmpty()){
		delete identifier;
	}
	else{
		tokens->push_back(identifier);
	}
	return TRUE;
}

// #define������ ���ǵ� �ĺ��ڸ� �˾Ƴ���
// �ش繮�ڿ��� ���� ��ȯ�� �ʿ��ϸ� m_aReplaceArray�� �ε����� ����
// ��ȯ�� �ʿ� ������ -1�� ����
int CTransScriptParser::GetReplaceIndex(CString before)
{
	CString temp;
	for(size_t i=0; i<m_aReplaceArray.size(); i++){
		temp = ((CReplaceIdentifier*)m_aReplaceArray[i])->m_strBefore;
		if(temp.Compare(before)==0) return i;
	}
	return -1;
}

// �����͹迭�� �ش� ��Ʈ���� �ּҸ� �ִ´�
// �ְ��� ���ο� CString��ü�� �����Ͽ� �� �ּҸ� �����Ѵ�
CString* CTransScriptParser::AddToken(vector<CString*> *pTokenList, CString *pString)
{
	int index = GetReplaceIndex(*pString);
	if(index>=0) *pString = ((CReplaceIdentifier*)m_aReplaceArray[index])->m_strAfter;
	pTokenList->push_back(pString);
	pString = new CString;
	*pString = _T("");
	return pString;
}

int CTransScriptParser::GetDataType(CString strType)
{
	if(strType.Compare(_T("void"))==0) return 0;
	else if(strType.Compare(_T("int"))==0) return 1;
	else if(strType.Compare(_T("string"))==0) return 2;
	// ���� ���������� ��� ��ġ���� �ʴ´ٸ� -1 ����
	else return -1;

}

// ��ȣ {,(�� �����ϴ� �κк��� �����Ǵ� },)������ ��ŵ�ϴ�
// ��ū �ε����� �������ش�
// ��ȣ },)�� �����ϸ� �ݴ�� �˻��Ѵ�

int CTransScriptParser::SkipBlock(vector<CString*>* paTokens, int tindex)
{
	CString tk, opentrace, closetrace;
	opentrace = *((CString*)(*paTokens)[tindex]);
	if(opentrace.Compare(L"{")==0) closetrace = L"}";
	else if(opentrace.Compare(L"}")==0) closetrace = L"{";
	else if(opentrace.Compare(L"(")==0) closetrace = L")";
	else if(opentrace.Compare(L")")==0) closetrace = L"(";
	else if(opentrace.Compare(L"[")==0) closetrace = L"]";
	else if(opentrace.Compare(L"]")==0) closetrace = L"[";
	else return -1;

	int level = 1;
	if(opentrace.Compare(L"{")==0 || opentrace.Compare(L"(")==0 || opentrace.Compare(L"[")==0){
		int maxindex = paTokens->size();
		tindex++;
		while(tindex<maxindex){
			tk = *((CString*)(*paTokens)[tindex]);
			if(tk.Compare(opentrace)==0) level++;
			else if(tk.Compare(closetrace)==0){
				level--;
				if(level==0) return tindex;
			}
			tindex++;
		}
	}
	else{
		tindex--;
		while(tindex>=0){
			tk = *((CString*)(*paTokens)[tindex]);
			if(tk.Compare(opentrace)==0) level++;
			else if(tk.Compare(closetrace)==0){
				level--;
				if(level==0) return tindex;
			}
			tindex--;
		}
	}

	return -1;
}

// �����͹迭�� ��� ���ҵ��� delete�� �������ִ� �Լ�
BOOL CTransScriptParser::DeleteElementsAll(vector<void*> *pArray)
{
	int i, count;
	count = (int)pArray->size();
	for(i=0; i<count; i++){
		delete (*pArray)[i];				// ��� ���ҵ��� �޸𸮿��� ����
	}
	pArray->clear();
	return TRUE;
}

// ��ūȭ ������ �����ϴ� �Լ� ������ TRUE��ȯ
BOOL CTransScriptParser::Tokenization(vector<CString*>* paTokens)
{
	//m_pSourcePtr = m_pSourceBuffer;
	while(*m_pSourcePtr){		// NULL���ڸ� ���������� ����

		CString line = GetLine();		// ������ �о�´�
		line = line.Trim();
		if(line.IsEmpty()) continue;

		if(line[0]=='#'){			// ��ó�����̶��
			//int cntToken;
			CString command = _tcstok((LPTSTR)(LPCTSTR)line, _T(" \t"));

			if(command.Compare(_T("#define"))==0){	// ���� define����̸�
				CReplaceIdentifier* pRI = new CReplaceIdentifier;
				CString before = _tcstok(NULL, _T(" \t"));
				CString after = _tcstok(NULL, _T(" \t"));
				if(before.IsEmpty() || after.IsEmpty()){
					m_strLastError = L"#define ������ �߸��Ǿ����ϴ�.";
					//MessageBox(NULL, m_strLastError, L"��ũ��Ʈ����", MB_OK);
					return FALSE;
				}
				pRI->m_strBefore = before;
				pRI->m_strAfter = after;

				m_aReplaceArray.push_back(pRI);
			}
			else if(command.Compare(_T("#include"))==0){	// ���� include����̸�
				CString filename = _tcstok(NULL, _T(" \t"));
				
				filename.Replace(_T("\""), _T(""));
				filename.Replace(_T("<"), _T(""));
				filename.Replace(_T(">"), _T(""));

				if(filename.IsEmpty()){
					m_strLastError = L"#include ������ �߸��Ǿ����ϴ�.";
					//MessageBox(NULL, m_strLastError, L"��ũ��Ʈ����", MB_OK);
					return FALSE;
				}
				// ��Ŭ��� ��ũ��Ʈ ����
				IncludeScriptFile(filename, paTokens);
			}
			else{								// �������� �ʴ� ��ó�� ����϶�
				m_strLastError = L"�������� �ʴ� ��ó�� ��ɾ��Դϴ�.";
				//MessageBox(NULL, m_strLastError, L"��ũ��Ʈ����", MB_OK);
				return FALSE;
			}
		}
		else{							// �Ϲ� �����̸�
			if(!Tokenize(line, paTokens)) return FALSE;
		}
	}

	return TRUE;	
}


void CTransScriptParser::ShowTokens(vector<CString*> *paTokens)
{
	CString temp = L"";
	for(size_t j=0; j<paTokens->size(); j++){
		temp += (wchar_t)'(';
		temp += *((CString*)(*paTokens)[j]);
		temp += (wchar_t)')';
	}
	MessageBox(NULL, temp, L"�ؼ��� ��ū��", MB_OK);
}

// �־��� ��ū���� ����
void* CTransScriptParser::InterpretBlock(vector<CString*> *paTokens, int* pRetType)
{	
	nBlockLevel++;

	int tindex = 0;
	int maxindex = paTokens->size();
	int startSP = m_aVariableTable.size();
	int startFP = m_aFunctionTable.size();
	CString token;
	void* pRetPtr = NULL;

	while(tindex<maxindex){
		token = *((CString*)(*paTokens)[tindex]);

		///////////////////////// {��}�� ��Ÿ������ /////////////////////////////
		if(token.Compare(L"{")==0){
			int beginindex = tindex+1;
			tindex = SkipBlock(paTokens, tindex);
			int endindex = tindex - 1;
			vector<CString*> temptokens;
			if(GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){
				pRetPtr = InterpretBlock(&temptokens, pRetType);
			}
		}


		///////////////////////// ����, �Լ� �����϶� /////////////////////////////
		else if(token.Compare(L"int")==0 || token.Compare(L"string")==0 || token.Compare(L"void")==0){
			// �Լ� ���Ǻζ��
			if(*((*paTokens)[tindex+2]) == L"(")
			{
				CFunction* newfunc = new CFunction();
				newfunc->SetBlockLevel(nBlockLevel);
				newfunc->SetType(GetDataType(token));
				newfunc->SetName(*((*paTokens)[++tindex]));

				// �Ű����� ����
				tindex += 2;
				while(*((*paTokens)[tindex]) != L")")
				{
					CVariable* newparam = new CVariable();
					newparam->SetType(GetDataType(*(*paTokens)[tindex]));
					newparam->SetName(*((*paTokens)[++tindex]));
					newfunc->m_aParams.push_back(newparam);
					if(*((*paTokens)[++tindex]) != L",") break;
					else tindex++;
				}

				// �Լ���ü ����
				if(*((*paTokens)[++tindex]) != L"{")
				{
					//MessageBox(NULL, L"�Լ� ��ü�� �����ϴ�.", L"��ũ��Ʈ����", MB_OK);
				}

				int beginindex = tindex+1;
				tindex = SkipBlock(paTokens, tindex);
				int endindex = tindex - 1;
				vector<CString*>* pTemptokens = new vector<CString*>();
				if(GetSubPtrArray(pTemptokens, paTokens, beginindex, endindex))
					newfunc->SetValue(pTemptokens);

				newfunc->m_pScriptContainer = this;
				m_aFunctionTable.push_back(newfunc);

			}

			// ���������̶��
			else
			{
				CVariable* newvar = new CVariable();
				int vartype = GetDataType(token);
				newvar->SetBlockLevel(nBlockLevel);
				newvar->SetType(vartype);
				newvar->SetName(*((*paTokens)[++tindex]));

				// �迭�̶��
				if(*((*paTokens)[tindex+1]) == L"[")
				{
					// ������ ������
					int nArraySize = 0;
					int endindex = SkipBlock(paTokens, tindex+1);
					if(endindex<0)
					{
						//MessageBox(NULL, L"�迭���𿡼� �ݴ� ��ȣ�� �����ϴ� (']')", L"��ũ��Ʈ����", MB_OK);
					}

					vector<CString*> temptks;
					if(!GetSubPtrArray(&temptks, paTokens, tindex+2, endindex-1))
					{
						//MessageBox(NULL, L"�迭������ ������ �����κ��� �߸��Ǿ����ϴ�", L"��ũ��Ʈ����", MB_OK);
					}

					int retType = 0;
					nArraySize = *((int*)GetValue(&temptks, &retType));

					// �迭 ���� ����
					vector<CVariable*>* pVector = new vector<CVariable*>();
					
					// �ʱ�ȭ
					for(int i=0; i<nArraySize; i++)
					{
						CVariable* pElement = new CVariable();
						pElement->SetType(vartype);
						pElement->SetBlockLevel(nBlockLevel);

						if(vartype==1){
							pElement->SetValue((void*)new int);
							*((int*)pElement->GetValue()) = 0;		// 0���� �ʱ�ȭ 
						}
						else if(vartype==2){
							pElement->SetValue((void*)new CString);
							*((CString*)pElement->GetValue()) = L"";	// ""���� �ʱ�ȭ
						}

						pVector->push_back(pElement);

					}

					// �迭 ���͸� ������ ����
					newvar->SetValue((void*)pVector);
					newvar->m_bIsArray = TRUE;
				}
				// �迭�� �ƴѰ��
				else
				{
					if(vartype==1){
						newvar->SetValue((void*)new int);
						*((int*)newvar->GetValue()) = 0;		// 0���� �ʱ�ȭ 
					}
					else if(vartype==2){
						newvar->SetValue((void*)new CString);
						*((CString*)newvar->GetValue()) = L"";	// ""���� �ʱ�ȭ
					}
					else
					{
						//MessageBoxA(NULL, "�� �� ���� ����Ÿ��", "��ũ��Ʈ����", MB_OK);
					}
				}

				m_aVariableTable.push_back(newvar);

				token = *((*paTokens)[++tindex]);
				while(token.Compare(L";")){
					if(token.Compare(L"=")==0){
						vector<CString*> temptokens;
						CString nexttoken;
						while(1){
							temptokens.push_back((*paTokens)[++tindex]);
							nexttoken = *((*paTokens)[tindex+1]);
							if(nexttoken.Compare(L"(")==0){
								int endtrace = SkipBlock(paTokens, tindex+1);
								for(tindex++; tindex < endtrace; tindex++){
									temptokens.push_back((*paTokens)[tindex]);
								}
								tindex--;
							}
							else if(nexttoken.Compare(L";")==0 || nexttoken.Compare(L",")==0) break;
						}
						int rettype;
						void* retptr = GetValue(&temptokens, &rettype);
						if(retptr){
							newvar->SetValue(RemakeValue(retptr, rettype, newvar->GetType()));
							DeleteInstance(retptr, rettype);
						}
					}
					else if(token.Compare(L",")==0){
						newvar = new CVariable();
						newvar->SetName(*((*paTokens)[++tindex]));
						newvar->SetType(vartype);
						if(vartype==1){
							newvar->SetValue((void*)new int);
							*((int*)newvar->GetValue()) = 0;		// 0���� �ʱ�ȭ 
						}
						else{
							newvar->SetValue((void*)new CString);
							*((CString*)newvar->GetValue()) = L"";	// ""���� �ʱ�ȭ
						}					
						m_aVariableTable.push_back(newvar);
					}
					token = *((*paTokens)[++tindex]);
				}
			}
		}

		//////////////////////// ����϶� ////////////////////////////////
		else if(token.Compare(L"break")==0){
			int nWantLevel = 1;		// ��������� �ݺ��� ����
			token = *((*paTokens)[++tindex]);
			if(token.Compare(L";")!=0){
				nWantLevel = _wtoi(token);
				tindex++;
			}
			if(nWantLevel > stackLoopBlock.GetLength())
			{
				//MessageBoxA(NULL, "break ���� ������ �߸��Ǿ����ϴ�", "��ũ��Ʈ����", MB_OK);
			}
			else{
				nQuitLevel = nBlockLevel - stackLoopBlock[nWantLevel-1];
			}
		}
		else if(token.Compare(L"return")==0){
			vector<CString*> temptokens;
			int rettype;
			int beginindex, endindex;

			tindex++;
			beginindex = tindex;
			tindex = SkipStatement(paTokens, tindex);
			endindex = tindex - 1;
			if(beginindex<=endindex && GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){	// ����� ���⿡ �����ߴٸ�
				pRetPtr = GetValue(&temptokens, &rettype);
				if(pRetType) *pRetType = rettype;
			}

			nQuitLevel = nBlockLevel;
		}
		else if(token.Compare(L"if")==0){
			int beginindex = tindex+2;
			tindex = SkipBlock(paTokens, tindex+1);
			int endindex = tindex - 1;
			vector<CString*> temptokens;
			if(GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){
				int rettype;
				int* retptr = (int*)GetValue(&temptokens, &rettype);
				if(*retptr){		// ������ ���̶��					
					tindex++;
					beginindex = tindex;
					tindex = SkipStatement(paTokens, tindex);
					endindex = tindex;
					if(GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){	// ����� ���⿡ �����ߴٸ�
						pRetPtr = InterpretBlock(&temptokens, pRetType);
					}
					if(tindex+1 < (int)paTokens->size()){
						token = *((*paTokens)[tindex+1]);
						if(token.Compare(L"else")==0){		// �ڿ� else���� �����ϸ�
							tindex = SkipStatement(paTokens, tindex+2);		// ��ŵ
						}
					}
				}
				else{					// ������ �����̸�
					tindex = SkipStatement(paTokens, tindex + 1);		// ��ŵ
					if(tindex+1 < (int)paTokens->size()){
						token = *((*paTokens)[tindex+1]);
						if(token.Compare(L"else")==0){		// �ڿ� else���� �����ϸ�
							tindex+=2;
							beginindex = tindex;
							tindex = SkipStatement(paTokens, tindex);
							endindex = tindex;
							if(GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){	// ����� ���⿡ �����ߴٸ�
								pRetPtr = InterpretBlock(&temptokens, pRetType);
							}
						}
					}
				}
				delete retptr;
			}
		}
		else if(token.Compare(L"for")==0){			// for���̶��
			int beginindex;
			int endindex;
			vector<CString*> initstate;
			vector<CString*> conditionstate;
			vector<CString*> updatestate;
			vector<CString*> block;
			int rettype;
			void* retptr;
			int condition;


			beginindex = tindex+2;
			endindex = SkipStatement(paTokens, beginindex) - 1;
			GetSubPtrArray(&initstate, paTokens, beginindex, endindex);

			beginindex = endindex+2;
			endindex = SkipStatement(paTokens, beginindex) - 1;
			GetSubPtrArray(&conditionstate, paTokens, beginindex, endindex);

			beginindex = endindex+2;
			tindex = SkipBlock(paTokens, tindex + 1);
			endindex = tindex - 1;
			GetSubPtrArray(&updatestate, paTokens, beginindex, endindex);

			tindex++;
			beginindex = tindex;
			tindex = SkipStatement(paTokens, tindex);
			endindex = tindex;
			GetSubPtrArray(&block, paTokens, beginindex, endindex);			

			// �ʱ�ȭ
			retptr = GetValue(&initstate, &rettype);
			if(retptr) DeleteInstance(retptr, rettype); 
			stackLoopBlock.Insert(0, (TCHAR)nBlockLevel);
			nBlockLevel++;

			// �ʱ� ���ǰ˻�
			retptr = GetValue(&conditionstate, &rettype);
			if(retptr){
				condition = *(int*)retptr;
				DeleteInstance(retptr, rettype);
			}
			else condition = 0;

			while(condition){
				// ������
				pRetPtr = InterpretBlock(&block, pRetType);
				if(nQuitLevel > 0){
					nQuitLevel--;
					break;
				}

				// ������Ʈ
				retptr = GetValue(&updatestate, &rettype);
				if(retptr) DeleteInstance(retptr, rettype); ;

				// ���ǰ˻�
				retptr = GetValue(&conditionstate, &rettype);
				if(retptr){
					condition = *(int*)retptr;
					DeleteInstance(retptr, rettype);
				}
				else condition = 0;
			}
			nBlockLevel--;
			stackLoopBlock.Delete(0, 1);
		}
		else if(token.Compare(L"while")==0){			// while���̶��
			int beginindex;
			int endindex;
			vector<CString*> conditionstate;
			vector<CString*> block;
			int rettype;
			void* retptr;
			int condition;


			beginindex = tindex+2;
			tindex = SkipBlock(paTokens, tindex + 1);
			endindex = tindex - 1;
			GetSubPtrArray(&conditionstate, paTokens, beginindex, endindex);

			tindex++;
			beginindex = tindex;
			tindex = SkipStatement(paTokens, tindex);
			endindex = tindex;
			GetSubPtrArray(&block, paTokens, beginindex, endindex);			

			stackLoopBlock.Insert(0, (TCHAR)nBlockLevel);
			nBlockLevel++;

			// �ʱ� ���ǰ˻�
			retptr = GetValue(&conditionstate, &rettype);
			if(retptr){
				condition = *(int*)retptr;
				DeleteInstance(retptr, rettype);
			}
			else condition = 0;

			while(condition){
				// ������
				pRetPtr = InterpretBlock(&block, pRetType);
				if(nQuitLevel > 0){
					nQuitLevel--;
					break;
				}

				// ���ǰ˻�
				retptr = GetValue(&conditionstate, &rettype);
				if(retptr){
					condition = *(int*)retptr;
					DeleteInstance(retptr, rettype);
				}
				else condition = 0;
			}
			nBlockLevel--;
			stackLoopBlock.Delete(0, 1);
		}
		else if(token.Compare(L"do")==0){			// do-while���̶��
			int beginindex;
			int endindex;
			vector<CString*> conditionstate;
			vector<CString*> block;
			int rettype;
			void* retptr;
			int condition;

			tindex++;
			beginindex = tindex;
			tindex = SkipStatement(paTokens, tindex);
			endindex = tindex;
			GetSubPtrArray(&block, paTokens, beginindex, endindex);

			token = *((*paTokens)[++tindex]);
			if(token.Compare(L"while")) continue;
			tindex++;
			beginindex = tindex + 1;
			tindex = SkipBlock(paTokens, tindex);
			endindex = tindex - 1;
			GetSubPtrArray(&conditionstate, paTokens, beginindex, endindex);
			tindex++;	// �����ݷ� �پ�ѱ�

			stackLoopBlock.Insert(0, (TCHAR)nBlockLevel);
			nBlockLevel++;

			do{
				// ������
				pRetPtr = InterpretBlock(&block, pRetType);
				if(nQuitLevel > 0){
					nQuitLevel--;
					break;
				}

				// ���ǰ˻�
				retptr = GetValue(&conditionstate, &rettype);
				if(retptr){
					condition = *(int*)retptr;
					DeleteInstance(retptr, rettype);
				}
				else condition = 0;
			}while(condition);
			nBlockLevel--;
			stackLoopBlock.Delete(0, 1);
		}

		/////////////////////// �Ϲ� ���� (����, ����, �Լ�ȣ�� ��..) //////
		else{
			int beginindex = tindex;
			token=*((*paTokens)[tindex]);
			while(token.Compare(L";")){
				tindex++;
				token = *((*paTokens)[tindex]);
				if(token.Compare(L"(")==0 || token.Compare(L"{")==0 || token.Compare(L"[")==0)
				{
					tindex = SkipBlock(paTokens, tindex);
					if(tindex == -1) break;
				}
			}

			vector<CString*> temptks;
			if(GetSubPtrArray(&temptks, paTokens, beginindex, tindex-1)){
				int rettype;
				void* retptr = GetValue(&temptks, &rettype);
				if(retptr) DeleteInstance(retptr, rettype);
			}
		}

		if(nQuitLevel>0){
			nQuitLevel--;
			break;
		}
		tindex++;
	}

	if(!(m_bContinuousMode==true && nBlockLevel==1))
	{
		// ���� �������Ƿ� �� ������ �����߾��� ���������� ����	
		CVariable* ptr;
		int a_index;
		for(a_index = (int)m_aVariableTable.size()-1; a_index>=startSP; a_index--){
			ptr = (CVariable*)m_aVariableTable[a_index];
			if(ptr) delete ptr;
			m_aVariableTable.pop_back();
		}

		// ���� �������Ƿ� �� ������ �����߾��� ��ǵ� ����	
		CFunction* fptr;
		for(a_index = (int)m_aFunctionTable.size()-1; a_index>=startFP; a_index--){
			fptr = (CFunction*)m_aFunctionTable[a_index];
			if(fptr) delete fptr;
			m_aFunctionTable.pop_back();
		}
	}

	nBlockLevel--;

	return pRetPtr;
}


void* CTransScriptParser::GetValue(CString strSentence, int* pnRetType)
{
	void* pRetVal = NULL;
	vector<CString*> vTokens;

	/////////////////// ��ūȭ ///////////////////
	if(Tokenize(strSentence, &vTokens)==FALSE){
		*pnRetType = 0;
		return NULL;
		//AfxMessageBox(m_strLastError, MB_OK);
	}
	//int a = (int)vTokens.size();
	//ShowTokens(&m_aTokens);

	/////////////////// ó�� /////////////////////
	pRetVal = GetValue(&vTokens, pnRetType);

	/////////////// �پ� ������ ���� //////////////
	DeleteAllTokens(&vTokens);				// ��ū ��Ʈ������ �޸𸮿��� ����

	return pRetVal;
}


void* CTransScriptParser::GetValue(vector<CString*> *paTokens, int* pnRetType)
{
	void* pRetPtr = NULL;
	int tindex;
	CString token;
	int count = paTokens->size();
	*pnRetType = 0;	


	if(count==0){
		*pnRetType = 0;
		return pRetPtr;
	}
	else if(count==1){
		token = *((*paTokens)[0]);

		if('0' <= token[0] && token[0] <= '9'){	// ����������
			pRetPtr = new int;
			*((int*)pRetPtr) = atoi2(token);
			*pnRetType = 1;			
			return pRetPtr;
		}
		else if(token[0]==0x22){						// ���ڿ� ������
			pRetPtr = new CString();
			token = token.Mid(1, token.GetLength()-2);
			*(CString*)pRetPtr = token;
			*pnRetType = 2;
			return pRetPtr;
		}
		else{
			CVariable* var = GetVariable(token);
			if(var==NULL){
				m_strLastError = L"������� ���� �� �� ���� �ĺ����Դϴ� ";
				m_strLastError += token;
				//MessageBox(NULL, m_strLastError, L"��Ʈ��Ʈ ����", MB_OK);
				return NULL;
			}
			if(var->GetType()==1){
				pRetPtr = new int;
				*(int*)pRetPtr = *(int*)(var->GetValue());
			}
			else{
				pRetPtr = new CString();
				*(CString*)pRetPtr = *(CString*)(var->GetValue());
			}
			*pnRetType = var->GetType();
			return pRetPtr;
		}
	}
	else{
		// ���Ա�ȣ �˻� ����
		for(tindex=0; tindex<count; tindex++){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L"(")==0 || token.Compare(L"[")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"=")==0 || token.Compare(L"+=")==0 || token.Compare(L"-=")==0 || token.Compare(L"*=")==0 || token.Compare(L"/=")==0 || token.Compare(L"%=")==0){
				CVariable* var = NULL;
				CString lValurStart = *((*paTokens)[tindex-1]);
				if( lValurStart == L"]" )
				{
					int startindex = SkipBlock(paTokens, tindex-1);
					if(startindex<0) return pRetPtr;
					vector<CString*> temptks;
					if(!GetSubPtrArray(&temptks, paTokens, startindex+1, tindex-2)) return pRetPtr;
					
					int tmpRetType = 0;
					int nArrayIndex = *((int*)GetValue(&temptks, &tmpRetType));

					CVariable* pArray = GetVariable(*((*paTokens)[startindex-1]));
					if(!pArray) return pRetPtr;
					var = pArray->GetVariableElemant(nArrayIndex);
				}
				else
				{
					var = GetVariable(lValurStart);
				}

				if(var==NULL){
					m_strLastError = L"�߸��� �������Դϴ� ";
					m_strLastError += *((*paTokens)[tindex-1]);
					//MessageBox(NULL, m_strLastError, L"��ũ��Ʈ����", MB_OK);
					return NULL;
				}

				int r_type;
				vector<CString*> temptks;
				void* r_valuePtr;
				if(token.Compare(L"=")==0){		// �ܼ�����
					GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
					r_valuePtr = GetValue(&temptks, &r_type);
				}
				else{							// ������ ����
					CString varname = var->GetName();
					CString opentrace = L"(";
					CString closetrace = L")";
					CString op = token.Mid(0, 1);

					temptks.push_back(&varname);			// ������
					temptks.push_back(&op);					// ������
					temptks.push_back(&opentrace);			// '('
					for(size_t i=tindex+1; i < paTokens->size(); i++){
						temptks.push_back((*paTokens)[i]);
					}
					temptks.push_back(&closetrace);			// ')'
					r_valuePtr = GetValue(&temptks, &r_type);
				}



				// ����ȯ �� ���� ������ �ν��Ͻ� ����
				var->SetValue(RemakeValue(r_valuePtr, r_type, var->GetType()));
				// ���ϰ� ����
				pRetPtr = RemakeValue(r_valuePtr, r_type, var->GetType());
				*pnRetType = var->GetType();
				// ����� ���� ���ϰ� ����
				DeleteInstance(r_valuePtr, r_type);
				// ����
				return pRetPtr;
			}
		}

		// &&, || �������� �˻� ����
		for((int)tindex=paTokens->size()-1; tindex>=0; tindex--){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L")")==0 || token.Compare(L"]")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"&&")==0 || token.Compare(L"||")==0){
				int l_type, r_type;
				void *l_valuePtr, *r_valuePtr;
				vector<CString*> temptks;

				GetSubPtrArray(&temptks, paTokens, 0, tindex-1);
				l_valuePtr = GetValue(&temptks, &l_type);

				GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
				r_valuePtr = GetValue(&temptks, &r_type);

				// ���������� ĳ��Ʈ
				int* tempptr;

				tempptr = (int*)RemakeValue(l_valuePtr, l_type, 1);
				DeleteInstance(l_valuePtr, l_type);
				l_valuePtr = tempptr; l_type = 1;

				tempptr = (int*)RemakeValue(r_valuePtr, r_type, 1);
				DeleteInstance(r_valuePtr, r_type);
				r_valuePtr = tempptr; r_type = 1;

				*pnRetType = 1;
				pRetPtr = new int;
				*(int*)pRetPtr = 0;					
				if(token.Compare(L"&&")==0){
					if(*(int*)l_valuePtr && *(int*)r_valuePtr) *(int*)pRetPtr = 1;
				}
				else if(token.Compare(L"||")==0){
					if(*(int*)l_valuePtr || *(int*)r_valuePtr) *(int*)pRetPtr = 1;
				}

				// ����� ���� ���ϰ� ����
				DeleteInstance(r_valuePtr, r_type);
				DeleteInstance(l_valuePtr, l_type);
				// ����
				return pRetPtr;
			}		
		}

		// �񱳿����� �˻� ����
		for(tindex=paTokens->size()-1; tindex>=0; tindex--){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L")")==0 || token.Compare(L"]")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"==")==0 || token.Compare(L"!=")==0 || token.Compare(L"<")==0 || token.Compare(L">")==0 || token.Compare(L"<=")==0 || token.Compare(L">=")==0){
				int l_type, r_type;
				void *l_valuePtr, *r_valuePtr;
				vector<CString*> temptks;

				GetSubPtrArray(&temptks, paTokens, 0, tindex-1);
				l_valuePtr = GetValue(&temptks, &l_type);

				GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
				r_valuePtr = GetValue(&temptks, &r_type);

				if(l_type != r_type){		// ���� �� ���������� �ٸ��ٸ� ���������� ĳ��Ʈ
					void* tempptr;

					tempptr = RemakeValue(l_valuePtr, l_type, 1);
					DeleteInstance(l_valuePtr, l_type);
					l_valuePtr = tempptr; l_type = 1;

					tempptr = RemakeValue(r_valuePtr, r_type, 1);
					DeleteInstance(r_valuePtr, r_type);
					r_valuePtr = tempptr; r_type = 1;
				}

				*pnRetType = 1;					// �������� �����ϹǷ� ������
				pRetPtr = new int;				// ���ϵ� �񱳰��(�� 1, ���� 0)
				*(int*)pRetPtr = 0;
				if(l_type==1){				// ������
					if(token.Compare(L"==")==0){
						if(*(int*)l_valuePtr == *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L"!=")==0){
						if(*(int*)l_valuePtr != *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L"<")==0){
						if(*(int*)l_valuePtr < *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L">")==0){
						if(*(int*)l_valuePtr > *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L"<=")==0){
						if(*(int*)l_valuePtr <= *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L">=")==0){
						if(*(int*)l_valuePtr >= *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
				}
				else if(l_type==2 && r_type==2){			// ���ڿ���
					CString strltemp = *(CString*)l_valuePtr;
					CString strrtemp = *(CString*)r_valuePtr;
					if(token.Compare(L"==")==0){
						if(strltemp.Compare(strrtemp)==0) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L"!=")==0){
						if(strltemp.Compare(strrtemp)) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
				}


				// ����� ���� ���ϰ� ����
				DeleteInstance(r_valuePtr, r_type);
				DeleteInstance(l_valuePtr, l_type);
				// ����
				return pRetPtr;
			}		
		}

		// +,- ��������� �˻� ����
		for(tindex=paTokens->size()-1; tindex>=0; tindex--){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L")")==0 || token.Compare(L"]")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"+")==0 || token.Compare(L"-")==0){
				int l_type, r_type;
				void *l_valuePtr, *r_valuePtr;
				vector<CString*> temptks;

				GetSubPtrArray(&temptks, paTokens, 0, tindex-1);
				l_valuePtr = GetValue(&temptks, &l_type);

				GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
				r_valuePtr = GetValue(&temptks, &r_type);

				if(l_type != r_type){		// ������ �� ���������� �ٸ��ٸ�
					if(l_type==0 && r_type==1){		// ��ȣ�� ǥ���ϰ��� ���� void���̶�..
						l_valuePtr = new int;
						*(int*)l_valuePtr = 0;
						l_type = 1;
					}
					else{							// �� ���� ��� ��Ʈ������ ĳ��Ʈ
						void* tempptr;

						tempptr = RemakeValue(l_valuePtr, l_type, 2);
						DeleteInstance(l_valuePtr, l_type);
						l_valuePtr = tempptr; l_type = 2;

						tempptr = RemakeValue(r_valuePtr, r_type, 2);
						DeleteInstance(r_valuePtr, r_type);
						r_valuePtr = tempptr; r_type = 2;
					}
				}

				if(l_type==1){							// ��������
					*pnRetType = 1;
					pRetPtr = new int;
					*(int*)pRetPtr = 0;	
					int l_val = *(int*)l_valuePtr;
					int r_val = *(int*)r_valuePtr;
					if(token.Compare(L"+")==0)
						*(int*)pRetPtr = l_val + r_val;						
					else if(token.Compare(L"-")==0)
						*(int*)pRetPtr = l_val - r_val;						
				}
				else if(l_type==2 && r_type==2){		// ���ڿ�����
					*pnRetType = 2;
					pRetPtr = new CString;
					*(CString*)pRetPtr = L"";					
					CString strltemp = *(CString*)l_valuePtr;
					CString strrtemp = *(CString*)r_valuePtr;
					if(token.Compare(L"+")==0){
						*(CString*)pRetPtr = strltemp;
						*(CString*)pRetPtr += strrtemp;
					}
					else if(token.Compare(L"-")==0){
						//strltemp.Replace(strrtemp, "");
						*(CString*)pRetPtr = strltemp;
					}
				}


				// ����� ���� ���ϰ� ����
				DeleteInstance(r_valuePtr, r_type);
				DeleteInstance(l_valuePtr, l_type);
				// ����
				return pRetPtr;
			}		
		}

		// *, /, % ��������� �˻� ����
		for(tindex=paTokens->size()-1; tindex>=0; tindex--){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L")")==0 || token.Compare(L"]")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"*")==0 || token.Compare(L"/")==0 || token.Compare(L"%")==0){
				int l_type, r_type;
				void *l_valuePtr, *r_valuePtr;
				vector<CString*> temptks;

				GetSubPtrArray(&temptks, paTokens, 0, tindex-1);
				l_valuePtr = GetValue(&temptks, &l_type);

				GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
				r_valuePtr = GetValue(&temptks, &r_type);

				// ���������� ĳ��Ʈ
				void* tempptr;

				tempptr = RemakeValue(l_valuePtr, l_type, 1);
				DeleteInstance(l_valuePtr, l_type);
				l_valuePtr = tempptr; l_type = 1;

				tempptr = RemakeValue(r_valuePtr, r_type, 1);
				DeleteInstance(r_valuePtr, r_type);
				r_valuePtr = tempptr; r_type = 1;

				*pnRetType = 1;
				pRetPtr = new int;
				*(int*)pRetPtr = 0;					
				if(token.Compare(L"*")==0)
					*(int*)pRetPtr = *(int*)l_valuePtr * *(int*)r_valuePtr;
				else if(token.Compare(L"/")==0)
					*(int*)pRetPtr = *(int*)l_valuePtr / *(int*)r_valuePtr;
				else if(token.Compare(L"%")==0)
					*(int*)pRetPtr = *(int*)l_valuePtr % *(int*)r_valuePtr;


				// ����� ���� ���ϰ� ����
				DeleteInstance(r_valuePtr, r_type);
				DeleteInstance(l_valuePtr, l_type);
				// ����
				return pRetPtr;
			}		
		}

		// ++, -- ���׿����� �˻� ����
		for(tindex=0; tindex<count; tindex++){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L"(")==0 || token.Compare(L"[")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"++")==0 || token.Compare(L"--")==0){
				*pnRetType = 1;
				pRetPtr = new int;
				*(int*)pRetPtr = 0;					
				CVariable* var;
				int addval = 0;
				if(token.Compare(L"++")==0) addval = 1;
				else if(token.Compare(L"--")==0) addval = -1;

				if(tindex>0){
					var = GetVariable(*((*paTokens)[tindex-1]));
					if(var==NULL) return pRetPtr;
					*(int*)pRetPtr = *(int*)var->GetValue();
					*(int*)var->GetValue() += addval;
				}
				else{
					var = GetVariable(*((*paTokens)[tindex+1]));
					if(var==NULL) return pRetPtr;
					*(int*)var->GetValue() += addval;						
					*(int*)pRetPtr = *(int*)var->GetValue();
				}

				return pRetPtr;
			}		
		}

		// '!'��ȣ �˻�
		token = *((*paTokens)[0]);
		if(token.Compare(L"!")==0){
			vector<CString*> temptks;
			if(!GetSubPtrArray(&temptks, paTokens, 1, paTokens->size()-1)) return pRetPtr;
			int rettype;
			void* retptr = GetValue(&temptks, &rettype);
			pRetPtr = RemakeValue(retptr, rettype, 1);
			DeleteInstance(retptr, rettype);
			rettype = *(int*)pRetPtr;
			*(int*)pRetPtr = !rettype;
			*pnRetType = 1;
			return pRetPtr;			
		}

		// '(' �˻�
		token = *((*paTokens)[0]);
		if(token.Compare(L"(")==0){
			int endindex = SkipBlock(paTokens, 0);
			if(endindex<0) return pRetPtr;
			vector<CString*> temptks;
			if(!GetSubPtrArray(&temptks, paTokens, 1, endindex-1)) return pRetPtr;
			pRetPtr = GetValue(&temptks, pnRetType);
			return pRetPtr;
		}

		// '[' �˻�
		token = *((*paTokens)[0]);
		if(token.Compare(L"[")==0){
			int endindex = SkipBlock(paTokens, 0);
			if(endindex<0) return pRetPtr;
			vector<CString*> temptks;
			if(!GetSubPtrArray(&temptks, paTokens, 1, endindex-1)) return pRetPtr;

			int tmpRetType = 0;
			void* pRetValue = GetValue(&temptks, &tmpRetType);
			
			// �޸� �ּҸ� ����� ���Դٸ�
			if(NULL != pRetValue && tmpRetType == 1)
			{
				*pnRetType = 1;
				pRetPtr = new int;
				memcpy(pRetPtr, *(void**)pRetValue, 4);
			}

			DeleteInstance(pRetValue, tmpRetType);

			return pRetPtr;
		}

		// �迭�� �Ǵ� �Լ� ȣ�� �˻�
		//token = *(CString*)paTokens->ElementAt(0);
		//AfxMessageBox("�� �Լ�ȣ��˻�");
		CString funcname = *((*paTokens)[0]);
		token = *((*paTokens)[1]);
		if(token.Compare(L"[")==0){			// �迭���̶��
			int endindex = SkipBlock(paTokens, 1);
			if(endindex<0) return pRetPtr;
			vector<CString*> temptks;
			if(!GetSubPtrArray(&temptks, paTokens, 2, endindex-1)) return pRetPtr;
			
			int tmpRetType;
			void* pRetValue = GetValue(&temptks, &tmpRetType);
			int nArrayIndex = *((int*)pRetValue);
			DeleteInstance(pRetValue, tmpRetType);
			
			CVariable* pArray = GetVariable(funcname);
			if(!pArray) return pRetPtr;
			pRetValue = pArray->GetValue((vector<CString*>*)(UINT_PTR)nArrayIndex);
			if(pArray->GetType()==1)
			{
				pRetPtr = new int;
				*(int*)pRetPtr = *(int*)pRetValue;
			}
			else if(pArray->GetType()==2)
			{
				pRetPtr = new CString;
				*(CString*)pRetPtr = *(CString*)pRetValue;
			}

			*pnRetType = pArray->GetType();
		}		
		else if(token.Compare(L"(")==0){	// �Լ�ȣ���̶��
			vector<CString*> aVarList;
			tindex = 2;
			token = *((*paTokens)[2]);
			int beginindex = 2;
			int tsize = paTokens->size();
			while(tindex<tsize){
				if(token.Compare(L"(")==0 || token.Compare(L"[")==0){
					tindex = SkipBlock(paTokens, tindex);
				}
				else if(token.Compare(L",")==0 || token.Compare(L")")==0){
					vector<CString*> temptokens;
					if(GetSubPtrArray(&temptokens, paTokens, beginindex, tindex-1)){
						int rettype;
						void* retptr = GetValue(&temptokens, &rettype);
						aVarList.push_back((CString*)RemakeValue(retptr, rettype, 2));
						if(retptr) DeleteInstance(retptr, rettype);
					}
					if(token.Compare(L")")==0) break;
					beginindex = tindex + 1;
				}
				tindex++;
				token = *((*paTokens)[tindex]);
			}

			CFunction* func = GetFunction(funcname);
			if(func){	// ����� �����Լ� ȣ���
				int tmpBlockLevel = nBlockLevel;
				int tmpQuitLevel = nQuitLevel;
				CString tmpLoopBlock = stackLoopBlock;				
				vector<CVariable*> tmpVariableTable;					// ���� ����
				tmpVariableTable = m_aVariableTable;
				// ������������ �� ����
				vector<CVariable*>::iterator iter = m_aVariableTable.begin();
				while(iter != m_aVariableTable.end())
				{
					if((*iter)->GetBlockLevel() == 1)
					{
						iter++;
					}
					else
					{
						iter = m_aVariableTable.erase(iter);
					}

				}

				pRetPtr = func->GetValue(&aVarList);
				*pnRetType = func->GetType();

				m_aVariableTable.clear();				// ���� ����
				m_aVariableTable = tmpVariableTable;
				nBlockLevel = tmpBlockLevel;
				nQuitLevel = tmpQuitLevel;
				stackLoopBlock = tmpLoopBlock;
			}
			else
			{		// �⺻ �Լ� ȣ���
				//pRetPtr = m_pCallbackClass->FunctionCall(funcname, &aVarList, pnRetType);		// �Լ� ȣ��
			}

			DeleteAllTokens(&aVarList);							// �Ű������� �޸𸮿��� ����
			return pRetPtr;
		}
	}

	return pRetPtr;
}

BOOL CTransScriptParser::GetSubPtrArray(vector<CString*> *paTar, vector<CString*> *paSrc, int beginindex, int endindex)
{
	if((paSrc==NULL) || (paTar==NULL) || (endindex >= (int)paSrc->size()) || (endindex < beginindex)) return FALSE;

	paTar->clear();
	for(int i=beginindex; i<=endindex; i++){
		paTar->push_back((*paSrc)[i]);
	}

	return TRUE;
}

void* CTransScriptParser::RemakeValue(void *pSrc, int before, int after)
{
	void* retptr = NULL;

	if(after==0){			// void ������ ��ȯ�ؾ� �Ҷ�

	}
	else if(after==1){		// ���������� ��ȯ�ؾ��Ҷ�
		retptr = new int;
		if(before==0){
			*(int*)retptr = 0;
		}
		else if(before==1){
			*(int*)retptr = *(int*)pSrc;
		}
		else if(before==2){
			*(int*)retptr = _wtoi( *(CString*)pSrc );
		}
	}
	else if(after==2){		// ��Ʈ�������� ��ȯ�ؾ��Ҷ�
		retptr = new CString;
		if(before==0){
			*(CString*)retptr = L"";
		}
		else if(before==1){
			CString temp;
			wchar_t temp_buf[512];
			swprintf(temp_buf, L"%d", *(int*)pSrc);
			temp = temp_buf;
			*(CString*)retptr = temp;
		}
		else if(before==2){
			*(CString*)retptr = *(CString*)pSrc;		
		}
	}

	return retptr;
}

CVariable* CTransScriptParser::GetVariable(CString strVarName)
{
	int i, count;
	count = m_aVariableTable.size();
	CVariable* temp;
	for(i=0; i<count; i++){
		temp = (CVariable*)m_aVariableTable[i];
		if(strVarName.CompareNoCase(temp->GetName())==0) return temp;
	}
	return NULL;
}

int CTransScriptParser::SkipStatement(vector<CString*> *paTokens, int tindex)
{
	int maxindex = paTokens->size();
	CString token = *((*paTokens)[tindex]);
	if(token.Compare(L"{") == 0){			// �������̶��
		tindex = SkipBlock(paTokens, tindex);
	}
	else if(token.Compare(L"if") == 0){		// if���̶��
		tindex = SkipBlock(paTokens, tindex+1) + 1;
		tindex = SkipStatement(paTokens, tindex);
		if(tindex+1 < (int)paTokens->size()){
			token = *((*paTokens)[tindex+1]);
			if(token.Compare(L"else") == 0){		// �ڿ� else�� �������
				tindex = SkipStatement(paTokens, tindex+2);
			}
		}
	}
	else{									// �� ���ǿ� ��� ���� ���� ���
		while(tindex<maxindex){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L"(")==0) tindex = SkipBlock(paTokens, tindex);
			else if(token.Compare(L";")==0) break;
			tindex++;
		}
	}

	return tindex;
}


wchar_t* CTransScriptParser::AllocateBufferFromScriptFile(CString filename)
{
	// ��ũ��Ʈ ���� ���� (���� -> ����)
	FILE* fp = _wfopen(filename, L"rb");

	if(!fp)
	{
		MessageBoxA(NULL, "������ ��ũ��Ʈ ������ ã�� �� �����ϴ�.", "��ũ��Ʈ����", MB_OK);
		return NULL;
	}
	fseek(fp,0l,SEEK_END);

	int sourceSize = ftell(fp);

	char* pTempBuffer = new char[sourceSize+1];
	memset(pTempBuffer, 0, sourceSize+1);


	fseek(fp, 0l, SEEK_SET);
	fread(pTempBuffer, sizeof(char), sourceSize, fp);
	fclose(fp);

	wchar_t* pScriptBuffer = new wchar_t[sourceSize+1];
	memset(pScriptBuffer, 0, (sourceSize+1)*2);
	MultiByteToWideChar(CP_ACP, 0, pTempBuffer, sourceSize, pScriptBuffer, sourceSize );	
	delete [] pTempBuffer;

	return pScriptBuffer;
}

void CTransScriptParser::ExecuteScriptFile(LPCTSTR filename)
{
	// ��ũ��Ʈ ���� ����
	wchar_t* pScriptBuffer = AllocateBufferFromScriptFile(filename);
	if(pScriptBuffer==NULL) return;

	// ��ũ��Ʈ ����
	ExecuteScript(pScriptBuffer);

	// ��ũ��Ʈ ���� ����
	delete [] pScriptBuffer;
}

void CTransScriptParser::DeleteInstance(void *ptr, int type)
{
	if(type==1){
		int* ptr_int = (int*)ptr;
		if(ptr_int) {
			delete ptr_int;
			ptr_int = 0;
		}
	}
	else if(type==2){
		CString* ptr_str = (CString*)ptr;
		if(ptr_str) {
			delete ptr_str;
			ptr_str = 0;
		}
	}
	else{
		if(ptr) {
			delete ptr;
			ptr = 0;
		}
	}
}

// �տ� "0x"�� ���� 16���� ���ڿ��� ó�� �����ϰ�
int CTransScriptParser::atoi2(CString str)
{
	int nRetValue = 0;

	str = str.MakeUpper();

	if(str.GetLength()>2 && str[1]=='X'){
		wchar_t tmp;
		for(int strIdx=2; strIdx<str.GetLength(); strIdx++){
			nRetValue <<= 4;
			tmp = str[strIdx];
			if('0' <= tmp && tmp <= '9') nRetValue += (tmp-'0');
			else if('A' <= tmp && tmp <= 'F') nRetValue += (10 + tmp-'A');
		}
	}
	else{
		nRetValue = _wtoi(str);
	}

	return nRetValue;
}

void CTransScriptParser::DeleteAllTokens(vector<CString*> *pArray)
{
	int i, count;
	CString* ptrString;
	count = pArray->size();
	for(i=0; i<count; i++){
		ptrString = (*pArray)[i];
		delete ptrString;				// ��� ���ҵ��� �޸𸮿��� ����
	}
	pArray->clear();

}

void CTransScriptParser::DeleteAllIdentifiers(vector<CReplaceIdentifier*> *pArray)
{
	int i, count;
	CReplaceIdentifier* ptrString;
	count = pArray->size();
	for(i=0; i<count; i++){
		ptrString = (*pArray)[i];
		delete ptrString;				// ��� ���ҵ��� �޸𸮿��� ����
	}
	pArray->clear();

}

void CTransScriptParser::DeleteAllVariables(vector<CVariable*> *pArray)
{
	int i, count;
	CVariable* ptrString;
	count = pArray->size();
	for(i=0; i<count; i++){
		ptrString = (CVariable*)(*pArray)[i];
		delete ptrString;				// ��� ���ҵ��� �޸𸮿��� ����
	}
	pArray->clear();
}

void CTransScriptParser::IncludeScriptFile(CString filename, vector<CString*>* paTokens)
{
	// ��ũ��Ʈ ���� ���� (���� -> ����)
	FILE* fp = _wfopen(filename, L"rb");

	if(!fp)
	{
		MessageBoxA(NULL, "������ ��Ŭ��� ������ ã�� �� �����ϴ�. : ", "��ũ��Ʈ����", MB_OK);
		return;
	}
	fseek(fp,0l,SEEK_END);

	int sourceSize = ftell(fp);

	char* pTempBuffer = new char[sourceSize+1];
	memset(pTempBuffer, 0, sourceSize+1);


	fseek(fp, 0l, SEEK_SET);
	fread(pTempBuffer, sizeof(char), sourceSize, fp);
	fclose(fp);

	wchar_t* pScriptBuffer = new wchar_t[sourceSize+1];
	memset(pScriptBuffer, 0, (sourceSize+1)*2);
	MultiByteToWideChar(CP_ACP, 0, pTempBuffer, sourceSize, pScriptBuffer, sourceSize );	
	delete [] pTempBuffer;

	// ��ũ��Ʈ ����
	IncludeScript(pScriptBuffer, paTokens);

	// ��ũ��Ʈ ���� ����
	delete [] pScriptBuffer;

}

void CTransScriptParser::IncludeScript(wchar_t *buffer, vector<CString*>* paTokens)
{
	LPCTSTR backupPtr = m_pSourcePtr;
	m_pSourcePtr = buffer;

	/////////////////// ��ūȭ ///////////////////
	if(Tokenization(paTokens)==FALSE){
		//AfxMessageBox(m_strLastError, MB_OK);
	}
	//ShowTokens(&m_aTokens);

	m_pSourcePtr = backupPtr;

}



/* ========================= CVariable ====================================== */

void CVariable::SetBlockLevel(int level)
{
	m_nBlockLevel = level;
}

int CVariable::GetBlockLevel()
{
	return m_nBlockLevel;
}

void CVariable::SetValue(void *pValue)
{
	if(m_pValue) DeleteInstance();
	m_pValue = pValue;
}

void* CVariable::GetValue(vector<CString*>* pParams)
{
	if(m_bIsArray)	// �迭�̸�
	{
		int nArrayIndex = (int)(UINT_PTR)pParams;
		vector<CVariable*>* pVector = (vector<CVariable*>*)(UINT_PTR)m_pValue;
		if((int)pVector->size() <= nArrayIndex)
		{
			//MessageBoxA(NULL, "�迭 ÷���� ������ �Ѿ����ϴ�", "��ũ��Ʈ����", MB_OK);
		}
		CVariable* pVar = (*pVector)[nArrayIndex];
		return pVar->GetValue();
	}

	return m_pValue;
}

CVariable* CVariable::GetVariableElemant(int idx)
{
	if(m_bIsArray)	// �迭�̸�
	{
		CVariable* pVar = (*(vector<CVariable*>*)m_pValue)[idx];
		return pVar;
	}

	return NULL;
}

void CVariable::SetName(CString strName)
{
	m_strVarName = strName;
}

CString CVariable::GetName()
{
	return m_strVarName;
}

void CVariable::DeleteInstance()
{
	if(m_pValue){

		if(m_bIsArray){	// �迭�� ���
			vector<CVariable*>* pVector = (vector<CVariable*>*)m_pValue;

			vector<CVariable*>::iterator iter = pVector->begin();
			for(; iter != pVector->end(); iter++)
			{
				CVariable* pVar = (*iter);
				delete pVar;
			}

			delete pVector;
		}		

		else
		{
			if(m_nVarType==1){
				int* ptr_int = (int*)m_pValue;
				delete ptr_int;
			}
			else if(m_nVarType==2){
				CString* ptr_str = (CString*)m_pValue;
				delete ptr_str;
			}
			else{
				delete m_pValue;
			}
		}
	}
	m_pValue = NULL;
}

void CVariable::SetType(int nVarType)
{
	m_nVarType = nVarType;
}

int CVariable::GetType()
{
	return m_nVarType;
}


/* ========================= CFunction ====================================== */
CFunction::CFunction()
{
	CVariable::CVariable();
	m_pCodeTokens = NULL;
	m_pScriptContainer = NULL;
}

CFunction::~CFunction()
{

	// �ڵ���ū ���� (�� ��Ʈ������ delete�� CScript�Ҹ��ڰ� ���ֹǷ� ���⼱ ���ص� ��)
	if(m_pCodeTokens)
	{
		vector<CString*>::iterator iter = m_pCodeTokens->begin();
		for(; iter!=m_pCodeTokens->end(); iter++)
		{
			delete (*iter);
		}
		delete m_pCodeTokens;
	}

	// �Ű����� ����
	int i, count;
	CVariable* ptrString;
	count = m_aParams.size();

	for(i=0; i<count; i++){
		ptrString = (CVariable*)m_aParams[i];
		delete ptrString;				// ��� ���ҵ��� �޸𸮿��� ����
	}
	m_aParams.clear();

}

void CFunction::SetValue(void *pCodeTokens)
{
	m_pCodeTokens = (vector<CString*>*)pCodeTokens;
	vector<CString*>::iterator iter = m_pCodeTokens->begin();
	for(; iter!=m_pCodeTokens->end(); iter++)
	{
		CString* pTmpToken = new CString;
		*pTmpToken = *(*iter);
		(*iter) = pTmpToken;
	}
}

void* CFunction::GetValue(vector<CString*> *pParams)
{
	void* pRetPtr = NULL;

	if(m_pScriptContainer && m_pCodeTokens)
	{
		int startSP = m_pScriptContainer->m_aVariableTable.size();

		// �Ű����� ����
		int i, count;
		CVariable* pVar;
		count = m_aParams.size();
		if(count != pParams->size()) {
			//MessageBoxA(NULL, "����������Լ� ���� : �Ű����� ������ ��ġ���� �ʽ��ϴ�.", "��ũ��Ʈ����", MB_OK);
			return NULL;
		}

		for(i=0; i<count; i++){
			pVar = m_aParams[i];
			pVar->SetValue(RemakeValue((*pParams)[i], 2, pVar->GetType()));
			m_pScriptContainer->m_aVariableTable.push_back(pVar);
		}

		// ó��
		int   tempType;
		void* tempPtr = m_pScriptContainer->InterpretBlock(m_pCodeTokens, &tempType);
		pRetPtr = RemakeValue(tempPtr, tempType, m_nVarType);
		m_pScriptContainer->DeleteInstance(tempPtr, tempType);

		// �Ű����� ����
		for(int a_index = m_pScriptContainer->m_aVariableTable.size()-1; a_index>=startSP; a_index--){
			m_pScriptContainer->m_aVariableTable.pop_back();
		}

	}

	return pRetPtr;
}

void CTransScriptParser::DeleteAllFunctions(vector<CFunction*> *pArray)
{
	int i, count;
	CFunction* ptrString;
	count = pArray->size();
	for(i=0; i<count; i++){
		ptrString = (CFunction*)(*pArray)[i];
		delete ptrString;				// ��� ���ҵ��� �޸𸮿��� ����
	}
	pArray->clear();
}

CFunction* CTransScriptParser::GetFunction(CString strFuncName)
{
	int i, count;
	count = m_aFunctionTable.size();
	CFunction* temp;
	for(i=0; i<count; i++){
		temp = (CFunction*)m_aFunctionTable[i];
		if(strFuncName.Compare(temp->GetName())==0) return temp;
	}
	return NULL;
}

void CTransScriptParser::SetRegisterValues( REGISTER_ENTRY* pRegs )
{
	*(DWORD*)(GetVariable(_T("EAX"))->m_pValue) = pRegs->_EAX;
	*(DWORD*)(GetVariable(_T("EBX"))->m_pValue) = pRegs->_EBX;
	*(DWORD*)(GetVariable(_T("ECX"))->m_pValue) = pRegs->_ECX;
	*(DWORD*)(GetVariable(_T("EDX"))->m_pValue) = pRegs->_EDX;
	*(DWORD*)(GetVariable(_T("ESI"))->m_pValue) = pRegs->_ESI;
	*(DWORD*)(GetVariable(_T("EDI"))->m_pValue) = pRegs->_EDI;
	*(DWORD*)(GetVariable(_T("EBP"))->m_pValue) = pRegs->_EBP;
	*(DWORD*)(GetVariable(_T("ESP"))->m_pValue) = pRegs->_ESP;
}

void* CFunction::RemakeValue(void *pSrc, int before, int after)
{
	void* retptr = NULL;

	if(after==0){			// void ������ ��ȯ�ؾ� �Ҷ�

	}
	else if(after==1){		// ���������� ��ȯ�ؾ��Ҷ�
		retptr = new int;
		if(before==0){
			*(int*)retptr = 0;
		}
		else if(before==1){
			*(int*)retptr = *(int*)pSrc;
		}
		else if(before==2){
			*(int*)retptr = _wtoi( *(CString*)pSrc );
		}
	}
	else if(after==2){		// ��Ʈ�������� ��ȯ�ؾ��Ҷ�
		retptr = new CString;
		if(before==0){
			*(CString*)retptr = L"";
		}
		else if(before==1){
			CString temp;
			wchar_t temp_buf[512];
			swprintf(temp_buf, L"%d", *(int*)pSrc);
			temp = temp_buf;
			*(CString*)retptr = temp;
		}
		else if(before==2){
			*(CString*)retptr = *(CString*)pSrc;		
		}
	}

	return retptr;
}

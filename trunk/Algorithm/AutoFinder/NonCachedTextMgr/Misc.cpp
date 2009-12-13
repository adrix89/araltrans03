
#include "../stdafx.h"
#include "../AutoFinder.h"
#include "Misc.h"


BOOL IsJapaneseW(LPCWSTR wszJapaneseText, int nJapaneseLen)
{
	BOOL bRet=FALSE;
	int i;

	if (nJapaneseLen < 0) 
		nJapaneseLen=lstrlenW(wszJapaneseText);



	for(i=0; i<nJapaneseLen; i++)
	{
		if (
			((0x2E80 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x2EFF)) ||	// 2E80 - 2EFF ������ �μ� ����
			//((0x3000 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x303F)) ||	// 3000 - 303F ������ ��ȣ �� ������
			((0x31C0 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x31FF)) ||	// 31C0 - 31EF ������ ���� ȹ
			//																		// 31F0 - 31FF ��Ÿī�� ���� Ȯ��
			//((0x3200 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x32FF)) ||	// 3200 - 32FF ������ ��ȣ ����
			((0x3300 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x4DBF)) ||	// 3300 - 33FF ������ ȣȯ��
			//																		// 3400 - 4DBF ������ ���� ���� Ȯ��-A
			((0x4E00 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x9FBF)) ||	// 4E00 - 9FBF ������ ���� ����
			((0xF900 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0xFAFF)) ||	// FA00 - FAFF ������ ȣȯ�� ����
			((0xFE30 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0xFE4F)) ||	// FE30 - FE4F ������ ȣȯ �۲�
			((0xFF66 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0xFF9F))		// FF66 - FF9F �ݰ� ��Ÿī��
			)
		{
			bRet=TRUE;	// �ϴ� �Ͼ�
		}
		else if ((0x3040 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x30FF))	// 3040 - 309F ���󰡳�
			//																		// 30A0 - 30FF ��Ÿī��
		{
			bRet=TRUE;	// Ȯ���� �Ͼ�
			break;

		}
		else if ((0xAC00 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0xD7AF))	// AC00 - D7AF �ѱ� ����
		{
			bRet=FALSE;	// Ȯ���� �ѱ�
			break;
		}
	}

/*
	if (bRet)	// debug
	{
		FILE *fp;
		char szTemp[1024]={0, };
		fp=fopen("c:\\noncached.txt", "a");
		fprintf(fp, "[JAP] ");

		MyWideCharToMultiByte(CP_UTF8, 0, wszJapaneseText, nJapaneseLen, szTemp, 1023, 0, 0);
		fprintf(fp, "%s\n", szTemp);

		if (i == nJapaneseLen)
		{
			fprintf(fp, "(len=%d) ", i);
			for (i=0; i < nJapaneseLen; i++)
			{
				fprintf(fp, "%04X ", wszJapaneseText[i]);
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
	}
//*/
	return bRet;
}
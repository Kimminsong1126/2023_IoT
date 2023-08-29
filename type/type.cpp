#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <tchar.h> // for _tfpoen()
#include <windows.h>
#define BUF_SIZE 1024

void Type(TCHAR* filename)
{
	TCHAR stringbuff[BUF_SIZE];
	FILE* pFile = _tfopen(filename, _T("rt")); // 텍스트 모드로 file read
	while (!feof(pFile)) {
		_fgetts(stringbuff, BUF_SIZE, pFile);
		_fputts(stringbuff, stdout);
	}
}

int _tmain(int argc, TCHAR* argv[])
{
	if (argc < 2) {
		_fputts(_T("Usage: type xxxx.txt \n"), stdout);
		return -1;
	}
	Type(argv[1]);
	return 0;
}
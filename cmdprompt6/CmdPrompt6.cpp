#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <locale.h>
#include <tlhelp32.h>

#define STR_LEN 256
#define CMD_TOKEN_NUM 10

int CmdTokenize(void);
int   CmdProcessing(int);    // exit, echo hello world
int ListProcessInfo(void); // lp
void KillProcess(void); // kp
void start_op(int); // start, start echo test
void sort_op(void);  // sort, sort > xxx.txt
void TypeTextFile(void);



TCHAR cmdString[STR_LEN];
TCHAR cmdTokenList[CMD_TOKEN_NUM][STR_LEN];
TCHAR seps[] = _T(" , \t \n");

int _tmain(int argc, TCHAR* argv[])
{
    // �ѱ� �Է��� ������ �ϱ� ����
    _tsetlocale(LC_ALL, _T("Korean"));
    // ���α׷� main()�� �������ڰ� �ִ� ��� ���
    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            _tcscpy(cmdTokenList[i - 1], argv[i]);
        }
        // argc-1 => tokenNum, cmdprompt.exe ������ �ִ� string�� token ��ȣ 1
        CmdProcessing(argc - 1);
    }
    DWORD isExit = NULL;
    while (1)
    {
        int tokenNum = CmdTokenize();
        if (tokenNum == 0) // Enter �Է½� ó��
            continue;
        isExit = CmdProcessing(tokenNum);
        if (isExit == TRUE)
        {
            _fputts(_T("��ɾ� ó���� �����մϴ�. \n"), stdout);
            break;
        }
    }
    return 0;
}

// command prompt�� �ԷµǴ� string���� token(���� ���ڿ�)���� �и��ϴ� �۾�
int CmdTokenize()
{
    TCHAR* token;

    _fputts(_T("USW command >> "), stdout);
    _fgetts(cmdString, 256, stdin);

    token = _tcstok(cmdString, seps);

    int tokenNum = 0;
    while (token != NULL) {
        // ������ ���ڿ�(token)���� cmd list �迭�� ����
        // �빮�ڸ� �ҹ��ڷ� �ٲٴ� ����� StrLower()���� �⺻ �Լ���_tcslwr()�� ��ü
        _tcscpy(cmdTokenList[tokenNum++], _tcslwr(token));
        // cmdString���� NULL�� ���е� ���� ���ڿ� �ҷ�����
        token = _tcstok(NULL, seps);
    }
    return tokenNum;
}

int CmdProcessing(int tokenNum)
{
    TCHAR ERROR_CMD[] = _T("'%s'��(��) ������ �� �ִ� ���α׷��� �ƴմϴ�.\n");

    BOOL isRun;
    STARTUPINFO si = { 0, };
    PROCESS_INFORMATION pi;
    si.cb = sizeof(si);

    TCHAR cmdStringWithOptions[STR_LEN] = { 0, };
    TCHAR optString[STR_LEN] = { 0, };

    if (!_tcscmp(cmdTokenList[0], _T("exit"))) {
        return TRUE;
    }
    else if (!_tcscmp(cmdTokenList[0], _T("start"))) {
        start_op(tokenNum);
    }
    else if (!_tcscmp(cmdTokenList[0], _T("echo"))) {
        // �Էµ� ���ڿ��� �ϳ��� string���� ����, �״�� ���
        for (int i = 1; i < tokenNum; i++)
            _stprintf(optString, _T("%s %s"), optString, cmdTokenList[i]);
        _tprintf(_T("echo message : %s \n"), optString);
    }
    else if (!_tcscmp(cmdTokenList[0], _T("lp"))) {
        // list all processes currently running on the system
        int retval = ListProcessInfo();
        if (retval < 0)
            _tprintf(_T("ListProcessInfo() call error! \n"));
    }
    else if (!_tcscmp(cmdTokenList[0], _T("kp"))) {
        if (tokenNum > 1) {
            KillProcess();
        }
        else {
            // kp �� �Էµ� ���
            _tprintf(_T("usage : kp <process ID> \n"));
        }
    }
    else if (!_tcscmp(cmdTokenList[0], _T("sort"))) {
        sort_op();
    }
    else if (!_tcscmp(cmdTokenList[0], _T("uswtype"))) {
        TypeTextFile();
    }
    else
    {
        _tcscpy(cmdStringWithOptions, cmdTokenList[0]);
        for (int i = 1; i < tokenNum; i++) {
            _stprintf(cmdStringWithOptions, _T("%s %s"),
                cmdStringWithOptions, cmdTokenList[i]);
        }
        isRun = CreateProcess(NULL, cmdStringWithOptions, NULL, NULL,
            TRUE, 0, NULL, NULL, &si, &pi);
        if (isRun == FALSE)
            _tprintf(ERROR_CMD, cmdTokenList[0]);
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    for (int i = 0; i < tokenNum; i++) {
        *cmdTokenList[i] = { 0 };
    }
    return 0;
}
void start_op(int tokenNum)
{
    BOOL isRun;
    STARTUPINFO si = { 0, };
    PROCESS_INFORMATION pi;
    si.cb = sizeof(si);

    TCHAR cmdStringWithOptions[STR_LEN] = { 0, };
    TCHAR optString[STR_LEN] = { 0, };

    // start �� �߰� ���ڿ� ó�� (��: start echo Hello U of SW)
    if (tokenNum > 1) {
        for (int i = 1; i < tokenNum; i++)
            _stprintf(optString, _T("%s %s"), optString, cmdTokenList[i]);
        _stprintf(cmdStringWithOptions, _T("%s %s"), _T("cmdpromt6.exe"), optString);
    }
    else
        // start �� �Էµ� ���
        _stprintf(cmdStringWithOptions, _T("%s"), _T("cmdprompt6.exe"));
    isRun = CreateProcess(NULL, cmdStringWithOptions, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}


int ListProcessInfo()
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        _tprintf(_T("CreateToolhelp32Snapshot error! \n"));
        return -1;
    }

    // ���μ��� ���� ��� ���� ����ü ����
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        _tprintf(_T("Process32First error! \n"));
        CloseHandle(hProcessSnap);
        return -1;
    }
    do {
        _tprintf(_T("%25s %5d \n"), pe32.szExeFile,
            pe32.th32ProcessID);
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return 0;
}

void KillProcess(void)
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,
        0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        _tprintf(_T("CreatToolhelp32Snapshot error!\n"));
        return;
    }
    PROCESSENTRY32 pe32; // ���μ��� ���� ��� ���� ����ü ���� 
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        _tprintf(_T("Process32First error! \n"));
        CloseHandle(hProcessSnap);
        return;
    }
    HANDLE hProcess;
    BOOL isKill = FALSE;
    TCHAR* fileName;
    do {
        fileName = _tcslwr(pe32.szExeFile);
        if (_tcscmp(fileName, cmdTokenList[1]) == 0) {
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, -1);
                isKill = TRUE;
            }
            CloseHandle(hProcess);
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    if (isKill == FALSE)
        _tprintf(_T("Kill process failed, Try again! \n"));
}

void sort_op(void) {
    BOOL isRun;
    STARTUPINFO si = { 0, };
    PROCESS_INFORMATION pi;
    si.cb = sizeof(si);

    if (!_tcscmp(cmdTokenList[1], _T(">"))) {
        // cmdTokenList[2] -> e.g. sort.txt
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE hFile = CreateFile(cmdTokenList[2], GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        si.hStdOutput = hFile;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdError = hFile;
        si.dwFlags |= STARTF_USESTDHANDLES;

        // cmdTokenList[0] -> sort
        isRun = CreateProcess(NULL, cmdTokenList[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(hFile);
    }
    else {
        isRun = CreateProcess(NULL, cmdTokenList[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void TypeTextFile(void)
{
    TCHAR cmdStringWithOptions[STR_LEN] = { 0, };
    BOOL isRun;

    if (!_tcscmp(cmdTokenList[2], _T("|")))
    {
        // Create anonymous pipe

        _tprintf(_T("start TypeTextFile!!\n"));

        HANDLE hReadPipe, hWritePipe;
        SECURITY_ATTRIBUTES pipeSA = { sizeof(SECURITY_ATTRIBUTES),NULL, TRUE };

        CreatePipe(&hReadPipe, &hWritePipe, &pipeSA, 0);

        // process "uswtype"�� ���� ����
        STARTUPINFO siType = { 0, };
        PROCESS_INFORMATION piType;

        siType.cb = sizeof(siType);
        siType.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        siType.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        siType.hStdOutput = hWritePipe; // ��� �����̷��� (to pipe)
        siType.dwFlags |= STARTF_USESTDHANDLES;

        _tcscpy(cmdStringWithOptions, cmdTokenList[0]);
        _stprintf(cmdStringWithOptions, _T("% s % s"), cmdStringWithOptions, cmdTokenList[1]);

        isRun = CreateProcess(NULL, cmdStringWithOptions, NULL, NULL, TRUE, 0, NULL, NULL, &siType, &piType);

        CloseHandle(piType.hThread);
        CloseHandle(hWritePipe);

        // process "sort"�� ���� ����
        STARTUPINFO siSort = { 0, };
        PROCESS_INFORMATION piSort;

        if (!_tcscmp(cmdTokenList[4], _T(">"))) {
            // cmdTokenList[2] -> e.g. sort.txt
            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
            HANDLE hFile = CreateFile(cmdTokenList[5], GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            siSort.cb = sizeof(siSort);
            siSort.hStdInput = hReadPipe; // �Է� �����̷��� (to pipe)
            siSort.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            siSort.hStdOutput = hFile;
            siSort.dwFlags |= STARTF_USESTDHANDLES;
            isRun = CreateProcess(NULL, cmdTokenList[3], NULL, NULL, TRUE, 0, NULL, NULL, &siSort, &piSort);
        }

        CloseHandle(piSort.hThread);
        CloseHandle(hReadPipe);

        WaitForSingleObject(piType.hProcess, INFINITE);
        WaitForSingleObject(piSort.hProcess, INFINITE);

        CloseHandle(piType.hProcess);
        CloseHandle(piSort.hProcess);
    }
    else
    {
        _tcscpy(cmdStringWithOptions, cmdTokenList[0]);
        _stprintf(cmdStringWithOptions, _T("%s %s"),
            cmdStringWithOptions, cmdTokenList[1]);
        STARTUPINFO si = { 0, };
        PROCESS_INFORMATION pi;
        si.cb = sizeof(si);
        isRun = CreateProcess(NULL, cmdStringWithOptions, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}
#include "debug.hpp"


static FILE* s_pStdIn = nullptr;
static FILE* s_pStdOut = nullptr;
static FILE* s_pStdErr = nullptr;
static HWND s_hWndConsole = NULL;
static CRITICAL_SECTION s_cs;


static void SuspendAllThreadsExceptThis(void)
{
    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    DWORD CurrentProcessID = GetCurrentProcessId();
    DWORD CurrentThreadID = GetCurrentThreadId();
    THREADENTRY32 ThreadEntry = {};
    
    ThreadEntry.dwSize = sizeof(THREADENTRY32);

    Thread32First(hThreadSnapshot, &ThreadEntry);

    do
    {
        if (ThreadEntry.th32OwnerProcessID == CurrentProcessID &&
            ThreadEntry.th32ThreadID != CurrentThreadID)
        {
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, ThreadEntry.th32ThreadID);
            if (hThread != NULL)
            {
                SuspendThread(hThread);
                CloseHandle(hThread);
            };
        };
    } while (Thread32Next(hThreadSnapshot, &ThreadEntry));

    CloseHandle(hThreadSnapshot);
};


void DbgInit(void)
{
    AllocConsole();
    AttachConsole(GetCurrentProcessId());

    freopen_s(&s_pStdIn, "CON", "r", stdin);
    freopen_s(&s_pStdOut, "CON", "w", stdout);
    freopen_s(&s_pStdErr, "CON", "w", stderr);

    s_hWndConsole = GetConsoleWindow();

    InitializeCriticalSection(&s_cs);
};


void DbgTerm(void)
{
    DeleteCriticalSection(&s_cs);
    
    std::fclose(s_pStdIn);
    std::fclose(s_pStdOut);
    std::fclose(s_pStdErr);

    FreeConsole();
    PostMessage(s_hWndConsole, WM_CLOSE, 0, 0);

    s_pStdIn = s_pStdOut = s_pStdErr = nullptr;
    s_hWndConsole = NULL;
};


void DbgAssert(const char* expression, const char* fname, int32 fline)
{
#ifdef _DEBUG
    EnterCriticalSection(&s_cs);
    DbgAssert(expression, fname, fline, "Unknown error\n");
    LeaveCriticalSection(&s_cs);
#endif
};


void DbgAssert(const char* expression, const char* fname, int32 fline, const char* format, ...)
{
#ifdef _DEBUG
    EnterCriticalSection(&s_cs);
    SuspendAllThreadsExceptThis();
    
    static char buff[4096] = { 0 };

    int32 written = sprintf_s(buff, sizeof(buff),
        "Assertion!\n"
        "\n"
        "Expression: %s\n"
        "File: %s(%u)\n"
        "Description:\n",
        expression,
        fname,
        fline
    );

    if (written)
    {
        va_list vl;
        va_start(vl, format);
        written += vsprintf_s(buff + written, sizeof(buff) - written, format, vl);
        va_end(vl);
    };

    DbgOutput(buff);
    DbgFatal(buff);
    LeaveCriticalSection(&s_cs);
#endif
};


void DbgOutput(const char* format, ...)
{
    static char szOutputBuffer[4096];
    szOutputBuffer[0] = '\0';

    va_list vl;
    va_start(vl, format);
    std::vsprintf(szOutputBuffer, format, vl);
    va_end(vl);

    std::printf("%s", szOutputBuffer);
    OutputDebugStringA(szOutputBuffer);
};


void DbgFatal(const char* reason)
{
    static char szFatalBuffer[4096];
    szFatalBuffer[0] = '\0';

    std::sprintf(szFatalBuffer, "%s\n\n", reason);
    std::strcat(szFatalBuffer, "Press OK to execute debugbreak or CANCEL to terminate program.");

    int32 iResult = MessageBoxA(NULL, szFatalBuffer, "Fatal error", MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONERROR);
    if (iResult == IDOK)
        __debugbreak();

    TerminateProcess(GetCurrentProcess(), -1);
};
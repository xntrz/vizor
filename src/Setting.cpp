#include "Setting.hpp"
#include "File.hpp"


bool SettingIsPresent(const char* pszName)
{
	std::string str0(pszName);
	std::tstring str1(str0.begin(), str0.end());

	TCHAR** argv = __targv;
	int32 argc = __argc;

    for (int32 i = 0; i < argc; ++i)
    {
        const TCHAR* arg = argv[i];

        if (arg[0] == TEXT('-') && arg[1])        
		{
            if (!_tcscmp(&arg[1], &str1[0]))
                return true;
        };
    };

    return false;
};


bool SettingIsCheckSumShowCall(void)
{
    TCHAR** argv = __targv;
    int32 argc = __argc;

    if (argc != 2)
        return false;

    void* hFile = VzFOpen(argv[1], TEXT("rb"));
    if (hFile)
    {
        uint32 ChkSum = VzFChkSumm(hFile);
        VzFClose(hFile);
        hFile = nullptr;

        {
            AllocConsole();
            AttachConsole(GetCurrentProcessId());
            
            FILE* StdOut = nullptr;
            FILE* StdIn = nullptr;
            freopen_s(&StdOut, "CON", "w", stdout);
            freopen_s(&StdIn, "CON", "r", stdin);
            
            _tprintf(
                TEXT("Checksum of file %s is - %u (0x%X)\n"),
                argv[1],
                ChkSum,
                ChkSum
            );
            
            _tprintf(
                TEXT("Press ENTER to exit...\n")
            );

            std::getchar();
            std::fclose(StdIn);
            std::fclose(StdOut);     
            
            FreeConsole();
        };

        return true;
    };

    return false;
};
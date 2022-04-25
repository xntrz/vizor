#include "MmKit.hpp"
#include "Target.hpp"
#include "List.hpp"


struct MmKitHistoryData_t
{
    char ModuleName[64];
    uint64 Address;
    const int8* NewPayload;
    int32 NewPayloadLen;
    const int8* OldPayload;
    int32 OldPayloadLen;
};


struct MmKitHistoryCmd_t
{
    ListNode_t Node;    
    MmKitHistoryData_t Data;
};


struct MmKitHistory_t
{
    List_t List;
    int32 Size;
};


struct MmKit_t
{
    HANDLE ProcessHandle;
    uint32 ProcessID;
    uint32 ThreadID;
    int32 HistoryNumOpen;
    MmKitHistory_t* HistoryCurrent;
    bool ProcessingHistoryUndoFlag;
};


struct MmKit_t MmKit = {};


static
void
CtrlProcessRun(
    bool bSuspend
)
{
    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    THREADENTRY32 ThreadEntry;
    ThreadEntry.dwSize = sizeof(THREADENTRY32);

    Thread32First(hThreadSnapshot, &ThreadEntry);

    do
    {
        if (ThreadEntry.th32OwnerProcessID == MmKit.ProcessID)
        {
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, ThreadEntry.th32ThreadID);
            if (hThread != NULL)
            {
                if (bSuspend)
                    SuspendThread(hThread);
                else
                    ResumeThread(hThread);
                
                CloseHandle(hThread);
            };
        };
    } while (Thread32Next(hThreadSnapshot, &ThreadEntry));

    CloseHandle(hThreadSnapshot);
};


static
bool
GetModuleAddressAndSize(
    const char*     pszModuleName,
    uint64*         ResultAddress,
    uint64*         ResultSize
)
{
    HMODULE ahModule[1024] = {};
    uint32 cbRequired = 0;
    uint64 ModuleAddress = 0;
    uint64 ModuleSize = 0;

    //
    //  Перечисляем все модули указанного процесса
    //
    if (!EnumProcessModules(MmKit.ProcessHandle, ahModule, sizeof(ahModule), LPDWORD(&cbRequired)))
        return false;

    for (int32 i = 0; i < (cbRequired / sizeof(HMODULE)); ++i)
    {
        MODULEINFO ModuleInfo = {};
        char szBuffer[MAX_PATH] = {};

        if (!GetModuleFileNameExA(MmKit.ProcessHandle, ahModule[i], szBuffer, sizeof(szBuffer)))
            return false;

        char* pszName = std::strrchr(szBuffer, '\\');
        if (!pszName)
            return false;

        ++pszName;

        if (std::strcmp(pszName, pszModuleName))
            continue;

        if (!GetModuleInformation(MmKit.ProcessHandle, ahModule[i], &ModuleInfo, sizeof(ModuleInfo)))
            return false;

        ModuleAddress = uint64(ModuleInfo.lpBaseOfDll);
        ModuleSize = uint64(ModuleInfo.SizeOfImage);
        break;
    };

    *ResultAddress = ModuleAddress;
    *ResultSize = ModuleSize;
    
    return true;
};


static
bool
IsInRange(
    uint32 Value,
    uint32 Min,
    uint32 Max
)
{
    return ((Value >= Min) && (Value <= Max));
};


static
uint8
GetBits(
    int8   Value
)
{
    if (IsInRange(Value & ~0x20, 'A', 'F'))
    {
        return ((Value & ~0x20) - 'A' + 0xA);
    }
    else
    {
        if (IsInRange(Value, '0', '9'))
            return (Value - '0');
        else
            return 0;
    };
};


static
uint8
GetByte(
    const int8* Signature
)
{
    return (GetBits(Signature[0]) << 4 |
            GetBits(Signature[1]));
};


static
uint32
GetSignatureLen(
    const int8* Signature,
    int64       SignatureLen
)
{
    const int32 IntegerRoundUp = (sizeof(uint32) - 1);
    return uint32((SignatureLen + IntegerRoundUp) / 3);
};


static
uint32
ValidateSignature(
    const int8* Signature,
    uint32      SignatureLen
)
{
    return 1;
};


bool MmKitInitialize(void)
{    
    DWORD ProcessID = 0;
    DWORD ThreadID = GetWindowThreadProcessId(Target.WindowHandle, LPDWORD(&ProcessID));

    MmKit.ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
    if (!MmKit.ProcessHandle)
        return false;

    MmKit.ProcessID = ProcessID;
    MmKit.ThreadID = ThreadID;

    return true;
};


void MmKitTerminate(void)
{
    if (MmKit.ProcessHandle)
    {
        CloseHandle(MmKit.ProcessHandle);
        MmKit.ProcessHandle = NULL;
    };
};


void MmKitSuspendProcess(void)
{
    CtrlProcessRun(true);
};


void MmKitResumeProcess(void)
{
    CtrlProcessRun(false);
};


uint64
MmKitSearchModuleAddress(
    const char*     pszModuleName
)
{
    uint64 Address = 0;
    uint64 Size = 0;
    GetModuleAddressAndSize(pszModuleName, &Address, &Size);
    
    return Address;
};


uint64
MmKitSearchModuleSize(
    const char*     pszModuleName
)
{
    uint64 Address = 0;
    uint64 Size = 0;
    GetModuleAddressAndSize(pszModuleName, &Address, &Size);

    return Size;
};


uint64
MmKitPatchHistoryCreate(
    void
)
{
    MmKitHistory_t* History = new MmKitHistory_t();
    if (!History)
        return 0;

    ListInit(&History->List);
    History->Size = 0;

    return uint64(History);
};


void
MmKitPatchHistoryDestroy(
    uint64 hHist
)
{
    ASSERT(hHist);

    MmKitHistory_t* History = (MmKitHistory_t*)hHist;
    MmKitHistoryCmd_t* HistoryCmd = (MmKitHistoryCmd_t*)History->List.Head;

    while (HistoryCmd)
    {
        MmKitHistoryCmd_t* Next = (MmKitHistoryCmd_t*)HistoryCmd->Node.Next;
        
        ListRemove(&History->List, (ListNode_t*)HistoryCmd);
        
        if (HistoryCmd->Data.NewPayload)
            delete HistoryCmd->Data.NewPayload;

        if (HistoryCmd->Data.OldPayload)
            delete HistoryCmd->Data.OldPayload;
        
        delete HistoryCmd;
        
        HistoryCmd = Next;
    };

    delete History;
};


void
MmKitPatchHistoryUndo(
    uint64 hHist
)
{
    ASSERT(hHist);

    MmKit.ProcessingHistoryUndoFlag = true;

    MmKitHistory_t* History = (MmKitHistory_t*)hHist;
    MmKitHistoryCmd_t* HistoryCmd = (MmKitHistoryCmd_t*)History->List.Tail;

    while (HistoryCmd)
    {
        MmKitHistoryCmd_t* Prev = (MmKitHistoryCmd_t*)HistoryCmd->Node.Prev;

        MmKitPatchAddress(
            HistoryCmd->Data.ModuleName,
            HistoryCmd->Data.Address,
            HistoryCmd->Data.OldPayload,
            HistoryCmd->Data.OldPayloadLen
        );

        ListRemove(&History->List, (ListNode_t*)HistoryCmd);

        if (HistoryCmd->Data.NewPayload)
            delete HistoryCmd->Data.NewPayload;

        if (HistoryCmd->Data.OldPayload)
            delete HistoryCmd->Data.OldPayload;

        delete HistoryCmd;
        
        HistoryCmd = Prev;
    };

    MmKit.ProcessingHistoryUndoFlag = false;
};


void
MmKitPatchHistorySetCurrent(
    uint64 hHist
)
{
    MmKit.HistoryCurrent = (MmKitHistory_t*)hHist;
};


bool
MmKitPatchAddress(
    const char*     pszModuleName,
    uint64          Address,
    const int8*     Payload,
    int32           PayloadLen
)
{
    uint64 ModuleAddress = 0;
    uint64 ModuleSize = 0;

    if (!GetModuleAddressAndSize(pszModuleName, &ModuleAddress, &ModuleSize))
        return false;

    DWORD OldProtect = 0;
    if (VirtualProtectEx(MmKit.ProcessHandle, LPVOID(ModuleAddress + Address), PayloadLen, PAGE_EXECUTE_READWRITE, &OldProtect))
    {
        if (MmKit.HistoryCurrent && !MmKit.ProcessingHistoryUndoFlag)
        {
            MmKitHistoryCmd_t* HistoryCmd = new MmKitHistoryCmd_t();
            std::strcpy(HistoryCmd->Data.ModuleName, pszModuleName);
            HistoryCmd->Data.Address = Address;
            HistoryCmd->Data.NewPayload = new int8[PayloadLen];
            HistoryCmd->Data.NewPayloadLen = PayloadLen;
            HistoryCmd->Data.OldPayload = new int8[PayloadLen];
            HistoryCmd->Data.OldPayloadLen = PayloadLen;
            std::memcpy((void*)HistoryCmd->Data.NewPayload, Payload, PayloadLen);
            MmKitReadProcessMem(ModuleAddress + Address, HistoryCmd->Data.OldPayload, HistoryCmd->Data.OldPayloadLen);            

            ++MmKit.HistoryCurrent->Size;
            ListInsert(&MmKit.HistoryCurrent->List, (ListNode_t*)HistoryCmd);
        };

        MmKitWriteProcessMem(ModuleAddress + Address, Payload, PayloadLen);
        VirtualProtectEx(MmKit.ProcessHandle, LPVOID(ModuleAddress + Address), PayloadLen, OldProtect, NULL);
    };

    return true;
};


int32
MmKitSearchSignature(
    const int8*     Buffer,
    int64           BufferSize,
    const int8*     Signature,
    int64           SignatureLen,
    uint64*         ResultAddress,
    uint64*         ResultLen
)
{
    int32 Result = 0;
    uint32 AddressFlag = 0;
    uint64 Address = 0;
    uint64 Len = 0;
    const int8* Pattern = Signature;
    uint32 LenTest = GetSignatureLen(Signature, SignatureLen);
    thread_local static const int8* SignatureLastPos = nullptr;
    thread_local static const int8* SignatureLastPtr = nullptr;
	thread_local static uint64 AddressLast = 0;
	thread_local static uint64 LenLast = 0;
	thread_local static uint64 SameCall = 0;

    //
    //	Если текущий вызов это продолжение
    //
	if (SignatureLastPtr == Signature)
	{
        Pattern = SignatureLastPos;
		Address = AddressLast;
		Len = LenLast;
		++SameCall;
	}
	else
	{
		SameCall = 0;
	};

    //
    //	Проверка буферов
    //
    if (!Buffer || !Signature)
    {
        *ResultAddress = Address;
        *ResultLen = Len;
        return Result;
    };

    //
    //	Проверка размеров
    //
    if (BufferSize < 1 || SignatureLen < 2)
    {
        *ResultAddress = Address;
        *ResultLen = Len;
        return Result;
    };

    for (int32 i = 0; i < BufferSize; ++i)
    {
        uint8 Byte = Buffer[i];

        if (*Pattern == '\?' || Byte == GetByte(Pattern))
        {
            //
            //	Проверям первое вхождение
            //
            if (!AddressFlag)
            {
                AddressFlag = 1;
                Address = i;
            };

            //
            //	Увеличиваем длину успешных проверок паттерна
            //
            ++Len;

            //
            //	Паттерн закончился, а буфер нет
            //
            if (&Pattern[2] >= (Signature + SignatureLen))
            {
                AddressFlag = 0;
				Address = (SameCall - Len) + 1;
                Result = 1;
                break;
            }
            else
            {
                if (Pattern[2])
                {
                    if (*(uint16*)Pattern == '\?\?' ||
                        *(uint8*)Pattern != '\?')
                        Pattern += 3;
                    else
                        Pattern += 2;
                };
            };
        }
        else
        {
            Pattern = Signature;
            Address = 0;
            AddressFlag = 0;
            Len = 0;
        };
    };

    //
    //	Буфер закончился, а паттерн нет
    //
    if (AddressFlag)
        Result = 2;

    //
    //  Устанавливаем результат
    //
    *ResultLen = Len;
    *ResultAddress = Address;

    //
    //	Сохраним для продолжения если необходимо
    //
    SignatureLastPos = Pattern;
    SignatureLastPtr = Signature;
	AddressLast = Address;
	LenLast = Len;

    return Result;
};


int32
MmKitReadProcessMem(
    uint64          Address,
    const int8*     Buffer,
    int64           BufferSize
)
{
    SIZE_T Readed = 0;
    
    if (!ReadProcessMemory(MmKit.ProcessHandle, LPCVOID(Address), LPVOID(Buffer), BufferSize, &Readed))
        OUTPUT("%s failed with error code: %u\n", __FUNCTION__, GetLastError());
    
    return (Readed > 0);
};


int32
MmKitWriteProcessMem(
    uint64          Address,
    const int8*     Buffer,
    int64           BufferSize
)
{
    SIZE_T Written = 0;
    
    if (!WriteProcessMemory(MmKit.ProcessHandle, LPVOID(Address), LPVOID(Buffer), BufferSize, &Written))
        OUTPUT("%s failed with error code: %u\n", __FUNCTION__, GetLastError());
    
    return (Written > 0);
};
#pragma once


bool MmKitInitialize(void);
void MmKitTerminate(void);
void MmKitSuspendProcess(void);
void MmKitResumeProcess(void);
uint64 MmKitPatchHistoryCreate(void);
void MmKitPatchHistoryDestroy(uint64 hHist);
void MmKitPatchHistoryUndo(uint64 hHist);
void MmKitPatchHistorySetCurrent(uint64 hHist);
bool MmKitPatchAddress(const char* pszModuleName, uint64 Address, const int8* Payload, int32 PayloadLen);

int32 MmKitSearchSignature(
    const int8* Buffer,
    int64 BufferSize,
    const int8* Signature,
    int64 SignatureLen,
    uint64* ResultAddress,
    uint64* ResultLen
);

int32 MmKitReadProcessMem(
    uint64 Address,
    const int8* Buffer,
    int64 BufferSize
);

int32 MmKitWriteProcessMem(
    uint64 Address,
    const int8* Buffer,
    int64 BufferSize
);

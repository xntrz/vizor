#pragma once


enum VzFSeek_t
{
    VzFSeek_Begin = 0,
    VzFSeek_Current,
    VzFSeek_End,
};


void* VzFOpen(const TCHAR* pszFilepath, const TCHAR* pszMode);
void VzFClose(void* hFile);
int32 VzFRead(void* hFile, int8* Buffer, int32 BufferSize);
int32 VzFWrite(void* hFile, int8* Buffer, int32 BufferSize);
int32 VzFTell(void* hFile);
void VzFSeek(void* hFile, VzFSeek_t seek, int32 offset = 0);
int32 VzFSize(void* hFile);
uint32 VzFChkSumm(void* hFile);
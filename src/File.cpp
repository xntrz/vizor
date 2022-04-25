#include "File.hpp"


struct VzFile_t
{
    FILE* Handle;
    TCHAR Path[MAX_PATH];
};


void* VzFOpen(const TCHAR* pszFilepath, const TCHAR* pszMode)
{
    VzFile_t* File = new VzFile_t();

    File->Handle = _tfopen(pszFilepath, pszMode);
    if (!File->Handle)
    {
        delete File;
        File = nullptr;
    }
	else
	{
		_tcscpy(File->Path, pszFilepath);
	};
        
    return File;
};


void VzFClose(void* hFile)
{
    VzFile_t* File = (VzFile_t*)hFile;
    if (File->Handle)
    {
        fclose(File->Handle);
        File->Handle = nullptr;
    };
    
    delete File;
};


int32 VzFRead(void* hFile, int8* Buffer, int32 BufferSize)
{
    VzFile_t* File = (VzFile_t*)hFile;
    ASSERT(File->Handle);
    return int32(fread(Buffer, sizeof(uint8), BufferSize, File->Handle));
};


int32 VzFWrite(void* hFile, int8* Buffer, int32 BufferSize)
{
    VzFile_t* File = (VzFile_t*)hFile;
    ASSERT(File->Handle);
    return int32(fwrite(Buffer, sizeof(uint8), BufferSize, File->Handle));
};


int32 VzFTell(void* hFile)
{
    VzFile_t* File = (VzFile_t*)hFile;
    ASSERT(File->Handle);
    return ftell(File->Handle);
};


void VzFSeek(void* hFile, VzFSeek_t seek, int32 offset)
{
    VzFile_t* File = (VzFile_t*)hFile;
    ASSERT(File->Handle);

    int32 fileseek = 0;
    switch (seek)
    {
    case VzFSeek_Begin:
        fileseek = SEEK_SET;
        break;

    case VzFSeek_Current:
        fileseek = SEEK_CUR;
        break;

    case VzFSeek_End:
        fileseek = SEEK_END;
        break;
    };

    fseek(File->Handle, offset, fileseek);
};


int32 VzFSize(void* hFile)
{
    int32 PrevFilePointer = VzFTell(hFile);    
    VzFSeek(hFile, VzFSeek_End);
    int32 Result = VzFTell(hFile);
    VzFSeek(hFile, VzFSeek_Begin, PrevFilePointer);
    return Result;
};


uint32 VzFChkSumm(void* hFile)
{
    uint32 Result = 0;
    int32 PrevFilePointer = VzFTell(hFile);
    char FileBuffer[4096];
    int32 FileSize = VzFSize(hFile);
    int32 FileOffset = 0;
    
    while (FileOffset != FileSize)
    {
        VzFSeek(hFile, VzFSeek_Begin, FileOffset);
        
        int32 Readed = VzFRead(
            hFile,
            FileBuffer,
            Min(FileSize - FileOffset, int32(sizeof(FileBuffer)))
        );

        for (int32 i = 0; i < Readed; ++i)
            Result += uint8(FileBuffer[i]);

        FileOffset += Min(FileSize - FileOffset, int32(sizeof(FileBuffer)));
    };

    VzFSeek(hFile, VzFSeek_Begin, PrevFilePointer);

    return Result;
};
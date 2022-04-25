#pragma once


void DbgInit(void);
void DbgTerm(void);
void DbgAssert(const char* expression, const char* fname, int32 fline);
void DbgAssert(const char* expression, const char* fname, int32 fline, const char* format, ...);
void DbgOutput(const char* format, ...);
void DbgFatal(const char* reason);


#ifdef ASSERT
#undef ASSERT
#endif

#if defined (_DEBUG)

#ifdef _DEBUG
#define DBGBREAK            \
    do                      \
    {                       \
        __asm{ int 0x3 };   \
    } while (0)
#else
#define DBGBREAK
#endif

#define ASSERT(expression, ...)                                             \
    do                                                                      \
    {                                                                       \
        if (!(expression))                                                  \
        {                                                                   \
	        DbgAssert(#expression, __FILE__, __LINE__, ##__VA_ARGS__);      \
        };                                                                  \
    } while(0)

#define OUTPUT(format, ...)     DbgOutput(format, ##__VA_ARGS__)

#else

#define DBGBREAK

#define ASSERT(expression, ...) ((void)0)

#define OUTPUT(format, ...)     ((void)0)

#endif
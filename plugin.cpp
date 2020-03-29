// 网上找的源码，侵删

///////////////////////////////////////////////////
// 文    件: FindProcessModuleAddress.c
// 描    述: 获取指定进程指定模块基址
// 作    者: yemoon
// 使用方法: FindProcessModuleAddress 进程名 模块名
// 创建时间: 2017/06/06 23:05:21
// 备    注:
//
//////////////////////////////////////////////////
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <iostream>
#include <locale>
using namespace std;

#define YSUCCESS 0
#define YERROR -1
typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
////////////////////////////////////////////////////////////////////
// copy from msdn
typedef struct _LSA_UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef enum _MEMORY_INFORMATION_CLASS
{
    MemoryBasicInformation,
    MemoryWorkingSetList,
    MemorySectionName
} MEMORY_INFORMATION_CLASS;
//typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
typedef NTSTATUS(WINAPI *ZwQueryVirtualMemoryfn)(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress,
    _In_ MEMORY_INFORMATION_CLASS MemoryInformationClass,
    _Out_ PVOID MemoryInformation,
    _In_ SIZE_T MemoryInformationLength,
    _Out_opt_ PSIZE_T ReturnLength);
////////////////////////////////////////////////////////////////////
typedef struct
{
    UNICODE_STRING SectionFileName;
    WCHAR NameBuffer[MAX_PATH * 2 + 2];
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;

static ZwQueryVirtualMemoryfn g_ZwQueryVirtualMemoryPtr = NULL;
///////////////////////////////////////////////////
// 函    数: YZwQueryVirtualMemory
// 描    述: ZwQueryVirtualMemory 代{过}{滤}理函数
// 参    数:
//    [url]https://msdn.microsoft.com/en-us/library/windows/hardware/dn957455[/url](v=vs.85).aspx
// 返 回 值:
//    int -- 成功返回 YSUCCESS
//           失败返回 YERROR
// 作    者: yemoon
// 创建时间: 2017/06/06 23:39:04
// 备    注:
//
///////////////////////////////////////////////////
static int WINAPI YZwQueryVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, ULONG MemoryInformationLength)
{
    NTSTATUS status = 0;

    if (g_ZwQueryVirtualMemoryPtr == NULL)
    {
        g_ZwQueryVirtualMemoryPtr = (ZwQueryVirtualMemoryfn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "ZwQueryVirtualMemory");
        if (g_ZwQueryVirtualMemoryPtr == NULL)
        {
            return YERROR;
        }
    }
    status = g_ZwQueryVirtualMemoryPtr(ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, NULL);
    if (status >= 0)
        return YSUCCESS;
    return YERROR;
}

///////////////////////////////////////////////////
// 函    数: tofilename
// 描    述: 指向UNICODE路径字符串中文件名部分
// 参    数:
//    wchar_t *path -- 待搜索的路径字符串(unicode)
// 返 回 值:
//    wchar_t * -- 指向待搜索的路径字符串中文件名部分
// 作    者: yemoon
// 创建时间: 2017/06/06 23:47:16
// 备    注:
//
///////////////////////////////////////////////////
static wchar_t *tofilename(wchar_t *path)
{
    wchar_t *p = wcsrchr(path, L'\\');
    if (p)
        return ++p;
    else
        return path;
}

///////////////////////////////////////////////////
// 函    数: enum_process_module
// 描    述: 获取指定进程指定模块基址
// 参    数:
//    unsigned int pid -- 进程PID
//    wchar_t *modulename -- 模块名
// 返 回 值:
//    unsigned long -- 模块地址   0 : 表示获取失败
// 作    者: yemoon
// 创建时间: 2017/06/06 23:32:11
// 备    注:
//
///////////////////////////////////////////////////
static unsigned long enum_process_module(unsigned int pid, wchar_t *modulename)
{
    HANDLE hProcess;
    unsigned long queryaddr = 0;
    MEMORY_BASIC_INFORMATION mbi;
    MEMORY_SECTION_NAME SectionName;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION + PROCESS_VM_READ, TRUE, pid);
    if (hProcess == NULL)
    {
        wprintf(L"[-]OpenProcess %d with error %u\n", pid, GetLastError());
        return 0;
    }

    while (queryaddr < 0x80000000)
    {
        if (YZwQueryVirtualMemory(hProcess, (PVOID)queryaddr, MemoryBasicInformation, &mbi, sizeof(mbi)) == YSUCCESS)
        {
            if (mbi.Type == MEM_IMAGE)
            {
                if (YZwQueryVirtualMemory(hProcess, (PVOID)queryaddr, MemorySectionName, &SectionName, sizeof(SectionName) - 2) == YSUCCESS)
                {
                    SectionName.SectionFileName.Buffer[SectionName.SectionFileName.Length] = L'\0';
                    //wprintf(L"[0x%.8x]\t%s\n", queryaddr, tofilename(SectionName.SectionFileName.Buffer));
                    if (wcsicmp(tofilename(SectionName.SectionFileName.Buffer), modulename) == 0)
                    {
                        CloseHandle(hProcess);
                        return queryaddr;
                    }
                }
            }
            queryaddr += mbi.RegionSize;
        }
        else
        {
            queryaddr += 0x1000;
        }
    }
    CloseHandle(hProcess);
    return 0;
}

char *WcharToChar(const wchar_t *wp)
{
    char *m_char;
    int len = WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
    m_char = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
    m_char[len] = '\0';
    return m_char;
}
wchar_t *CharToWchar(const char *c)
{
    wchar_t *m_wchar;
    int len = MultiByteToWideChar(CP_ACP, 0, c, strlen(c), NULL, 0);
    m_wchar = new wchar_t[len + 1];
    MultiByteToWideChar(CP_ACP, 0, c, strlen(c), m_wchar, len);
    m_wchar[len] = '\0';
    return m_wchar;
}

///////////////////////////////////////////////////
// 函    数: enum_process
// 描    述: 查找指定进程
// 参    数:
//    wchar_t *processname -- 进程名
//    wchar_t *modulename -- 模块名
// 返 回 值: 无
// 作    者: yemoon
// 创建时间: 2017/06/06 23:19:53
// 备    注:
//
///////////////////////////////////////////////////
void enum_process(wchar_t *processname, wchar_t *modulename, DWORD &ret1, unsigned int &ret2)
{

    HANDLE hSnap;
    PROCESSENTRY32 pe32 = {0};
    unsigned long addr = 0;

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
    {
        wprintf(L"[-]CreateToolhelp32Snapshot with error %u\n", GetLastError());
        return;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnap, &pe32))
    {
        do
        {
            wchar_t *temp = tofilename(pe32.szExeFile);
            char *temp2 = WcharToChar(temp);
            char *temp3 = WcharToChar(processname);
            //cout << temp2 << endl;
            //cout << temp2 << " vs " << temp3 << endl;
            if (wcsicmp(temp, processname) == 0)
            {
                cout << "successfully get into process" << endl;
                addr = enum_process_module(pe32.th32ProcessID, modulename);
                if (addr != 0)
                {
                    wprintf(L"%u:%s:0x%.8x\n", pe32.th32ProcessID, WcharToChar(modulename), addr);
                    ret1 = pe32.th32ProcessID;
                    ret2 = addr;
                }
            }

        } while (Process32Next(hSnap, &pe32));
    }
    CloseHandle(hSnap);
}

///////////////////////////////////////////////////
// 函    数: enable_debug_priv
// 描    述: 提升进程权限
// 参    数: 无
// 返 回 值:
//    int -- 成功返回 YSUCCESS
//           失败返回 YERROR
// 作    者: yemoon
// 创建时间: 2017/06/06 23:13:21
// 备    注:
//
///////////////////////////////////////////////////
static int enable_debug_priv()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    LUID Luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        wprintf(L"[-] OpenProcessToken error with %u\n", GetLastError());
        return YERROR;
    }

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid))
    {
        wprintf(L"[-] LookupPrivilegeValue error with %u\n", GetLastError());
        CloseHandle(hToken);
        return YERROR;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = Luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
    {
        wprintf(L"[-] AdjustTokenPrivileges error with %u\n", GetLastError());
        CloseHandle(hToken);
        return YERROR;
    }
    return YSUCCESS;
}

void plugin_get(DWORD &processid, unsigned int &address_base)
{
    //if (argc != 3)
    //{
    //    wprintf(L"Usage:\n\tFindProcessModuleAddress\tprocessname\tmodulename\n");
    //    return 0;
    //}
    // 提权先
    if (enable_debug_priv() != YSUCCESS)
    {
        cout << "error_enable_debug" << endl;
        return;
    }
    //cout << argv[1] <<endl;
    //wchar_t *a = CharToWchar(argv[1]);
    //wchar_t *b = CharToWchar(argv[2]);
    //std::wcout.imbue(std::locale("chs"));
    //locale::global(std::locale("CHS"));
    //wcout << argv[1] << " " << argv[2] << endl;
    //DWORD processid;
    //unsigned int address_base;
    enum_process(CharToWchar("frame.exe"), CharToWchar("ChatClient.dll"), processid, address_base);
    //cout << "error_result" << endl;
}

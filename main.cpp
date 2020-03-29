#pragma once
#include <iostream>
#include <windows.h>
#include <Tlhelp32.h>
#include <string>
#include <map>
using namespace std;

std::map<int, int> read_map = {
    {5, 0},   //司
    {6, 1},   //军
    {7, 2},   //师
    {8, 3},   //旅
    {9, 4},   //团
    {10, 7},  //营
    {11, 8},  //连
    {12, 9},  //排
    {13, 11}, //兵
    {3, 6},   //雷
    {4, 5},   //炸
    {2, 10},  //旗
};
typedef unsigned char byte;

std::string WcharToChar(const wchar_t *wp, size_t m_encode = CP_ACP)
{
    std::string str;
    int len = WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
    char *m_char = new char[len + 1];
    WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
    m_char[len] = '\0';
    str = m_char;
    delete m_char;
    return str;
}

int toInt(unsigned char *bytes) //byte
{
    int number = 0;
    for (int i = 0; i < 4; i++)
    {
        number += bytes[i] << i * 8;
    }
    return number;
}

int GetModAdd(DWORD PID, char *ModName)
{
    MODULEENTRY32 mo;
    HANDLE LM;
    LM = CreateToolhelp32Snapshot(0x8, PID);
    if (LM > 0)
    {
        mo.dwSize = sizeof(mo);
        if (Module32First(LM, &mo))
        {
            do
            {
                char output[256];
                sprintf(output, "%s", mo.szModule);
                cout << output << endl;
                if (strcmp(output, ModName) == 0)
                {
                    CloseHandle(LM);
                    return toInt(mo.modBaseAddr);
                }
            } while (Module32Next(LM, &mo) != 0);
        }
    }
    return 0;
}

int init(int base, HANDLE &processh)
{
    int temp = 0;
    base += 0x3F014; //0x3EE38;
    cout << base << endl;
    if (ReadProcessMemory(processh, (LPVOID)base, &temp, 4, NULL))
    {
        cout << temp << endl;
    }
    temp += 0x68; //0xBC;
    if (ReadProcessMemory(processh, (LPVOID)temp, &temp, 4, NULL))
    {
        cout << temp << endl;
    }
    temp += 0xC; //0x300;
    if (ReadProcessMemory(processh, (LPVOID)temp, &temp, 4, NULL))
    {
        cout << temp << endl;
    }
    temp += 0xF9C; //0x10;
    //if (ReadProcessMemory(processh, (LPVOID)temp, &temp, 4, NULL))
    //{
    //    cout << temp << endl;
    //}
    //temp += 0xC;
    //if (ReadProcessMemory(processh, (LPVOID)temp, &temp, 4, NULL))
    //{
    //    cout << temp << endl;
    //}
    //temp += 0xF9C;
    temp++;
    cout << temp << endl;
    //if (ReadProcessMemory(processh, (LPVOID)temp, &temp, 4, NULL))
    //{
    //    cout <<temp<<endl;
    //}
    return temp;
}

void write(HANDLE processh, int _base, int *array)
{
    _base -= 0x12 * 4;
    int flag = 24;
    int data = 1;
    int realvalue = 0;
    for (int i = 0; i < 5; i++)
    {
        realvalue = read_map[array[flag--]];
        WriteProcessMemory(processh, (LPVOID)_base, &data, 1, NULL);
        WriteProcessMemory(processh, (LPVOID)(_base + 1), &realvalue, 1, NULL);
        _base += 0x12;
    } //1
    _base += 0xD8;
    for (int i = 0; i < 5; i++)
    {
        realvalue = read_map[array[flag--]];
        WriteProcessMemory(processh, (LPVOID)_base, &data, 1, NULL);
        WriteProcessMemory(processh, (LPVOID)(_base + 1), &realvalue, 1, NULL);
        _base += 0x12;
    } //2
    _base += 0xD8;

    for (int i = 0; i < 3; i++)
    {
        realvalue = read_map[array[flag--]];
        WriteProcessMemory(processh, (LPVOID)_base, &data, 1, NULL);
        WriteProcessMemory(processh, (LPVOID)(_base + 1), &realvalue, 1, NULL);
        _base += 0x24;
    } //3
    _base += 0xC6;

    for (int i = 0; i < 4; i++)
    {
        realvalue = read_map[array[flag--]];
        WriteProcessMemory(processh, (LPVOID)_base, &data, 1, NULL);
        WriteProcessMemory(processh, (LPVOID)(_base + 1), &realvalue, 1, NULL);
        if (i == 1)
            _base += 0x24;
        else
            _base += 0x12;
    } //4
    _base += 0xD8;

    for (int i = 0; i < 3; i++)
    {
        realvalue = read_map[array[flag--]];
        WriteProcessMemory(processh, (LPVOID)_base, &data, 1, NULL);
        WriteProcessMemory(processh, (LPVOID)(_base + 1), &realvalue, 1, NULL);
        _base += 0x24;
    } //5
    _base += 0xC6;

    for (int i = 0; i < 5; i++)
    {
        realvalue = read_map[array[flag--]];
        WriteProcessMemory(processh, (LPVOID)_base, &data, 1, NULL);
        WriteProcessMemory(processh, (LPVOID)(_base + 1), &realvalue, 1, NULL);
        _base += 0x12;
    } //6
    _base += 0xD8;
    int _base_right = _base + 5 * 0x12;
    for (int i = 0; i < 5; i++)
    {
        realvalue = 13;
        WriteProcessMemory(processh, (LPVOID)_base_right, &data, 1, NULL);
        WriteProcessMemory(processh, (LPVOID)(_base_right + 1), &realvalue, 1, NULL);
        _base_right += 0x132;
    }
    int _base_left = _base - 0x12;
    for (int i = 0; i < 5; i++)
    {
        realvalue = 13;
        WriteProcessMemory(processh, (LPVOID)_base_left, &data, 1, NULL);
        WriteProcessMemory(processh, (LPVOID)(_base_left + 1), &realvalue, 1, NULL);
        _base_left += 0x132;
    }
}

int *read(HANDLE &h, int base)
{
    int *array = new int[25];
    for (int i = 0; i < 25; i++)
        array[i] = 0;
    //int array[25] = {0};
    int flag = 0;
    cout << "**********" << endl;
    cout << base << endl; //10
    for (int i = 0; i < 5; i++)
    {

        ReadProcessMemory(h, (LPVOID)(base), &array[flag++], 1, NULL);
        base += 0x12;
    } //1
    base += 0xD8;
    for (int i = 0; i < 3; i++)
    {
        ReadProcessMemory(h, (LPVOID)(base), &array[flag++], 1, NULL);
        base += 0x24;
    } //2
    base += 0xC6;

    for (int i = 0; i < 4; i++)
    {
        ReadProcessMemory(h, (LPVOID)(base), &array[flag++], 1, NULL);
        if (i == 1)
            base += 0x24;
        else
            base += 0x12;
    } //3
    base += 0xD8;

    for (int i = 0; i < 3; i++)
    {
        ReadProcessMemory(h, (LPVOID)(base), &array[flag++], 1, NULL);
        base += 0x24;
    } //4
    base += 0xC6;

    for (int i = 0; i < 5; i++)
    {
        ReadProcessMemory(h, (LPVOID)(base), &array[flag++], 1, NULL);
        base += 0x12;
    }
    base += 0xD8;
    for (int i = 0; i < 5; i++)
    {
        ReadProcessMemory(h, (LPVOID)(base), &array[flag++], 1, NULL);
        base += 0x12;
    }
    cout << "why not cout" << endl;
    return array;
}

//int main()
//{                                                          //四国军棋 -- 房间:百万雄师
//    HWND h = ::FindWindow("ClientFrame_CMainFrame", NULL); //	  寻找并打开进程
//    DWORD processid;
//    GetWindowThreadProcessId(h, &processid);
//    cout << processid << endl;
//    HANDLE processh = 0;
//    processh = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processid);
//
//    if (processh == 0)
//    { //    对应处理
//        printf("打开进程失败!\n");
//        return 1;
//    }
//    else
//    {
//        printf("打开进程成功!\n");
//        int base = init(0x03720000, processh);
//        byte data = 1;
//        WriteProcessMemory(processh,
//                           (LPVOID)base, //要写入的起始地址
//                           &data,        //写入的缓存区
//                           1,            //要写入缓存区的大小
//                           NULL);
//        read(processh, base - 0x1 + 0xCCF);
//    }
//    return 0;
//}
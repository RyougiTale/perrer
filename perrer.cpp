#pragma once

#include <iostream>
using namespace std;
#include <boost/asio.hpp>
#include <future>
#include <thread>
//using namespace boost::asio;
#include "./plugin.cpp"
#include "./main.cpp"

int base;
HANDLE processh = 0;
string get_local_ip()
{
    using boost::asio::ip::udp;
    try
    {
        boost::asio::io_service netService;
        udp::resolver resolver(netService);
        udp::resolver::query query(udp::v4(), "baidu.com", "");
        udp::resolver::iterator endpoints = resolver.resolve(query);
        udp::endpoint ep = *endpoints;
        udp::socket socket(netService);
        socket.connect(ep);
        boost::asio::ip::address addr = socket.local_endpoint().address();
        std::cout << "My IP according to baidu is: " << addr.to_string() << std::endl;
        return addr.to_string();
    }
    catch (std::exception &e)
    {
        std::cerr << "Could not deal with socket. Exception: " << e.what() << std::endl;
    }
    return "";
}

void begin_listen(std::function<void(int *)> func, string &addr)
{
    std::thread([func, addr]() {
        boost::asio::io_service io_service;
        boost::asio::ip::udp::socket udp_socket(io_service);
        boost::asio::ip::udp::endpoint local_add(boost::asio::ip::address::from_string(addr), 2000);
        cout << "begin listen to " << addr << ":" << 2000 << endl;
        udp_socket.open(local_add.protocol());
        udp_socket.bind(local_add);

        int receive_str[1024] = {0};
        while (true)
        {
            boost::asio::ip::udp::endpoint sendpoint;
            udp_socket.receive_from(boost::asio::buffer(receive_str, 1024), sendpoint);
            //cout << "receive" << sendpoint.address().to_string() << ":" << receive_str << endl;
            func(receive_str);
            //udp_socket.send_to(boost::asio::buffer("server get success"), sendpoint);
            memset(receive_str, 0, 1024);
        }
    })
        .detach();
}

void begin_change_memory(int *array)
{
    int *ptr = array;
    if (*ptr == '\0')
        return;
    //for (int i = 0; i < 25; i++)
    //    cout << ptr[i] << endl;
    write(processh, base, ptr);
}

int main()
{
    int *chess;
    DWORD _processid;
    unsigned int _address_base;
    plugin_get(
        _processid, _address_base);
    //

    HWND h = ::FindWindow("ClientFrame_CMainFrame", NULL); //	  寻找并打开进程
    DWORD processid;
    GetWindowThreadProcessId(h, &processid);
    cout << processid << endl;
    processh = 0;
    processh = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processid);

    if (processh == 0)
    { //    对应处理
        printf("fail!\n");
        return -1;
    }
    else
    {
        printf("open process successfully!\n");
        base = init(_address_base, processh);
        int data = 1;
        WriteProcessMemory(processh,
                           (LPVOID)base, //要写入的起始地址
                           &data,        //写入的缓存区
                           1,            //要写入缓存区的大小
                           NULL);
        chess = read(processh, base - 0x1 + 0xCCF);
        //for (int i = 0; i < 25; i++)
        //{
        //    cout << chess[i] << endl;
        //}
    }
    //
    cout << processid << " " << _address_base << endl;
    string server_ip;
    string local_ip = get_local_ip();
    if (local_ip == (string) "192.168.0.106")
        server_ip = "192.168.0.108";
    else
        server_ip = "192.168.0.106";

    begin_listen(begin_change_memory, local_ip);

    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket udp_socket(io_service);
    boost::asio::ip::udp::endpoint local_add(boost::asio::ip::address::from_string(server_ip), 2000);
    udp_socket.open(local_add.protocol());
    char receive_str[1024] = {0};
    while (true)
    {
        boost::asio::ip::udp::endpoint sendpoint; //请求的IP以及端口
        //string s;
        //cin >> s;
        char c;
        c = getchar();
        chess = read(processh, base - 0x1 + 0xCCF);
        //udp_socket.send_to(boost::asio::buffer(s.c_str(), s.size()), local_add);
        udp_socket.send_to(boost::asio::buffer(chess, 25*sizeof(int)), local_add);
        //udp_socket.receive_from(boost::asio::buffer(receive_str, 1024), local_add); //收取
        //cout << "receive from server" << receive_str << endl;
        memset(receive_str, 0, 1024); //清空字符串
    }
    return 0;
}

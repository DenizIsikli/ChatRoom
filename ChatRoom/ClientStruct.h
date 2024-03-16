#pragma once
#include <WinSock2.h>
#include <string>


struct ClientStruct {
    SOCKET socket;
    std::string name;
    std::string color;
};

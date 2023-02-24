#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdlib.h>

#define DEFAULT_PORT "8000"

int WINAPI
wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PWSTR pCmdLine,
    int nCmdShow)
{
    // Initialize Winsock
    WSADATA wsaData;
    int result;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0)
    {
        OutputDebugString("WSAStartup failed with error code: ");
        char buffer[32] = {};
        _itoa_s(result, buffer, 10, 10);
        OutputDebugString(buffer);
        OutputDebugString("\n");
        return 1;
    }

    // Confirm that the Winsock dll is the correct version
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        OutputDebugString("Could not find a usable version of Winsock.dll. Exiting.\n");
        WSACleanup();
        return 1;
    }
    else
    {
        OutputDebugString("The Winsock 2.2 dll was found okay.\n");
    }

    struct addrinfo hints = {};
    hints.ai_family = AF_INET; // TODO: change?
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* infos;
    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &infos);
    if (result != 0)
    {
        OutputDebugString("getaddrinfo failed with error code: ");
        char buffer[32] = {};
        _itoa_s(result, buffer, 10, 10);
        OutputDebugString(buffer);
        OutputDebugString("\n");
        WSACleanup();
        return 1;
    }

    // Create listen socket
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(infos->ai_family, infos->ai_socktype, infos->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        OutputDebugString("socket failed with error code: ");
        char buffer[32] = {};
        _itoa_s(WSAGetLastError(), buffer, 10, 10);
        OutputDebugString(buffer);
        OutputDebugString("\n");
        freeaddrinfo(infos);
        WSACleanup();
        return 1;
    }

    // Bind socket to address/port
    result = bind(ListenSocket, infos->ai_addr, (int)infos->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        OutputDebugString("bind failed with error code: ");
        char buffer[32] = {};
        _itoa_s(WSAGetLastError(), buffer, 10, 10);
        OutputDebugString(buffer);
        OutputDebugString("\n");
        freeaddrinfo(infos);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(infos);

    // Listen for client connection
    OutputDebugString("Listening for a client connection...\n");
    if (listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR)
    {
        OutputDebugString("listen failed with error code: ");
        char buffer[32] = {};
        _itoa_s(WSAGetLastError(), buffer, 10, 10);
        OutputDebugString(buffer);
        OutputDebugString("\n");
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept client connection
    SOCKET ClientSocket = INVALID_SOCKET;
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        OutputDebugString("accept failed with error code: ");
        char buffer[32] = {};
        _itoa_s(WSAGetLastError(), buffer, 10, 10);
        OutputDebugString(buffer);
        OutputDebugString("\n");
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    OutputDebugString("Accepted a client connection.\n");

    // Shutdown sending to client
    result = shutdown(ClientSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        OutputDebugString("shutdown failed with error code: ");
        char buffer[32] = {};
        _itoa_s(WSAGetLastError(), buffer, 10, 10);
        OutputDebugString(buffer);
        OutputDebugString("\n");
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // Receive from client until it disconnects from us
    char recvbuf[512] = {};
    int numBytesRecv = 0;
    OutputDebugString("Receiving data from client...");
    while (1)
    {
        numBytesRecv = recv(ClientSocket, recvbuf, 512, 0);
        if (numBytesRecv > 0)
        {
            OutputDebugString("Received bytes: ");
            char buffer[32] = {};
            _itoa_s(numBytesRecv, buffer, 10, 10);
            OutputDebugString(buffer);
            OutputDebugString("\n");

            OutputDebugString("Printing bytes received:\n");
            OutputDebugString(recvbuf);
        }
        else if (numBytesRecv == 0)
        {
            OutputDebugString("Received 0 bytes - connection has been gracefully closed.\n");
            break;
        }
        else
        {
            OutputDebugString("recv failed with error code: ");
            char buffer[32] = {};
            _itoa_s(numBytesRecv, buffer, 10, 10);
            OutputDebugString(buffer);
            OutputDebugString("\n");
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    }

    // Cleanup
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}

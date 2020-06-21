#ifndef UNICODE
#define UNICODE 1
#endif
//The socket function creates a socket that is bound to a specific transport service provider.
// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>   // Needed for _wtoi

/*

__cdecl is the default calling convention for C and C++ programs. 
Because the stack is cleaned up by the caller, it can do vararg functions. 
The __cdecl calling convention creates larger executables than __stdcall, 
because it requires each function call to include stack cleanup code. 
The following list shows the implementation of this calling convention. 
The __cdecl modifier is Microsoft-specific.

see:
https://docs.microsoft.com/en-us/cpp/cpp/cdecl?redirectedfrom=MSDN&view=vs-2019

/ Gv（向量调用），/ Gz（stdcall）或/ Gr（快速调用）编译器选项的唯一时间。 / Gd编译器选项强制使用__cdecl调用约定。

struct CMyClass {
   void __cdecl mymethod();
};

void CMyClass::mymethod() { return; }

void __cdecl CMyClass::mymethod() { return; }

以下使用C命名和调用约定。

// Example of the __cdecl keyword on function
int __cdecl system(const char *);
// Example of the __cdecl keyword on function pointer
typedef BOOL (__cdecl *funcname_ptr)(void * arg1, const char * arg2, DWORD flags, ...)


*/

int __cdecl wmain(int argc, wchar_t **argv)
{

    //-----------------------------------------
    // Declare and initialize variables
    WSADATA wsaData = {0};
    int iResult = 0;

//    int i = 1;

    SOCKET sock = INVALID_SOCKET;
    int iFamily = AF_UNSPEC;
    int iType = 0;
    int iProtocol = 0;

    // Validate the parameters
    if (argc != 4) {
        wprintf(L"usage: %s <addressfamily> <type> <protocol>\n", argv[0]);
        wprintf(L"socket opens a socket for the specified family, type, & protocol\n");
        wprintf(L"%ws example usage\n", argv[0]);
        wprintf(L"   %ws 0 2 17\n", argv[0]);
        wprintf(L"   where AF_UNSPEC=0 SOCK_DGRAM=2 IPPROTO_UDP=17\n", argv[0]);
        return 1;
    }

    iFamily = _wtoi(argv[1]);
    iType = _wtoi(argv[2]);
    iProtocol = _wtoi(argv[3]);
    
    //1 Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wprintf(L"WSAStartup failed: %d\n", iResult);
        return 1;
    }

    wprintf(L"Calling socket with following parameters:\n");
    wprintf(L"  Address Family = ");
    switch (iFamily) {
    case AF_UNSPEC:
        wprintf(L"Unspecified");
        break;
    case AF_INET:
        wprintf(L"AF_INET (IPv4)");
        break;
    case AF_INET6:
        wprintf(L"AF_INET6 (IPv6)");
        break;
    case AF_NETBIOS:
        wprintf(L"AF_NETBIOS (NetBIOS)");
        break;
    case AF_BTH:
        wprintf(L"AF_BTH (Bluetooth)");
        break;
    default:
        wprintf(L"Other");
        break;
    }
    wprintf(L" (%d)\n", iFamily);
    
    wprintf(L"  Socket type = ");
    switch (iType) {
    case 0:
        wprintf(L"Unspecified");
        break;
    case SOCK_STREAM:
        wprintf(L"SOCK_STREAM (stream)");
        break;
    case SOCK_DGRAM:
        wprintf(L"SOCK_DGRAM (datagram)");
        break;
    case SOCK_RAW:
        wprintf(L"SOCK_RAW (raw)");
        break;
    case SOCK_RDM:
        wprintf(L"SOCK_RDM (reliable message datagram)");
        break;
    case SOCK_SEQPACKET:
        wprintf(L"SOCK_SEQPACKET (pseudo-stream packet)");
        break;
    default:
        wprintf(L"Other");
        break;
    }
    wprintf(L" (%d)\n", iType);

    wprintf(L"  Protocol = %d = ", iProtocol);
    switch (iProtocol) {
    case 0:
        wprintf(L"Unspecified");
        break;
    case IPPROTO_ICMP:
        wprintf(L"IPPROTO_ICMP (ICMP)");
        break;
    case IPPROTO_IGMP:
        wprintf(L"IPPROTO_IGMP (IGMP)");
        break;
    case IPPROTO_TCP:
        wprintf(L"IPPROTO_TCP (TCP)");
        break;
    case IPPROTO_UDP:
        wprintf(L"IPPROTO_UDP (UDP)");
        break;
    case IPPROTO_ICMPV6:
        wprintf(L"IPPROTO_ICMPV6 (ICMP Version 6)");
        break;
    default:
        wprintf(L"Other");
        break;
    }
    wprintf(L" (%d)\n", iProtocol);
	// 2
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) 
        wprintf(L"socket function failed with error = %d\n", WSAGetLastError() );
    else {
        wprintf(L"socket function succeeded\n");


		    // The socket address to be passed to bind
			sockaddr_in service;
			service.sin_family = AF_INET;
			service.sin_addr.s_addr = inet_addr("127.0.0.1");
			service.sin_port = htons(27015);
			// 3
			iResult = bind(sock,(SOCKADDR*)&service,sizeof(service));
			if(iResult == SOCKET_ERROR){
				wprintf(L"error bind socket\n");
				return 2;
			}else{
				wprintf(L"Success bind socket\n");
			}
			// Listen for incoming connection requests 
				//4  on the created socket
				if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
					wprintf(L"listen function failed with error: %d\n", WSAGetLastError());

				wprintf(L"Listening on socket...\n");
			// Create a SOCKET for accepting incoming requests.
			SOCKET AcceptSocket;
			wprintf(L"Waiting for client to connect...\n");

			//----------------------
			// 5 Accept the connection.
			AcceptSocket = accept(sock, NULL, NULL);
			if (AcceptSocket == INVALID_SOCKET) {
				wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
				closesocket(sock);
				WSACleanup();
				return 1;
			} else
				wprintf(L"Client connected.\n");

    }
	 
	
        // Close the socket to release the resources associated
        // Normally an application calls shutdown() before closesocket 
        //   to  disables sends or receives on a socket first
        // This isn't needed in this simple sample
        iResult = closesocket(sock);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"closesocket failed with error = %d\n", WSAGetLastError() );
            WSACleanup();
            return 1;
        }    
    WSACleanup();

    return 0;
}
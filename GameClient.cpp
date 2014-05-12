#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>


using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    int iResult, iRecvResult;
	int guessNumber;
	int guessedRight = -1;

    SOCKET ConnectSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    char *sendbuf = "Default client message \0";
	char inputBuffer[DEFAULT_BUFLEN] = "";
    char recvbuf[DEFAULT_BUFLEN] = "";
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

	
	//u_long iMode=1;
	//ioctlsocket(ConnectSocket,FIONBIO,&iMode);

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
	
	ofstream myfile;
	myfile.open ("output.txt");

    do {
		if(guessedRight == 1)
		{
			printf("Server guessed right. Enter next integer: ");
			gets(inputBuffer);
			guessNumber = atoi(inputBuffer);
			memset(&inputBuffer[0], '\0', DEFAULT_BUFLEN);
			printf("Asking server to guess the number...");
			string temp = "Your answer was right. Guess the new integer that client is thinking of...";
			std::copy(temp.begin(), temp.end(), inputBuffer);
		}
		else if (guessedRight == 0)
		{
			printf("Server guessed wrong. Enter next integer: ");
			gets(inputBuffer);
			guessNumber = atoi(inputBuffer);
			memset(&inputBuffer[0], '\0', DEFAULT_BUFLEN);
			printf("Asking server to guess the number...");
			string temp = "Your answer was wrong. Guess the new integer that client is thinking of...";
			std::copy(temp.begin(), temp.end(), inputBuffer);
		}
		else
		{
			printf("Enter an integer: ");
			gets(inputBuffer);
			guessNumber = atoi(inputBuffer);
			memset(&inputBuffer[0], '\0', DEFAULT_BUFLEN);
			printf("Asking server to guess the number...\n");
			string temp = "Guess the integer that client is thinking of...";
			std::copy(temp.begin(), temp.end(), inputBuffer);
		}




		// Send an initial buffer
		iResult = send( ConnectSocket, inputBuffer, (int)strlen(inputBuffer), 0 );
		time_t timerSent;
		time(&timerSent);

		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		
		myfile << "Sent: " << inputBuffer << "\n" ;

		memset(&recvbuf[0], '\0', DEFAULT_BUFLEN);

		time_t timerAcked;
		time(&timerAcked);
		double seconds = difftime(timerSent, timerAcked);
		do
		{
			iRecvResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		
			if ( iRecvResult > 0 && strcmp(recvbuf, "ack")==0)
			{
				//printf("Acknowledged!!\n", recvbuf);
			}
			else if (iRecvResult == -1)
			{}
			else
				printf("recv failed with error: %d\n", WSAGetLastError());
		} while (iRecvResult == -1);


		memset(&recvbuf[0], '\0', DEFAULT_BUFLEN);
		memset(&inputBuffer[0], '\0', DEFAULT_BUFLEN);

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
		{
            printf("Server guessed: %s\n", recvbuf);
			int serverGuess = atoi(recvbuf);
			if (serverGuess == guessNumber)
				guessedRight = 1;
			else
				guessedRight = 0;
			myfile << "Received: " << recvbuf << "\n" ;
		}
        else if ( iResult == 0 )
            printf("Connection closed\n");
		else if ( iResult == -1)
		{}
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while(iResult > 0);

	
	myfile.close();
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}

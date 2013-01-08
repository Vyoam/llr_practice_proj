#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

DWORD WINAPI SocketHandler(void*);

int main(int argv, char** argc){

    //The port you want the server to listen on
    int host_port= 1101;

    //Initialize socket support WINDOWS ONLY!
    unsigned short wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD( 2, 2 );
     err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
            HIBYTE( wsaData.wVersion ) != 2 )) {
        fprintf(stderr, "Could not find useable sock dll %d\n",WSAGetLastError());
        goto FINISH;
    }

    //Initialize sockets and set any options
    int hsock;
    int * p_int ;
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n",WSAGetLastError());
        goto FINISH;
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n", WSAGetLastError());
        free(p_int);
        goto FINISH;
    }
    free(p_int);

    //Bind and listen
    struct sockaddr_in my_addr;

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;
    
    if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",WSAGetLastError());
        goto FINISH;
    }
    if(listen( hsock, 10) == -1 ){
        fprintf(stderr, "Error listening %d\n",WSAGetLastError());
        goto FINISH;
    }
    
    //Now lets to the server stuff

    int* csock;
    sockaddr_in sadr;
    int    addr_size = sizeof(SOCKADDR);
    
    while(true){
        printf("Waiting for a connection\n");
        csock = (int*)malloc(sizeof(int));
        
        if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET ){
            printf("Received connection from %s\n",inet_ntoa(sadr.sin_addr));
            CreateThread(0,0,&SocketHandler, (void*)csock , 0,0);
        }
        else{
            fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
        }
		//test, thread memory not freed?
		if(getchar()=='q')
			return 0;
    }

FINISH:
;
}

DWORD WINAPI SocketHandler(void* lp){
    int *csock = (int*)lp;

    char buffer[1024];
    int buffer_len = 1024;
    int bytecount;

    memset(buffer, 0, buffer_len);
    if((bytecount = recv(*csock, buffer, buffer_len, 0))==SOCKET_ERROR){
        fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
        goto FINISH;
    }
    printf("Received bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
    strcat(buffer, " SERVER ECHO");

    if((bytecount = send(*csock, buffer, strlen(buffer), 0))==SOCKET_ERROR){
        fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
        goto FINISH;
    }
    
    printf("Sent bytes %d\n", bytecount);


FINISH:
    free(csock);
    return 0;
}
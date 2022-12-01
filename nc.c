#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

//we use this sites to write the code:
//https://linux.die.net/man/1/nc
//https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
//https://www.commandlinux.com/man-page/man1/nc.1.html


//the server will get the massage fron the user(stdin) and will send it to the stdout
int server(char *portNum)
{
    int port = atoi(portNum);//convert the string of port from user to int
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if(listen(socket_desc, 1) < 0){
        printf("Error while listening\n");
        return -1;
    }
    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);

    if (client_sock < 0){
        printf("Can't accept\n");
        return -1;
    }
    //printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    //we use an infinite loop to always read from the client until we can't
    while (1) {
        // Receive client's message:
        if (recv(client_sock, client_message, sizeof(client_message), 0) < 0) {
            printf("Couldn't receive\n");
            return -1;
        }
        printf("%s\n", client_message);
    }

    // Closing the socket:
    close(client_sock);
    close(socket_desc);

    return 0;
}

//int ipTOint(char *ip){
//    //for this converting from a ip address to an int i used this code: https://gist.github.com/jayjayswal/fc435fe261af9e45ccaf
//    char c;
//    c = *ip;
//    unsigned int integer;
//    int val;
//    int i,j=0;
//    for (j=0;j<4;j++) {
//        if (!isdigit(c)){  //first char is 0
//            return (0);
//        }
//        val=0;
//        for (i=0;i<3;i++) {
//            if (isdigit(c)) {
//                val = (val * 10) + (c - '0');
//                c = *++ip;
//            } else
//                break;
//        }
//        if(val<0 || val>255){
//            return (0);
//        }
//        if (c == '.') {
//            integer=(integer<<8) | val;
//            c = *++ip;
//        }
//        else if(j==3 && c == '\0'){
//            integer=(integer<<8) | val;
//            break;
//        }
//
//    }
//    if(c != '\0'){
//        return (0);
//    }
//    return (htonl(integer));
//}

//the client is the user(stdin) that send massage to the socket(server)
int client(char * portNum, char * ipAddress)
{
    int port = atoi(portNum);//convert the string of port from user to int
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ipAddress);

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    // Get input from the user:
    printf("Enter message: ");

    //we run in an infinite loop to always read from stdin(user)
    while (1) {
        read(0, client_message, 256);

        // Send the message to server:
        if (send(socket_desc, client_message, strlen(client_message), 0) < 0) {
            printf("Unable to send message\n");
            return -1;
        }
    }
    // Close the socket:
    close(socket_desc);

    return 0;
}

int main(int argc, char *argv[]) {
    //the nc command gets a flag(we interested only in -lv and -v), a port and an ip address
    if (strcmp(argv[1], "-lv") == 0)
    {
        server(argv[2]); //exe: nc -lv 1234
    }
    else
    {
        client(argv[3], argv[2]); //exe: nc -v 10.0.2.4 1234
    }
    return 0;
}
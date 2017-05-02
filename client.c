#include "client.h"
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    struct message mSent;
    struct message mRecv;
    char messageS[1000] , server_reply[2000];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
     
    //keep communicating with server
    while(1)
    {
        printf("Enter message : ");
        scanf("%s" , messageS);
        strcpy(mSent.payload, messageS);
        mSent.code=2;
        //Send some data
        if( send(sock , &mSent , sizeof(struct message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , &mRecv , sizeof(struct message) , 0) < 0)
        {
            puts("recv failed");
            break;
        }
         
        puts("Server reply :");
        printf("%s\n", mRecv.payload);
    }
     
    close(sock);
    return 0;
}

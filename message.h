#ifndef MESSAGE_H
#define MESSAGE_H
#define MESSAGE_MAX_LENGTH 2000
typedef struct message {
    int code;
    char payload[MESSAGE_MAX_LENGTH];
} message;
//server-> client
//code
#define C_OK 0
#define C_REFUSE 1
#define C_SERVER_ERROR 2
#define C_INFO 3
//message
#define M_SERVER_ERROR "An error occured on ther servor\n"
#define M_CONN_REFUSE "Sorry the connection was refused by the server\n"
#define M_GREET_CLIENT "Welcome to Papayoo Online, you are now connected\n" 
#define M_SIGNUP_CLIENT_OK "Thank you ! You've been correctly signed up for the next game.\n" 
//client-> server
#define C_ADD_PLAYER 0
#define C_DEFAULT 1337 
//TO DO GERER L INDEX DU TABLEAU / CONSTANTE DE MANIERE PLUS COHERENTE
#endif

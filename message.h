#define MESSAGE_MAX_LENGTH 2000
typedef struct message {
    int code;
    char payload[MESSAGE_MAX_LENGTH];
} message;
#define C_OK 0
#define C_CLIENT_ERROR 1
#define C_SERVER_ERROR 2
#define C_DEFAULT 3

#define M_GREET_CLIENT  "Everybody get up it's time to slam now\nWe got a real jam goin' down\nWelcome to the Space Jam\nHere's your chance do your dance at the Space Jam\nAlright.\n"
#define M_SIGNUP_CLIENT_OK "Thank you ! You've been correctly signed up for the next game.\n" 

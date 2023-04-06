#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <signal.h>

#include <unistd.h>

// to read(),write() and close()

#include <sys/types.h>

//#include <sys/types.h> -> this contain definatoins of number of data types usen in system call

// ans also used for #include <sys/socket.h> and #include <netinet/in.h>

#include <sys/socket.h>

//#include <sys/socket.h> -> contain defination and stuctures such as 'sockaddr'

#include <netinet/in.h>

//#include <netinet/in.h> -> contain defination,constat and sturcture (e.g.- 'sockaddr_in')for internet domain address

#include <arpa/inet.h>

//#include <arpa/inet.h> -> contains definitions for internet operations .

#include <pthread.h> // for threading

#define LENGTH 2048

// Global variables

volatile sig_atomic_t flag = 0;

int sockfd = 0; // socket id

char name[32];

void str_overwrite()

{

    printf("%s", "> ");

    fflush(stdout);

    // to flush the output buffer of the stream
}

void trim(char *arr, int length)

{

    int i;

    for (i = 0; i < length; i++)

    { // trim the length of the buffer/message.

        if (arr[i] == '\n')

        {

            arr[i] = '\0';

            break;
        }
    }
}

void LEAVE_ROOM(int sig)

{

    // function to disconnect the client

    flag = 1;
}

void send_msg()

{

    char message[LENGTH] = {};

    char buffer[LENGTH + 32] = {};

    while (1)

    {

        str_overwrite();

        fgets(message, LENGTH, stdin);

        trim(message, LENGTH);

        if (strcmp(message, "Bye") == 0)

        {

            // if the client types "Bye",it instructs the code to shut doen the client.

            break;
        }

        else

        {

            sprintf(buffer, "%s: %s\n", name, message); // prints the input message on the same screen.

            send(sockfd, buffer, strlen(buffer), 0); // sends the message to the server.
        }

        bzero(message, LENGTH); // clears the message.

        bzero(buffer, LENGTH + 32); // clears the buffer.
    }

    LEAVE_ROOM(2); // instructs the code to exit the client.
}

void recv_msg()

{

    char message[LENGTH] = {};

    while (1)

    {

        int receive = recv(sockfd, message, LENGTH, 0); // receives the message from the server.

        if (receive > 0)

        {

            // prints the message if no error occurs.

            printf("%s", message);

            str_overwrite(); // clears stdout.
        }

        else if (receive == 0) // connection lost.

            break;

        else

        {

            // -1->error
        }

        memset(message, 0, sizeof(message));
    }
}

int main(int argc, char **argv)

{

    // argc-> server parameter

    // agrv-> port number

    // to verify the server parameter and port number =>

    if (argc != 2)

    {

        printf("Usage: %s <port>\n", argv[0]);

        return EXIT_FAILURE;
    }

    char *ip = "127.0.0.1";

    int port = atoi(argv[1]); // converting ip string to int (port)

    signal(SIGINT, LEAVE_ROOM); // signal to check if to close the client

    printf("Please enter your name: ");

    fgets(name, 32, stdin);

    trim(name, strlen(name));

    // rejecting the name if greater than 32 characters

    if (strlen(name) > 32 || strlen(name) < 2)

    {

        printf("Name must be less than 30 and more than 2 characters.\n");

        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    // sockadd_in structure...

    // Socket settings

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr(ip);

    server_addr.sin_port = htons(port);

    // Connect to Server

    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (err == -1) // if err is -1 then do Exit

    {

        printf("ERROR: connect\n");

        return EXIT_FAILURE;
    }

    // Send name

    send(sockfd, name, 32, 0);

    printf("\t\t=====WELCOME TO THE CHATROOM=====\n");

    // creating thread to send the message

    pthread_t send_msg_thread;

    if (pthread_create(&send_msg_thread, NULL, (void *)send_msg, NULL) != 0)

    {

        printf("ERROR: pthread\n");

        return EXIT_FAILURE;
    }

    // creating thread to receive the message

    pthread_t recv_msg_thread;

    if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg, NULL) != 0)

    {

        printf("ERROR: pthread\n");

        return EXIT_FAILURE;
    }

    while (1)

    {

        if (flag) // to disconnect the client.

        {

            printf("\nBye\n");

            printf("%s has left the ChatRoom.", name);

            break;
        }
    }

    close(sockfd); // closing socket.

    return EXIT_SUCCESS;
}

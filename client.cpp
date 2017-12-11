#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEFAULT_BUFLEN 256

/* Declarations */
struct client_type
{
    int id;
    int sockfd;
    char received_message[DEFAULT_BUFLEN];
};

const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;

/* Function prototypes */
int process_client(client_type &new_client);

int process_client(client_type &new_client)
{
    while (true) {
        memset(new_client.received_message, 0, DEFAULT_BUFLEN);

        if (new_client.sockfd != 0) {
            int iResult = recv(
                new_client.sockfd,
                new_client.received_message,
                DEFAULT_BUFLEN, 0
            );

            if (iResult != SOCKET_ERROR) {
                printf("%s\n", new_client.received_message);
            } else {
                fprintf(stderr, "recv() failed\n");
                break;
            }
        }
    }

    // if (WSAGetLastError() == WSAECONNRESET)
    fprintf(stderr, "The server has disconnected\n");

    return 0;
}

int main(int argc, char** argv)
{
    int port_no;
    struct sockaddr_in server_addr;
    struct hostent *server;
    client_type client = { INVALID_SOCKET, -1, "" };
    std::string message;
    std::string sent_message = "";
    int ret = 0;

    /* Banner */
    fprintf(stdout, "simple-chat-client v0.0.1-alpha\n");

    /* Arguments */
    if (argc < 3) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        return -1;
    }

    port_no = atoi(argv[2]);
    if (port_no <= 0) {
        fprintf(stderr, "Invalid port -.-\n");
        return -1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return -2;
    }

    fprintf(stdout, "Starting client...\n");

    /* Opening socket */
    client.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client.sockfd < 0) {
        fprintf(stderr, "ERROR opening socket\n");
        return -2;
    }

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy(
        (char *)server->h_addr,
        (char *)&server_addr.sin_addr.s_addr,
        server->h_length
    );
    server_addr.sin_port = htons(port_no);

    /* Connecting */
    printf("Trying connect to %s:%d...\n", (char *)server->h_addr, port_no);
    if (connect(client.sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        close(client.sockfd);
        fprintf(stderr, "Unable to connect to server!\n");
        return -3;
    }
    fprintf(stdout, "Connected!\n");

    /* Receiving handshake */
    recv(client.sockfd, client.received_message, DEFAULT_BUFLEN, 0);
    message = client.received_message;

    if (message != "Server is full") {
        client.id = atoi(client.received_message);

        std::thread my_thread(process_client, std::ref(client));

        while (true) {
            fprintf(stdout, "You> ");
            getline(std::cin, sent_message);

            ret = send(client.sockfd, sent_message.c_str(), strlen(sent_message.c_str()), 0);

            if (ret <= 0) {
                fprintf(stderr, "send() failed\n");
                break;
            }
        }

        /* Shutdown the connection since no more data will be sent */
        my_thread.detach();
    } else {
        printf("%s\n", client.received_message);
    }


    /* Closing socket */
    fprintf(stdout, "Closing socket...\n");
    ret = shutdown(client.sockfd, SHUT_WR);
    if (ret == SOCKET_ERROR) {
        fprintf(stderr, "shutdown() failed with error.\n");
        close(client.sockfd);
        return -4;
    }

    close(client.sockfd);
    return 0;
}

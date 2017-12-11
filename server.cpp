#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char** argv)
{
    int sockfd;
    int port_no;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    char buffer[256];

    fprintf(stdout, "simple-chat-server v0.0.1-alpha\n");

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        return -1;
    }

    port_no = atoi(argv[1]);
    if (port_no <= 0) {
        fprintf(stderr, "Invalid port -.-\n");
        return -1;
    }


    /* Create socket */
    fprintf(stdout, "Creating server socket...\n");

    sockfd =  socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR opening socket\n");
        return -2;
    }


    /* Clear address structure */
    bzero((char *) &server_addr, sizeof(server_addr));

    /* Setup the host_addr structure for use in bind call */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);

    /* Bind */
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ERROR on binding\n");
        return -3;
    }

    /* Listen */
    listen(sockfd, 5);
    printf("Listening on %s:%d\n", &server_addr.sin_addr.s_addr, port_no);

    client_len = sizeof(client_addr);

    /* Accept new connection */
    int incoming;
    incoming = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
    if (incoming < 0) {
        fprintf(stderr, "ERROR on accept\n");
        return -5;
    }

    /* Got connection */
    char remote_host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
    printf("Got connection from %s port %d\n", remote_host, ntohs(client_addr.sin_port));


    /* Sending hello :D */
    send(incoming, "Hello :D\n", 9, 0);

    bzero(buffer, 256);

    int n;
    n = read(incoming, buffer, 255);
    if (n < 0) {
        fprintf(stderr, "ERROR reading from socket\n");
        return -5;
    }

    printf("Message received: %s\n", buffer);

    /* Closing */
    close(incoming);
    close(sockfd);

    return 0;
}

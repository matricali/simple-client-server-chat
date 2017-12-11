#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char** argv)
{
    int sockfd;
    int port_no;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[256];

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

    /* Opening socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
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
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ERROR opening socket\n");
        return -3;
    }
    fprintf(stdout, "Connected!\n");

    /* Sending message */
    printf("Please enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    int n;
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        fprintf(stderr, "ERROR writing to socket\n");
        return -4;
    }
    bzero(buffer, 256);

    /* Receiving message */
    n = read(sockfd, buffer, 255);
    if (n < 0) {
        fprintf(stderr, "ERROR reading from socket\n");
        return -4;
    }
    printf("%s\n", buffer);

    /* Closing socket */
    close(sockfd);
    return 0;
}

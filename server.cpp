#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* Definitions */
#define DEFAULT_BUFLEN 256

/* Declarations */
struct client_type
{
    int id;
    int sockfd;
};

const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
const int MAX_CLIENTS = 5;


/* Function prototypes */
int process_client(
    client_type &new_client,
    std::vector<client_type> &client_array,
    std::thread &thread
);

int process_client(
    client_type &new_client,
    std::vector<client_type> &client_array,
    std::thread &thread
)
{
    std::string msg = "";
    char tempmsg[DEFAULT_BUFLEN] = "";

    /* Session */
    while (true) {
        memset(tempmsg, 0, DEFAULT_BUFLEN);

        if (new_client.sockfd != 0) {
            int iResult = recv(new_client.sockfd, tempmsg, DEFAULT_BUFLEN, 0);

            if (iResult != SOCKET_ERROR) {
                if (strcmp("", tempmsg)) {
                    msg = "Client #" + std::to_string(new_client.id) + ": " + tempmsg;
                }

                printf("%s\n", msg.c_str());

                /* Broadcast that message to the other clients */
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_array[i].sockfd != INVALID_SOCKET) {
                        if (new_client.id != i) {
                            iResult = send(client_array[i].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
                        }
                    }
                }
            } else {
                msg = "Client #" + std::to_string(new_client.id) + " Disconnected";

                printf("%s\n", msg.c_str());

                close(new_client.sockfd);
                close(client_array[new_client.id].sockfd);
                client_array[new_client.id].sockfd = INVALID_SOCKET;

                /* Broadcast the disconnection message to the other clients */
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (client_array[i].sockfd != INVALID_SOCKET) {
                        iResult = send(client_array[i].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
                    }
                }

                break;
            }
        }
    }

    thread.detach();

    return 0;
}

int main(int argc, char** argv)
{
    int sockfd;
    int port_no;
    int num_clients = 0;
    int temp_id = -1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    std::vector<client_type> client(MAX_CLIENTS);
    std::string msg = "";
    std::thread client_threads[MAX_CLIENTS];

    /* Banner */
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
    char local_host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr.sin_addr), local_host, INET_ADDRSTRLEN);
    printf("Listening on %s:%d\n", local_host, port_no);

    /* Initialize the client list */
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client[i] = (client_type) { -1, INVALID_SOCKET };
    }

    client_len = sizeof(client_addr);

    while (true) {
        /* Accept new connection */
        int incoming = INVALID_SOCKET;
        incoming = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);

        if (incoming == INVALID_SOCKET) {
            continue;
        }

        /* Reset the number of clients */
        num_clients = -1;

        /* Create a temporary id for the next client */
        temp_id = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client[i].sockfd == INVALID_SOCKET && temp_id == -1) {
                client[i].sockfd = incoming;
                client[i].id = i;
                temp_id = i;
            }

            if (client[i].sockfd != INVALID_SOCKET) {
                num_clients++;
            }
        }

        if (temp_id != -1) {
            char remote_host[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
            printf(
                "Connection accepted from %s:%d (Client #%d) accepted\n",
                remote_host,
                ntohs(client_addr.sin_port),
                client[temp_id].id
            );

            /* Send the id to that client */
            msg = std::to_string(client[temp_id].id);
            send(client[temp_id].sockfd, msg.c_str(), strlen(msg.c_str()), 0);

            /* Create a thread process for that client */
            client_threads[temp_id] = std::thread(
                process_client,
                std::ref(client[temp_id]),
                std::ref(client),
                std::ref(client_threads[temp_id])
            );
        } else {
            /* The server is full :( */
            msg = "Server is full";
            send(incoming, msg.c_str(), strlen(msg.c_str()), 0);

            char remote_host[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
            printf(
                "Connection rejected to %s:%d (Client #%d). The server is full!\n",
                remote_host,
                ntohs(client_addr.sin_port),
                client[temp_id].id
            );

            /* Close connection */
            close(client[temp_id].sockfd);
        }
    }

    /* Closing */
    close(sockfd);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_threads[i].detach();
        close(client[i].sockfd);
    }

    return 0;
}

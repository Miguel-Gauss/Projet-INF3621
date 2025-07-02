#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12900);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("connect");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int received;

    // Réception du message 1
    memset(buffer, 0, sizeof(buffer));
    received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) { perror("recv mes1"); close(client_socket); exit(EXIT_FAILURE); }
    printf("%s", buffer);

    // Envoi ID
    char input1[256];
    fgets(input1, sizeof(input1), stdin);
    send(client_socket, input1, strlen(input1), 0);

    // Réception du message 2
    memset(buffer, 0, sizeof(buffer));
    received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) { perror("recv mes2"); close(client_socket); exit(EXIT_FAILURE); }
    printf("%s", buffer);

    // Envoi nombre de recommandations
    char input2[256];
    fgets(input2, sizeof(input2), stdin);
    send(client_socket, input2, strlen(input2), 0);

    // Réception du message 3
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) { perror("recv mes3"); close(client_socket); exit(EXIT_FAILURE); }
        printf("%s", buffer);

        // Envoi choix algo
        char input3[256];
        fgets(input3, sizeof(input3), stdin);
        send(client_socket, input3, strlen(input3), 0);

        int algo = atoi(input3);
        if (algo == 1 || algo == 2 || algo == 3) break;
    }

    // Attente et affichage du résultat final (résultat de l’algorithme exécuté)
    memset(buffer, 0, sizeof(buffer));
    while ((received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[received] = '\0';
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    close(client_socket);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "knn.h"
#include "facto.h"
#include "graphe.h"

int main() {
    int serveur_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (serveur_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12900);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(serveur_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(serveur_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(serveur_socket, 10) < 0) {
        perror("listen");
        close(serveur_socket);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur 127.0.0.1:12900\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(serveur_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_socket);
            continue;
        } else if (pid == 0) {
            close(serveur_socket);

            char *mes1 = "Veuillez entrer votre id !\n";
            char *mes2 = "Veuillez entrer le nombre de recommandations voulu !\n";
            char *mes3 = "Veuillez entrer l'algo de recommandation voulu. Tapez 1 pour knn, 2 pour factorisation matricielle, 3 pour graphe !\n";

            char recept1[1024] = {0};
            char recept2[1024] = {0};
            char recept3[1024] = {0};

            send(client_socket, mes1, strlen(mes1), 0);
            recv(client_socket, recept1, sizeof(recept1) - 1, 0);

            send(client_socket, mes2, strlen(mes2), 0);
            recv(client_socket, recept2, sizeof(recept2) - 1, 0);

            send(client_socket, mes3, strlen(mes3), 0);
            recv(client_socket, recept3, sizeof(recept3) - 1, 0);

            int id = atoi(recept1);
            int nb_recommandations = atoi(recept2);
            int choix_algo = atoi(recept3);

            while (choix_algo != 1 && choix_algo != 2 && choix_algo != 3) {
                send(client_socket, mes3, strlen(mes3), 0);
                memset(recept3, 0, sizeof(recept3));
                recv(client_socket, recept3, sizeof(recept3) - 1, 0);
                choix_algo = atoi(recept3);
            }

            if (choix_algo == 1) {
                lancer_knn("database.txt", id, nb_recommandations, 5, client_socket);
            } else if (choix_algo == 2)
            {
                lancer_facto("database.txt", id, nb_recommandations, 5, client_socket);
            }else{
                lancer_graphe("database.txt", id, nb_recommandations, 5, client_socket);
            }

            close(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            close(client_socket);
        }
    }

    close(serveur_socket);
    return 0;
}

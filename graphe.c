#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "graphe.h"

#define MAX_USERS 1000
#define MAX_ITEMS 5000
#define MIN_ITEM_ID 1

typedef struct {
    int user_id;
    int item_id;
} Interaction;

static int user_item[MAX_USERS][MAX_ITEMS - MIN_ITEM_ID + 1] = {0};

static void load_data(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    int user_id, item_id, cat_id;
    float rating;
    long timestamp;

    while (fscanf(file, "%d %d %d %f %ld", &user_id, &item_id, &cat_id, &rating, &timestamp) == 5) {
        if (user_id < 1 || user_id > MAX_USERS || item_id < MIN_ITEM_ID || item_id >= MIN_ITEM_ID + MAX_ITEMS) continue;
        user_item[user_id - 1][item_id - MIN_ITEM_ID] = 1;
    }

    fclose(file);
}

void lancer_graphe(const char* fichier_data, int user_id, int nb_recos, int profondeur, int client_socket) {
    load_data(fichier_data);

    int u = user_id - 1;
    int num_items = MAX_ITEMS - MIN_ITEM_ID + 1;
    int scores[num_items];
    memset(scores, 0, sizeof(scores));

    for (int v = 0; v < MAX_USERS; v++) {
        if (v == u) continue;

        int common = 0;
        for (int i = 0; i < num_items; i++) {
            if (user_item[u][i] && user_item[v][i]) common++;
        }

        if (common > 0) {
            for (int i = 0; i < num_items; i++) {
                if (!user_item[u][i] && user_item[v][i]) {
                    scores[i] += 1; // augmente score si un voisin l’a aimé
                }
            }
        }
    }

    // Sélection des meilleurs articles
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Articles recommandés par graphe pour l'utilisateur %d :\n", user_id);
    send(client_socket, buffer, strlen(buffer), 0);

    int recos = 0;
    int *déjà_fait = calloc(num_items, sizeof(int));

    while (recos < nb_recos) {
        int max_score = -1;
        int best_item = -1;

        for (int i = 0; i < num_items; i++) {
            if (scores[i] > max_score && !user_item[u][i] && !déjà_fait[i]) {
                max_score = scores[i];
                best_item = i;
            }
        }

        if (best_item == -1) break;

        snprintf(buffer, sizeof(buffer), " - Article %d (popularité : %d)\n", best_item + MIN_ITEM_ID, scores[best_item]);
        send(client_socket, buffer, strlen(buffer), 0);

        déjà_fait[best_item] = 1;
        recos++;
    }

    (void)profondeur;

    if (recos == 0) {
        char *msg = "Aucune recommandation disponible via graphe.\n";
        send(client_socket, msg, strlen(msg), 0);
    }

    free(déjà_fait);
}

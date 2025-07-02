#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include "facto.h"

#define MAX_USERS 1000
#define MAX_ITEMS 5000
#define MIN_ITEM_ID 1

static float ratings[MAX_USERS][MAX_ITEMS - MIN_ITEM_ID + 1] = {0};
static int rated[MAX_USERS][MAX_ITEMS - MIN_ITEM_ID + 1] = {0};

// Charge les données dans ratings et rated
static void load_data(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        exit(1);
    }

    int user_id, item_id, cat_id;
    float rating;
    long timestamp;

    while (fscanf(file, "%d %d %d %f %ld", &user_id, &item_id, &cat_id, &rating, &timestamp) == 5) {
        if (user_id < 1 || user_id > MAX_USERS || item_id < MIN_ITEM_ID || item_id >= MIN_ITEM_ID + MAX_ITEMS) continue;
        ratings[user_id - 1][item_id - MIN_ITEM_ID] = rating;
        rated[user_id - 1][item_id - MIN_ITEM_ID] = 1;
    }
    fclose(file);
}

// Fonction qui fait la factorisation matricielle par descente de gradient simple
// (Méthode basique pour illustration)
void lancer_facto(const char* fichier_data, int user_id, int nb_recos, int rank, int client_socket) {
    load_data(fichier_data);

    int num_users = MAX_USERS;
    int num_items = MAX_ITEMS - MIN_ITEM_ID + 1;

    // Initialiser matrices U (user features) et V (item features) aléatoirement
    float **U = malloc(num_users * sizeof(float*));
    float **V = malloc(num_items * sizeof(float*));
    if (!U || !V) {
        perror("malloc");
        return;
    }

    for (int i = 0; i < num_users; i++) {
        U[i] = malloc(rank * sizeof(float));
        for (int f = 0; f < rank; f++)
            U[i][f] = ((float)rand() / RAND_MAX) * 0.1f;
    }

    for (int j = 0; j < num_items; j++) {
        V[j] = malloc(rank * sizeof(float));
        for (int f = 0; f < rank; f++)
            V[j][f] = ((float)rand() / RAND_MAX) * 0.1f;
    }

    // Hyperparamètres simples
    float learning_rate = 0.005f;
    float regularization = 0.02f;
    int epochs = 50;

    // Descente de gradient pour ajuster U et V
    for (int epoch = 0; epoch < epochs; epoch++) {
        for (int i = 0; i < num_users; i++) {
            for (int j = 0; j < num_items; j++) {
                if (rated[i][j]) {
                    // prédiction
                    float pred = 0;
                    for (int f = 0; f < rank; f++) {
                        pred += U[i][f] * V[j][f];
                    }
                    float err = ratings[i][j] - pred;
                    // Mise à jour
                    for (int f = 0; f < rank; f++) {
                        float uf = U[i][f];
                        float vf = V[j][f];
                        U[i][f] += learning_rate * (err * vf - regularization * uf);
                        V[j][f] += learning_rate * (err * uf - regularization * vf);
                    }
                }
            }
        }
    }

    // Calcul des scores de prédiction pour l'utilisateur demandé
    int user = user_id - 1;
    float *scores = malloc(num_items * sizeof(float));
    int *already_rated = malloc(num_items * sizeof(int));
    if (!scores || !already_rated) {
        perror("malloc");
        // Libération mémoire partielle
        for (int i = 0; i < num_users; i++) free(U[i]);
        for (int j = 0; j < num_items; j++) free(V[j]);
        free(U);
        free(V);
        free(scores);
        free(already_rated);
        return;
    }

    for (int j = 0; j < num_items; j++) {
        already_rated[j] = rated[user][j];
        scores[j] = 0;
        for (int f = 0; f < rank; f++) {
            scores[j] += U[user][f] * V[j][f];
        }
    }

    // Envoi des résultats sur client_socket
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Articles recommandés par factorisation matricielle pour l'utilisateur %d :\n", user_id);
    send(client_socket, buffer, strlen(buffer), 0);

    int recommandations = 0;
    int *already_reco = calloc(num_items, sizeof(int));
    if (!already_reco) {
        perror("malloc");
        goto cleanup;
    }

    while (recommandations < nb_recos) {
        float best_score = -1e9f;
        int best_item = -1;

        for (int j = 0; j < num_items; j++) {
            if (!already_rated[j] && !already_reco[j]) {
                if (scores[j] > best_score) {
                    best_score = scores[j];
                    best_item = j;
                }
            }
        }

        if (best_item == -1) break;

        snprintf(buffer, sizeof(buffer), " - Article %d (score estimé : %.2f)\n", best_item + MIN_ITEM_ID, best_score);
        send(client_socket, buffer, strlen(buffer), 0);

        already_reco[best_item] = 1;
        recommandations++;
    }

    if (recommandations == 0) {
        char *msg = "Aucune recommandation disponible.\n";
        send(client_socket, msg, strlen(msg), 0);
    }

    free(already_reco);

cleanup:
    free(scores);
    free(already_rated);
    for (int i = 0; i < num_users; i++) free(U[i]);
    for (int j = 0; j < num_items; j++) free(V[j]);
    free(U);
    free(V);
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h> // pour write/send
#include <sys/socket.h>
#include "knn.h"

#define MAX_USERS 1000
#define MAX_ITEMS 5000
#define MIN_ITEM_ID 1

static float ratings[MAX_USERS][MAX_ITEMS - MIN_ITEM_ID + 1] = {0};
static int rated[MAX_USERS][MAX_ITEMS - MIN_ITEM_ID + 1] = {0};

void load_data(const char* filename) {
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

float moyenne_utilisateur(int user) {
    float somme = 0;
    int count = 0;
    for (int i = 0; i < MAX_ITEMS - MIN_ITEM_ID + 1; i++) {
        if (rated[user][i]) {
            somme += ratings[user][i];
            count++;
        }
    }
    return count > 0 ? somme / count : 0;
}

float pearson(int u1, int u2) {
    float moy1 = moyenne_utilisateur(u1);
    float moy2 = moyenne_utilisateur(u2);
    float num = 0, denom1 = 0, denom2 = 0;

    for (int i = 0; i < MAX_ITEMS - MIN_ITEM_ID + 1; i++) {
        if (rated[u1][i] && rated[u2][i]) {
            float d1 = ratings[u1][i] - moy1;
            float d2 = ratings[u2][i] - moy2;
            num += d1 * d2;
            denom1 += d1 * d1;
            denom2 += d2 * d2;
        }
    }

    if (denom1 == 0 || denom2 == 0) return 0;
    return num / (sqrt(denom1) * sqrt(denom2));
}

typedef struct {
    int user_id;
    float sim;
} SimUser;

int compare_sim(const void* a, const void* b) {
    float diff = ((SimUser*)b)->sim - ((SimUser*)a)->sim;
    if (diff > 0) return 1;
    else if (diff < 0) return -1;
    else return 0;
}

void lancer_knn(const char* fichier_data, int user_id, int nb_recos, int k, int client_socket) {
    load_data(fichier_data);

    SimUser sim_users[MAX_USERS];
    int user = user_id - 1;

    for (int i = 0; i < MAX_USERS; i++) {
        if (i == user) {
            sim_users[i].user_id = i;
            sim_users[i].sim = -2; // Ignorer soi-même
            continue;
        }
        sim_users[i].user_id = i;
        sim_users[i].sim = pearson(user, i);
    }

    qsort(sim_users, MAX_USERS, sizeof(SimUser), compare_sim);

    float scores[MAX_ITEMS - MIN_ITEM_ID + 1] = {0};
    float total_sim[MAX_ITEMS - MIN_ITEM_ID + 1] = {0};

    for (int i = 0; i < k; i++) {
        int voisin = sim_users[i].user_id;
        float sim = sim_users[i].sim;

        if (sim <= 0) continue; // Ignore les similitudes nulles ou négatives

        for (int j = 0; j < MAX_ITEMS - MIN_ITEM_ID + 1; j++) {
            if (!rated[user][j] && rated[voisin][j]) {
                scores[j] += sim * ratings[voisin][j];
                total_sim[j] += fabs(sim);
            }
        }
    }

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Articles recommandés pour l'utilisateur %d :\n", user_id);
    send(client_socket, buffer, strlen(buffer), 0);

    // Marquer les articles recommandés pour ne pas les répéter
    int recommandés = 0;
    //int items_copies = 0;
    int max_items = MAX_ITEMS - MIN_ITEM_ID + 1;
    int *already_reco = calloc(max_items, sizeof(int));
    if (!already_reco) {
        perror("malloc");
        return;
    }

    while (recommandés < nb_recos) {
        float best_score = -1;
        int best_item = -1;

        for (int j = 0; j < max_items; j++) {
            if (!rated[user][j] && total_sim[j] > 0 && !already_reco[j]) {
                float final_score = scores[j] / total_sim[j];
                if (final_score > best_score) {
                    best_score = final_score;
                    best_item = j;
                }
            }
        }

        if (best_item == -1) break;

        snprintf(buffer, sizeof(buffer), " - Article %d (score estimé : %.2f)\n", best_item + MIN_ITEM_ID, best_score);
        send(client_socket, buffer, strlen(buffer), 0);

        already_reco[best_item] = 1;
        recommandés++;
    }

    if (recommandés == 0) {
        char *msg = "Aucune recommandation disponible.\n";
        send(client_socket, msg, strlen(msg), 0);
    }

    free(already_reco);
}

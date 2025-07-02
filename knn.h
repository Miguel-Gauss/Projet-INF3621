#ifndef KNN_H
#define KNN_H

#ifdef __cplusplus
extern "C" {
#endif

// Ajout de client_socket pour envoyer les résultats au client
void lancer_knn(const char* fichier_data, int user_id, int nb_recos, int k, int client_socket);

#ifdef __cplusplus
}
#endif

#endif // KNN_H

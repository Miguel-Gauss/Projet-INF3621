                                      =====Système de Recommandation Client-Serveur en C=====

 ===Objectif====

Ce projet a pour but de développer un système de recommandation basé sur une architecture client-serveur en langage C. Il permet de recommander des items à un utilisateur à partir d'un ensemble de transactions (notations).

Le serveur centralise les données, applique un algorithme de recommandation (KNN, MF ou Graphe) et retourne les résultats au client qui les demande.

 

 ===Fonctionnalités===

  =Côté Client=

- Envoie une requête au serveur contenant :
  - user_id
  - nombre de recommandations (topN)
  - algorithme de recommandation : KNN, MF, ou GRAPH
- Reçoit les recommandations du serveur et les affiche

 =Côté Serveur=

- Gère plusieurs clients simultanément (via fork())
- Charge les données du fichier ratings.txt
- Applique l'algorithme choisi :
  - KNN (K-Nearest Neighbors) : utilisateurs similaires
  - MF (Matrix Factorization) : factorisation matricielle
  - GRAPH (PageRank) : graphe de co-notes
- Envoie les items recommandés au client

 =Prétraitement des données=

Des fonctions supplémentaires permettent :

1.  Extraction d’une période temporelle donnée :
   - Exemple : janvier 2000 à février 2000

2. Filtrage des utilisateurs/items peu fréquents :
   - Supprime ceux ayant moins de minu (utilisateurs) ou mini (items) transactions

3. Nettoyage des données de test :
   - Supprime dans les données de test les utilisateurs/items absents dans l’ensemble d’apprentissage

   ==Compilation==

Utiliser la commande suivante dans la racine du projet :

make

Cela compile :
- Les bibliothèques (libknn.a, libmf.a, libgraph.a)
- Le serveur : server
- Le client : client

  ==Exécution==

Démarrer le serveur :

./server

Lancer un client (dans un autre terminal) :

./client

Vous devrez entrer :
- Un user_id existant
- Un nombre de recommandations
- Un algorithme : KNN, MF, ou GRAPH

  =Format du fichier ratings.txt=

Chaque ligne suit le format :

<user_id> <item_id> <category_id> <rating> <timestamp>

Exemple :

249 1087 21 4.3 969215645
83 1134 34 3.5 1000121189
71231 43 2 2.4 998855800

  =Dépendances=

- Système Linux
- Compilateur gcc
- Bibliothèque standard C (Pas de dépendance externe)

  =Auteurs=
  
- Nom : TCHELIEBOU KAMGA FELIX         22Y1044
- Nom : SAKTA NZIA PIERRICK MIGUEL     22Y1042
- Université : Université de Yaoundé I
- Cours : INF362 - Programmation Système & Services Réseaux

 =Contacts=

En cas de bug ou suggestion, n'hésitez pas à créer un issue ou envoyer un mail à :  
 miguel.sakta@facsiences-uy1.cm ou felix.tcheliebou@facsiences-uy1.cm

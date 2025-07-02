CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC
AR = ar
ARFLAGS = rcs

# Fichiers sources
KNN_SRC = knn.c
FACTO_SRC = facto.c
GRAPHE_SRC = graphe.c
SERVEUR_SRC = serveur.c

# Objets
KNN_OBJ = knn.o
FACTO_OBJ = facto.o
GRAPHE_OBJ = graphe.o
SERVEUR_OBJ = serveur.o

# Bibliothèques statiques
KNN_LIB = libknn.a
FACTO_LIB = libfacto.a
GRAPHE_LIB = libgraphe.a

# Binaire final
TARGET = serveur

all: $(TARGET)

$(KNN_OBJ): $(KNN_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(FACTO_OBJ): $(FACTO_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(GRAPHE_OBJ): $(GRAPHE_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVEUR_OBJ): $(SERVEUR_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(KNN_LIB): $(KNN_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(FACTO_LIB): $(FACTO_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(GRAPHE_LIB): $(GRAPHE_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(TARGET): $(SERVEUR_OBJ) $(KNN_LIB) $(FACTO_LIB) $(GRAPHE_LIB)
	$(CC) $(CFLAGS) $^ -o $@ -lm

clean:
	rm -f *.o *.a $(TARGET)

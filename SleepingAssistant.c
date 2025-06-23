#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_ETUDIANTS 5
#define NB_CHAISES 3

sem_t etudiant_sem;      // Nombre d'étudiants qui attendent (signale au TA)
sem_t ta_sem;            // Signale que le TA est prêt à aider
pthread_mutex_t mutex_chaises;

int chaises[NB_CHAISES]; // File circulaire des chaises (id des étudiants)
int pos_debut = 0;
int pos_fin = 0;
int nb_chaises_occupees = 0;

void *etudiant(void *num) {
    int id = *(int *)num;
    while (1) {
        printf("Étudiant %d programme...\n", id);
        sleep(rand() % 5 + 1); // Simulation de la programmation

        pthread_mutex_lock(&mutex_chaises);
        if (nb_chaises_occupees < NB_CHAISES) {
            // L'étudiant prend une chaise
            chaises[pos_fin] = id;
            pos_fin = (pos_fin + 1) % NB_CHAISES;
            nb_chaises_occupees++;
            printf("Étudiant %d attend sur une chaise (%d/%d occupées).\n", id, nb_chaises_occupees, NB_CHAISES);
            pthread_mutex_unlock(&mutex_chaises);

            sem_post(&etudiant_sem); // Réveille le TA si nécessaire
            sem_wait(&ta_sem);       // Attend que le TA soit prêt à aider
        } else {
            printf("Étudiant %d ne trouve pas de chaise libre. Il reviendra plus tard.\n", id);
            pthread_mutex_unlock(&mutex_chaises);
        }
    }
    pthread_exit(NULL);
}

void *ta(void *arg) {
    while (1) {
        sem_wait(&etudiant_sem); // Attend un étudiant

        pthread_mutex_lock(&mutex_chaises);
        int id = chaises[pos_debut];
        pos_debut = (pos_debut + 1) % NB_CHAISES;
        nb_chaises_occupees--;
        pthread_mutex_unlock(&mutex_chaises);

        printf("TA aide l'étudiant %d...\n", id);
        sleep(rand() % 3 + 1); // Simulation de l'aide
        printf("TA a fini d'aider l'étudiant %d.\n", id);
        sem_post(&ta_sem); // Signale à l'étudiant que l'aide est terminée
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    pthread_t ta_thread;
    pthread_t threads_etudiants[NUM_ETUDIANTS];
    int id_etudiants[NUM_ETUDIANTS];

    // Initialisation
    sem_init(&etudiant_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);
    pthread_mutex_init(&mutex_chaises, NULL);

    pthread_create(&ta_thread, NULL, ta, NULL);

    for (int i = 0; i < NUM_ETUDIANTS; i++) {
        id_etudiants[i] = i + 1;
        pthread_create(&threads_etudiants[i], NULL, etudiant, &id_etudiants[i]);
    }

    pthread_join(ta_thread, NULL); // Jamais atteint dans ce cas
    for (int i = 0; i < NUM_ETUDIANTS; i++) {
        pthread_join(threads_etudiants[i], NULL);
    }

    pthread_mutex_destroy(&mutex_chaises);
    sem_destroy(&etudiant_sem);
    sem_destroy(&ta_sem);

    return 0;
}

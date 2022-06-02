#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#define NbVA 10 // Nombre de processus symbolisant les voitures abonnées
#define NbVN 8 // Nombre de processus symbolisant les voitures non abonnées

int placeA = 10;
int placeN = 10;
int temps;
pthread_t tidVA[NbVA];
pthread_t tidVN[NbVN];
pthread_t tidB[1];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t attendre, dormir = PTHREAD_COND_INITIALIZER;

// initialisations
int NbVoituresAttente = 0; // Nombre de clients en attente d'etre coiffes

void Barriere()
{
    pthread_mutex_lock(&mutex);
    if (placeA+placeN > 0) 
    {
        //pthread_cond_wait(&dormir, &mutex); //barriere fermée car une voiture sort
        pthread_cond_signal(&attendre); // La voiture s'installe dans le parking (et va sortir)
    }
    else
    {
        printf("La barrière est fermée car pas de place\n");
        pthread_cond_wait(&dormir, &mutex);
        pthread_cond_signal(&attendre); // La voiture s'installe dans le parking (et va sortir)
    }
    pthread_mutex_unlock(&mutex);
}

bool inArray(pthread_t tid, pthread_t tab[]){
    for (int i = 0; i < sizeof(tab); i++){
        if (tab[i] == tid){
            return true;
        }
    }
    return false;
}

void Voiture(int i)
{

    if (inArray(pthread_self(), tidVA) && placeA>0)
    {
        pthread_mutex_lock(&mutex);
        printf("La voiture A%d avertit la barrière qu'elle est la et patiente\n", i);
        pthread_cond_signal(&dormir);
        placeA--;
        pthread_cond_wait(&attendre, &mutex);
        printf("La barrière invite la voiture abonnée %d à se garer \n", (int)i);
        pthread_mutex_unlock(&mutex);
    }
    else if (inArray(pthread_self(), tidVN) && placeN>0)
    {
        pthread_mutex_lock(&mutex);
        printf("La voiture N%d avertit la barrière qu'elle est la et patiente\n", i);
        pthread_cond_signal(&dormir);
        placeN--;
        pthread_cond_wait(&attendre, &mutex);
        printf("La barrière invite la voiture non abonnée %d à se garer \n", (int)i);
        pthread_mutex_unlock(&mutex);
    }
    else{
        if (inArray(pthread_self(), tidVA)){
            printf("La voiture A%d ne trouve pas de place et passe son chemin\n", i);
        } else {
            printf("La voiture N%d ne trouve pas de place et passe son chemin\n", i);
        }
    }
        
}

void *fonc_parking()
{

    while (1)
    {
        Barriere();
        /* temps de stationement d'une voiture */
        usleep(300000);
    }
}

void *fonc_voiture(void *i)
{
    Voiture((int)i);

    /* temps de sortie parking */
    usleep(100000);
    if (inArray(pthread_self(), tidVA)){
            printf("La voiture A%d quitte le parking\n", (int)i);
            placeA++;

        } else {
            printf("La voiture N%d quitte le parking\n", (int)i);
            placeN++;
        }
}

int main(int argc, char *argv[])
{
    temps = atoi(argv[1]);
    printf("%d\n",temps);
    int num;
    // pthread_mutex_init(&mutex, 1);

    // creation de la thread coiffeur
    pthread_create(tidB, 0, (void *(*)())fonc_parking, NULL);

    // creation des threads voitures
    for (num = 0; num < NbVA; num++) //abonné
        pthread_create(tidVA + num, 0, (void *(*)())fonc_voiture, (void *)num);

    for (num = 0; num < NbVN; num++) //non abonné
        pthread_create(tidVN + num, 0, (void *(*)())fonc_voiture, (void *)num);

    // attend la fin de toutes les threads voiture
    for (num = 0; num < NbVA; num++) //abonné
        pthread_join(tidVA[num], NULL);
    
    for (num = 0; num < NbVN; num++)
    pthread_join(tidVN[num], NULL);  //non abonné

    /* liberation des ressources");*/
    pthread_exit(NULL);

    exit(0);
}
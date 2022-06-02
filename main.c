#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#define NbVA 1 // Nombre de processus symbolisant les voitures abonnées
#define NbVN 10 // Nombre de processus symbolisant les voitures non abonnées

int placeA = 0;
int placeN = 10;
int sortie = 0;
int coeff;
int plafond;
pthread_t tidVA[NbVA];
pthread_t tidVN[NbVN];
pthread_t tidB[1];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t attendre, dormir, attendreb = PTHREAD_COND_INITIALIZER;

// initialisations
int NbVoituresAttente = 0; // Nombre de clients en attente d'etre coiffes

void Barriere()
{
    pthread_mutex_lock(&mutex);
    if (placeA+placeN > 0) 
    {
        if (sortie == 1){
        do{
            printf("Barrière fermée, voiture en sortie\n");
            usleep(10000);
        }while(sortie == 1);
        }
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

    if (placeN>0 && plafond>0) //En effet, les places non abonnées dispo sont pas les mêmes en fonction du temps de la journée
    {
        /*Entrée stationnement*/
        pthread_mutex_lock(&mutex);
        printf("La voiture N%d avertit la barrière qu'elle est la et patiente\n", i);
        pthread_cond_signal(&dormir);
        pthread_cond_wait(&attendre, &mutex);
        printf("La barrière à invité la voiture N%d à se garer \n", (int)i);
        placeN--;
        plafond--; //décrémentation du plafond pour le %de places non abonné dispo en fonction du temps de la journée
        pthread_mutex_unlock(&mutex);

        usleep(10000);

        /*sortie stationnement*/
        pthread_mutex_lock(&mutex);
        sortie = 1; //verouillage de la barrière en mode sortie
        printf("La voiture N%d sort et verrouille la barrière\n", (int)i);
        placeN++;
        plafond++;
        printf("La voiture N%d a quitté le parking et déverouille la barrière\n", (int)i);
        sortie = 0; //deverouillage de la barrière
        pthread_mutex_unlock(&mutex);

    }
    else{
        printf("La voiture N%d ne trouve pas de place et passe son chemin\n", i);
    }
    
}

void VoitureA(int i)
{

    if (placeA>0)
    {
        /*Entrée stationnement*/
        pthread_mutex_lock(&mutex);
        printf("La voiture A%d avertit la barrière qu'elle est la et patiente\n", i);
        pthread_cond_signal(&dormir);
        pthread_cond_wait(&attendre, &mutex);
        printf("La barrière à invité la voiture A%d à se garer \n", (int)i);
        placeA--;
        pthread_mutex_unlock(&mutex);

        usleep(10000);
        
        /*sortie stationnement*/
        pthread_mutex_lock(&mutex);
        sortie = 1; //verouillage de la barrière en mode sortie
        printf("La voiture A%d sort et verrouille la barrière\n", (int)i);
        placeA++;
        printf("La voiture A%d a quitté le parking et déverouille la barrière\n", (int)i);
        sortie = 0; //deverouillage de la barrière
        pthread_mutex_unlock(&mutex);

    } else if (placeA == 0 && placeN >0)
    {
        /*Entrée stationnement*/
        pthread_mutex_lock(&mutex);
        printf("La voiture A%d avertit la barrière qu'elle est la et patiente\n", i);
        pthread_cond_signal(&dormir);
        pthread_cond_wait(&attendre, &mutex);
        printf("La barrière à invité la voiture A%d à se garer sur une place non abonnée \n", (int)i);
        placeN--;
        pthread_mutex_unlock(&mutex);

        usleep(10000);
        
        /*sortie stationnement*/
        pthread_mutex_lock(&mutex);
        sortie = 1; //verouillage de la barrière en mode sortie
        printf("La voiture A%d sort de sa place non abonnée et verrouille la barrière\n", (int)i);
        placeN++;
        printf("La voiture A%d a quitté le parking et déverouille la barrière\n", (int)i);
        sortie = 0; //deverouillage de la barrière
        pthread_mutex_unlock(&mutex);
    }
    
    else{
        printf("La voiture A%d ne trouve pas de place et passe son chemin\n", i);
    }
        /* temps de sortie parking */

        
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
}

void *fonc_voitureA(void *i)
{
    VoitureA((int)i);
}

int main(int argc, char *argv[])
{
    coeff = atoi(argv[1]); //coefficient en fonction de l'heure de la journée
    plafond = (coeff / 100.0) * placeN; //on définit le plafond pour les non abonnées en fonction de l'heure passée en paramètre.
    printf("le plafond %d\n",plafond);
    int num;

    // creation de la thread coiffeur
    pthread_create(tidB, 0, (void *(*)())fonc_parking, NULL);

    // creation des threads voitures
    for (num = 0; num < NbVA; num++) //abonné
        pthread_create(tidVA + num, 0, (void *(*)())fonc_voitureA, (void *)num);

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
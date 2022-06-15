/* Projet réalisé par Brice VAN AKEN et Thomas STREBLER */

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

// #define NbVA 255 // Nombre de processus symbolisant les voitures abonnées
// #define NbVN 255 // Nombre de processus symbolisant les voitures non abonnées

int NbV = 50;       // Nombre de voitures 
int placeA = 5;     // Nombre de places abonné
int placeN = 10;    // Nombre de places non abonné

pthread_t tidB[1]; //tableau du tid de la barrière

int sortie = 0;
int plafond; //plafond des places non abonné
// pthread_t tidVA[NbVA]; //tableau regroupant les tid des threads abonné
// pthread_t tidVN[NbVN]; //tableau regroupant les tid des threads non abonné

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t attendre, dormir = PTHREAD_COND_INITIALIZER;


void Barriere()
{
    pthread_mutex_lock(&mutex);
    if (placeA+placeN > 0) 
    {
        if (sortie == 1){
        do{
            printf("Barrière fermée, voiture en sortie\n");
            /*synchonisation affichage
            temps vérification si voiture est bien sortie*/
            usleep(10000);
        }while(sortie == 1);
        }
        pthread_cond_signal(&attendre); // La voiture s'installe dans le parking (et va sortir)
    }
    else
    {
        printf("La barrière est fermée car pas de place\n");
        pthread_cond_wait(&dormir, &mutex); //endormissement de la barriere
        pthread_cond_signal(&attendre); // La voiture s'installe dans le parking (et va sortir)
    }
    pthread_mutex_unlock(&mutex);
}

void Voiture(int i)
{
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&attendre, &mutex); //en attente de la barrière

    if (placeN>0 && plafond>0) //En effet, les places non abonnées dispo sont pas les mêmes en fonction du temps de la journée
    {
        /*Entrée stationnement*/
        printf("La voiture N%d arrive sur le parking\n", i);
        printf("La barrière à invité la voiture N%d à se garer \n", (int)i);
        placeN--; //décrémentation des places
        plafond--; //décrémentation du plafond pour le %de places non abonné dispo en fonction du temps de la journée
        pthread_mutex_unlock(&mutex);

        /* temps de stationnement d'une voiture aléatoire entre 5000 et 30000 µs */
        usleep(rand()%25001 + 5000);

        /*sortie stationnement*/
        pthread_mutex_lock(&mutex);
        sortie = 1; //verouillage de la barrière en mode sortie
        printf("La voiture N%d sort et verrouille la barrière\n", (int)i);
        placeN++;
        plafond++;
        printf("La voiture N%d a quitté le parking et déverouille la barrière\n", (int)i);
        sortie = 0; //deverouillage de la barrière
        pthread_cond_signal(&dormir); //on signale à la barriere qu'on part
        pthread_mutex_unlock(&mutex);

    }
    else{
        pthread_mutex_unlock(&mutex);
        printf("La voiture N%d ne trouve pas de place et passe son chemin\n", i);
    }
    
}

void VoitureA(int i)
{
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&attendre, &mutex); //en attente de la barrière

    if (placeA>0)
    {
        /*Entrée stationnement*/
        printf("La voiture A%d arrive sur le parking\n", i);
        printf("La barrière a invité la voiture A%d à se garer \n", (int)i);
        placeA--;
        pthread_mutex_unlock(&mutex);

        /* temps de stationnement d'une voiture aléatoire entre 5000 et 30000 µs */
        usleep(rand()%25001 + 5000);
        
        /*sortie stationnement*/
        pthread_mutex_lock(&mutex);
        sortie = 1; //verouillage de la barrière en mode sortie
        printf("La voiture A%d sort et verrouille la barrière\n", (int)i);
        placeA++;
        printf("La voiture A%d a quitté le parking et déverouille la barrière\n", (int)i);
        sortie = 0; //deverouillage de la barrière
        pthread_cond_signal(&dormir); //on signale à la barriere qu'on part
        pthread_mutex_unlock(&mutex);

    } else if (placeA == 0 && placeN >0)
    {
        /*Entrée stationnement*/
        printf("La voiture A%d arrive sur le parking\n", i);
        printf("La barrière à invité la voiture A%d à se garer sur une place non abonnée \n", (int)i);
        placeN--;
        pthread_mutex_unlock(&mutex);

        /* temps de stationnement d'une voiture aléatoire entre 5000 et 30000 µs */
        usleep(rand()%25001 + 5000);
        
        /*sortie stationnement*/
        pthread_mutex_lock(&mutex);
        sortie = 1; //verouillage de la barrière en mode sortie
        printf("La voiture A%d sort de sa place non abonnée et verrouille la barrière\n", (int)i);
        placeN++;
        printf("La voiture A%d a quitté le parking et déverouille la barrière\n", (int)i);
        sortie = 0; //deverouillage de la barrière
        pthread_cond_signal(&dormir);//on signale à la barriere qu'on part
        pthread_mutex_unlock(&mutex);
    }
    
    else{
        pthread_mutex_unlock(&mutex);
        printf("La voiture A%d ne trouve pas de place et passe son chemin\n", i);
    }
        /* temps de sortie parking */

        
}

void *fonc_parking()
{
    while (1)
    {
        Barriere();
        /*synchronisation entrée / sortie affichage
        temps de levée / abaissement barriere*/
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
    int coeff = atoi(argv[1]); ////coefficient régissant le plafond des places non abonnées en fonction de l'heure
    //remplacement des valeurs de place par défaut par celles données en argument
    if (argc > 4){
        placeA = atoi(argv[2]);
        placeN = atoi(argv[3]);
        NbV = atoi(argv[4]);
    }
    plafond = (coeff / 100.0) * placeN; //on définit le plafond pour les non abonnées en fonction de l'heure passée en paramètre.
    printf("\n!! Zone de débordement activée, coefficient = %d %%!!\n", coeff);
    printf("!! Places non abonnées disponibles : %d !! \n\n",plafond);


    printf("PlaceA = %d, PlaceN = %d, NbV = %d\n",placeA,placeN,NbV);

    // creation de la thread barrière
    pthread_create(tidB, 0, (void *(*)())fonc_parking, NULL);

    int num;
    int numA = 0;
    int numN = 0;
    pthread_t tidV[NbV]; //tableau des tid des voitures

    // creation des threads voitures
    for (num = 0; num < NbV; num++){
        if (rand()%2 == 0){
            pthread_create(tidV + num, 0, (void *(*)())fonc_voitureA, (void *)numA);
            numA++;
        } else{
            pthread_create(tidV + num, 0, (void *(*)())fonc_voiture, (void *)numN);
            numN++;
        }
        usleep(100000);
    }

    // attend la fin de toutes les threads voiture
    for (num = 0; num < NbV; num++) //abonné
        pthread_join(tidV[num], NULL);

    /* liberation des ressources */
    pthread_exit(NULL);

    exit(0);
}
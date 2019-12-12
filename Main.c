#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>

#define quantum 1
#define IFLAGS (SEMPERM | IPC_CREAT)
#define SKEY   (key_t) IPC_PRIVATE	
#define SEMPERM 0600
#define TAILLETAB 10
#define NBPROCESSUS 10

int tabPid[NBPROCESSUS];
int prioriteProcessus[NBPROCESSUS];
int* tempsProcessus;
int priorites[TAILLETAB]; //Table d'allocation des priorités (choisie avec ces pourcentages-là par défaut)
int tabDate[NBPROCESSUS];

typedef struct elem {
	int valeur;
	int temps;
	struct elem*suivant;
}element;

typedef struct {
    long type;			
    pid_t pid;			
} treponse;

typedef element * T;

T filePriorite[TAILLETAB];
int sem_id,shmid,msgid;
struct sembuf sem_oper_P ;  /* Operation P */
struct sembuf sem_oper_V ; /* Operation V */

int initsem(key_t semkey) 
{
    
	int status = 0;		
	int semid_init;

   	union semun {
		int val;
		struct semid_ds *stat;
		short * array;
	} ctl_arg;

    if ((semid_init = semget(semkey, 2, IFLAGS)) > 0) {
		
		short array[2] = {0,0};
		ctl_arg.array = array;
		status = semctl(semid_init, 0, SETALL, ctl_arg);
		ctl_arg.val = 1;
		semctl(semid_init, 0, SETVAL, ctl_arg);
    }
   if (semid_init == -1 || status == -1) { 
		perror("Erreur initsem");
		return (-1);
    } else return (semid_init);
}

void P(int semnum) {
	sem_oper_P.sem_num = semnum;
	sem_oper_P.sem_op  = -1 ;
	sem_oper_P.sem_flg = 0 ;
	int retrn = semop(sem_id,&sem_oper_P,semnum);
	
	while(retrn < 0){
		retrn = semctl(sem_id, semnum, GETVAL, 0);
	}
}

void V(int semnum) {
	sem_oper_V.sem_num = semnum;
	sem_oper_V.sem_op  = 1 ;
	sem_oper_V.sem_flg  = 0 ;
	int retrn = semop(sem_id, &sem_oper_V, semnum);
}

//affiche le tableau d'allocation des priorités
void afficheTableau(){
	printf("**********************************************\nTableau des priorités:\n");
	printf("———————————————————————————————————\n|numéro de priorité | pourcentage |\n");
	for (int i = 0; i < TAILLETAB; ++i)
	{
		printf("| %d                 | %d          |\n",i,priorites[i]);
	}
	printf("———————————————————————————————————\n");
	printf("**********************************************\n");
}
//permet de modifier les valeurs du tableau d'allocation CPU
void modifierTableau(){
	//on affiche le tableau si l'utilisateur a choisi -t
	afficheTableau();
	char reponse[8];

	printf("\nVoulez-vous modifier le tableau des priorités? (o/n)\n");
	fflush(stdout);

	//entrée de l'utilisateur, test de réponse pour qu'on ne puisse entrer que n ou o
	//strcmp compare deux chaines de caractères (renvoie 1 si faux, 0 si vrai)
	while(strcmp(reponse,"o") && strcmp(reponse,"n")){
		scanf("%s",reponse);//on récupère l'input de l'utilisateur

		if(strcmp(reponse,"o") && strcmp(reponse,"n")){
			printf("entree : %s\n",reponse);
			printf("entrée incorrecte : entrez o pour oui et n pour non\n");
			fflush(stdout);
		}
	}

	if(strcmp(reponse,"o") == 0){
		//choix de modification par l'utilisateur
		printf("Entrez les nouvelles valeurs des pourcentages :\n");
		int prioriteTemp[TAILLETAB]; // tableau temporaire pour stocker les pourcentages

		int resultatVrai = 0; //sert de condition pour vérifier si les valeurs entrées sont bonnes
		while(resultatVrai == 0){

			for (int i = 0; i < TAILLETAB; i+=1)
			{
				printf("priorité %d : pourcentage? ",i);
				scanf("%s",reponse);
				prioriteTemp[i] = atoi(reponse);
			}
			//test si les valeurs sont correctes. Si oui, on applique les modifications
			/*a adapter à TAILLETAB*/
			int somme= 0;
			for(int i =0; i<TAILLETAB; i++){
				somme+=prioriteTemp[i];
			}
			if(somme == 100){
				resultatVrai = 1;
				for (int i = 0; i < TAILLETAB; i+=1){
					priorites[i] = prioriteTemp[i];
				}
			} else {
				printf("Les valeurs entrées ne correspondent pas à des pourcentages valides.\n");
			}
		}
		afficheTableau();
	}
}
//renvoie le PGCD de a et b
int PGCD(int a, int b){
    if(a==b){
            return a;        
    }     
    else{
        if(a>b)
           return PGCD(a-b, b);
        else
           return PGCD(a, b-a);
    }
}
//renvoie le PGCD de tous les entiers du tableau
int superPGCD(int tab[]){
	int pgcd = tab[0];
	for (int i = 0; i < TAILLETAB-1; ++i)
	{
		pgcd = PGCD(pgcd,tab[i+1]);
	}
	return pgcd;
}
//genere le tourniquet de priorités en fonction du tableau de priorités
int* genereRoundRobin(int taille){
	int *round = malloc(taille * sizeof(int));
	int compteur[taille];
	int increment = 0;
	for (int i = 0; i < TAILLETAB; ++i)
	{	
		compteur[i] =priorites[i]*taille/100;
	}
	for (int i = 0; i < taille; ++i)
	{
		while(compteur[increment] == 0){
			increment++;
			if(increment >= TAILLETAB){
				increment = 0;
			}
		}
		round[i] = increment;
		compteur[increment]--;
		increment++;
		if(increment >= TAILLETAB){
			increment = 0;
		}
	}
	return round;
}
//génère NBPROCESSUS processus
void genereProcessus(){
	int pid,key;
	char buffer [200];
	if ((key = ftok("main", 'A')) == -1) {
		perror("Erreur de creation de la clé \n");
		exit(1);
	}
    if ((msgid = msgget(key, 0750 | IPC_CREAT )) == -1) {
		perror("Erreur de creation de la file\n");
		exit(1);
    }
	shmid = shmget(IPC_PRIVATE, sizeof(int)*NBPROCESSUS, 0666);
  	tempsProcessus = (int *)shmat(shmid, NULL, 0);
	sem_id = initsem(SKEY);

	for (int i = 0; i < NBPROCESSUS; ++i)
	{
		prioriteProcessus[i] = (int)(rand() / (double)RAND_MAX * 9);
		P(0);
		tempsProcessus[i] = (int)(rand() / (double)RAND_MAX * 9);
		V(0);
		tabDate[i] = (int)(rand() / (double)RAND_MAX * 9);
		pid = fork();
		tabPid[i] = pid;
		if(pid == 0){
			sprintf(buffer, "%d %d %d",msgid,sem_id,tempsProcessus[i]);
			if (execl("./processus","processus",buffer,(char*)NULL) == -1){
				printf("erreur de execl\n");
	       		fflush(stdout);
			}
		}
		else {
			P(0);
			printf("pid : %d et priorite :%d et date arrivee %d et temps : %d\n",pid, prioriteProcessus[i],tabDate[i],tempsProcessus[i]);
			V(0);
		}
	}
	return;
}

int verifierTemps(){
	int reponse = 0;
	for(int i = 0; i<TAILLETAB; i++){
		if(filePriorite[i] != NULL){
			reponse = 1;
		}
	}
	return reponse;
}

void ajouterFin(int priorite, int pidProcessus,int temps){
	T temp,I;
	temp=(T)malloc(sizeof(element));
	temp->valeur = pidProcessus;
	temp->temps = temps;
	temp ->suivant=NULL;

	if(filePriorite[priorite]==NULL){
		filePriorite[priorite] =temp;
	}
	else{
		I=filePriorite[priorite];
		while(I->suivant!=NULL){
			I=I->suivant;
		}
		I->suivant = temp;
	}
}
void retirerDebut(int priorite){
	T m;
	int nouvellePrio;
	if(filePriorite[priorite]!=NULL)
	{
		m=filePriorite[priorite]->suivant;
		if(priorite == TAILLETAB-1){
			nouvellePrio = 0;
		} else {
			nouvellePrio = priorite+1;
		}
		treponse requete;
		requete.pid = getpid();
		requete.type = filePriorite[priorite]->valeur;
		filePriorite[priorite]->temps --;
		msgsnd(msgid,&requete,sizeof(treponse)-4,0);
		if(filePriorite[priorite]->temps !=0){
			ajouterFin(nouvellePrio,filePriorite[priorite]->valeur,filePriorite[priorite]->temps);
		}
		free(filePriorite[priorite]);
		filePriorite[priorite] = m;
	}
}

int tourniquet(int* roundRobin,int pgcd){
	int date = 0;
	int taille = 100/pgcd;
	int moche = 0;
	while(verifierTemps() == 1 || moche == 0){

		for(int i = 0; i<NBPROCESSUS;i++){
			if(date == tabDate[i]){
				P(0);
				ajouterFin(prioriteProcessus[i],tabPid[i],tempsProcessus[i]);
				V(0);
				moche = 1;
			}
		}
		int file = roundRobin[date%taille];
		for(int i =0; i<TAILLETAB; i++%taille){
			if(filePriorite[file] != NULL){
				retirerDebut(file);
				i = TAILLETAB;
			}
			file = (file+1)%TAILLETAB;
		}
		//P(1);
		sleep(quantum);
		date ++;
	}
	
}



int main(int argc, char const *argv[])
{
	if(argc > 1 && strcmp ("-t", argv[1]) != 0){
		//différentes options de lancement : pour l'instant il n'y a que -t
		printf("Vous n'avez pas entré d'options valides. Options disponibles :\n -t : afficher ou modifier la table d'allocation CPU\n");
		return(1);
	}

	for (int i = 0; i < TAILLETAB; ++i)
	{
		priorites[i] = 100/TAILLETAB;
	}

	//Si il y a l'argument '-t', on affiche la table d'allocation CPU
	if(argc == 2 && strcmp ("-t", argv[1]) == 0){
		modifierTableau();
	}
	int pgcd = superPGCD(priorites);
	int* roundRobin = genereRoundRobin(100/pgcd);
	
	genereProcessus();
	
	tourniquet(roundRobin,pgcd);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(sem_id, 0, IPC_RMID, NULL);
	return 0;
}

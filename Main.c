#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/sem.h>
#include <errno.h>
#include <time.h>
#include <sys/shm.h>

#define quantum 200000
#define MAXCHAR 100
#define TAILLETAB 11 //nombre de priorités existantes
#define NBPROCESSUS 10 //nombre de processus qui vont être lancés

int tabPid[NBPROCESSUS]; //tableau contenant les PID des processus
int prioriteProcessus[NBPROCESSUS]; // tableau contenant les priorités des processus
int tempsProcessus[NBPROCESSUS]; // adresse de la zone de mémoire contenant les temps accordés aux processus (non fonctionnel)
int priorites[TAILLETAB]; //Table d'allocation des priorités (choisie avec des pourcentages égaux par défaut)
int tabDate[NBPROCESSUS]; // tableau contenant les dates d'exécution des processus

typedef struct elem {
	int valeur;
	int temps;
	struct elem*suivant;
}element; // structure d'élément de liste chainée contenant le pid et le temps d'execution d'un processus

typedef struct {
    long type;			
    pid_t pid;			
} treponse;// structure d'un message envoyé par la file de messages

typedef element * T;

T filePriorite[TAILLETAB]; //files de priorité
int msgid,isRandom;

int isProcFinished(int pid){

	for(int i = 0; i < TAILLETAB; i++){
		if(filePriorite[i] != NULL){
			T temp = filePriorite[i];
			while(temp != NULL){
				if(temp->valeur == pid){
					return 1;
				}
				temp = temp->suivant;

			}
		}
	}
	return 0;
}

void lireDonnes(){
	FILE *fp;
    char str[MAXCHAR];
    char* filename = "donnees.txt";
 
    fp = fopen(filename,"r");
    if (fp == NULL){
        printf("Erreur d'ouverture de fichier %s",filename);
        exit(1);
    }
	int donnees[TAILLETAB];
	fscanf (fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &donnees[0],&donnees[1],&donnees[2],&donnees[3],&donnees[4],&donnees[5],&donnees[6],&donnees[7],&donnees[8],&donnees[9],&donnees[10]);
	for(int i =0; i<TAILLETAB;i++){
		priorites[i] = donnees[i];
	}
	fscanf (fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &donnees[0],&donnees[1],&donnees[2],&donnees[3],&donnees[4],&donnees[5],&donnees[6],&donnees[7],&donnees[8],&donnees[9]);
	for(int i =0; i<NBPROCESSUS;i++){
		prioriteProcessus[i] = donnees[i];
	}
	fscanf (fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &donnees[0],&donnees[1],&donnees[2],&donnees[3],&donnees[4],&donnees[5],&donnees[6],&donnees[7],&donnees[8],&donnees[9]);
	for(int i =0; i<NBPROCESSUS;i++){
		tabDate[i] = donnees[i];
	}
	fscanf (fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &donnees[0],&donnees[1],&donnees[2],&donnees[3],&donnees[4],&donnees[5],&donnees[6],&donnees[7],&donnees[8],&donnees[9]);
	for(int i =0; i<NBPROCESSUS;i++){
		tempsProcessus[i] = donnees[i];
	}
    fclose(fp);
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
			//teste si les valeurs sont correctes. Si oui, on applique les modifications
			int somme= 0;
			int isZero = 1;
			for(int i =0; i<TAILLETAB; i++){
				somme+=prioriteTemp[i];
				if (prioriteTemp[i] == 0 || prioriteTemp[i] < 0){
					isZero = 0;
				}
			}
			if(somme == 100 && isZero == 1){
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

//renvoie le PGCD de tous les entiers du tableau tab
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
	//génère un tableau contenant le nombre de fois que chaque priorité doit apparaitre dans le tourniquet
	for (int i = 0; i < TAILLETAB; ++i)
	{	
		compteur[i] =priorites[i]*taille/100;
	}
	//remplit le tableau selon les priorités choisies
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
	printf("———————————————————————————————————————————————————————————————————————\n");
	printf("| pid		| priorite	  | date arrivee	|temps	      |\n");
	int pid,key;
	char buffer [200];
	//création d'une file de messages dont l'id sera transmis aux processus
	if ((key = ftok("main", 'A')) == -1) {
		perror("Erreur de creation de la clé \n");
		exit(1);
	}
    if ((msgid = msgget(key, 0750 | IPC_CREAT )) == -1) {
		perror("Erreur de creation de la file\n");
		exit(1);
    }
	//création des processus
	srand (time (NULL));
	for (int i = 0; i < NBPROCESSUS; ++i)
	{
		if(isRandom == 1){
			prioriteProcessus[i] = (int)(rand() / (double)RAND_MAX * TAILLETAB-1);
			tempsProcessus[i] =1+ (int)(rand() / (double)RAND_MAX * 10);
			tabDate[i] = (int)(rand() / (double)RAND_MAX * TAILLETAB-1);
		}
		pid = fork();
		tabPid[i] = pid;
		if(pid == 0){
			sprintf(buffer, "%d %d",msgid,tempsProcessus[i]);
			if (execl("./processus","processus",buffer,(char*)NULL) == -1){
				printf("erreur de execl\n");
	       		exit(1);
			}
		}
		else {
			printf("|%d		|%d		  |%d			|%d	      |\n",pid, prioriteProcessus[i],tabDate[i],tempsProcessus[i]);
		}
	}
	printf("———————————————————————————————————————————————————————————————————————\n");
	return;
}

//vérifie qu'il reste toujours des processus demandant du temps de CPU (renvoie 0 si faux)
int verifierTemps(){
	int reponse = 0;
	for(int i = 0; i<TAILLETAB; i++){
		if(filePriorite[i] != NULL){
			reponse = 1;
		}
	}
	return reponse;
}

//ajoute le processus de pid pidProcessus à la file de priorite priorite (avec le temps d'éxecution temps)
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

//ajoute un caractère à une chaîne de caractères
int  append(char*s, char c) {
    int len = strlen(s);
    s[len] = c;
    s[len+1] = '\0';
    return 0;
}

//permet à un processus de s'éxecuter pendant un quantum de temps et retire le processus de sa file de priorité actuelle pour le "décaler"
void retirerDebut(int priorite,int date){
	T m;
	int nouvellePrio;
	if(filePriorite[priorite]!=NULL)
	{
		m = filePriorite[priorite]->suivant;
		if(priorite == TAILLETAB-1){
			nouvellePrio = 0;
		} else {
			nouvellePrio = priorite+1;
		}
		//envoi d'un message au processus pour qu'il s'éxecute
		treponse requete;
		requete.pid = getpid();
		requete.type = filePriorite[priorite]->valeur;
		filePriorite[priorite]->temps --;
		msgsnd(msgid,&requete,sizeof(treponse)-4,0);

		char* message = malloc(100);
		free(message);
		message = malloc(100);
		if(date < 10){
			printf("%d    ",date);
		} else {
			printf("%d   ",date);
		}
		
		for(int i = 0; i<NBPROCESSUS;i++){
			if(tabPid[i] == requete.type){
				append(message,'|');
				append(message,'0');
				append(message,' ');
			} else if (date < tabDate[i] || isProcFinished(tabPid[i]) == 0){
				append(message,'|');
				append(message,' ');
				append(message,' ');
			} else {
				append(message,'|');
				append(message,'*');
				append(message,' ');
			}
		}
		append(message,'|');
		printf("%s\n",message);
		free(message);

		if(filePriorite[priorite]->temps !=0){//décale le processus d'une file de message uniquement s'il lui reste du temps d'éxecution
			ajouterFin(nouvellePrio,filePriorite[priorite]->valeur,filePriorite[priorite]->temps);
		}
		free(filePriorite[priorite]);
		filePriorite[priorite] = m;
	}
}

//tourniquet qui va distribuer du temps de processeur entre les divers processus
int tourniquet(int* roundRobin,int pgcd){
	printf("**********************************************\n");
	printf("Début du tourniquet\n\n");
	printf("date |P0|P1|P2|P3|P4|P5|P6|P7|P8|P9|\n");
	fflush(stdout);

	int date = 0;
	int taille = 100/pgcd;
	int conditionProcessus = 0;
	while(verifierTemps() == 1 || conditionProcessus == 0){

		//teste si un processus doit être ajouté au gestionnaire
		for(int i = 0; i<NBPROCESSUS;i++){
			if(date == tabDate[i]){
				ajouterFin(prioriteProcessus[i],tabPid[i],tempsProcessus[i]);
				conditionProcessus = 1;
			}
		}
		//choisit le bon processus à exécuter et l'exécute
		int file = roundRobin[date%taille];
		for(int i =0; i<TAILLETAB; i++%taille){
			if(filePriorite[file] != NULL){
				retirerDebut(file,date);
				i = TAILLETAB;
			}
			file = (file+1)%TAILLETAB;
		}
		usleep(quantum);
		date ++;
	}
	
}

int main(int argc, char const *argv[])
{
	isRandom = 1;
	if(argc > 1 && (strcmp ("-t", argv[1]) != 0 && strcmp ("-f", argv[1]) != 0)){
		//différentes options de lancement : pour l'instant il n'y a que -t
		printf("Vous n'avez pas entré d'options valides. Options disponibles :\n
		 -t : afficher ou modifier la table d'allocation CPU\n
		 -f : utiliser les données de donnees.txt au lieu de données aléatoires\n");
		return(1);
	}

	//on définit des priorités plus ou moins égales par défaut
	for (int i = 0; i < TAILLETAB-1; ++i)
	{
		priorites[i] = 9;
	}
	priorites[TAILLETAB-1] = 10;

	//Si il y a l'argument '-t', on affiche la table d'allocation CPU
	//Si il y a l'argument '-f', on lance le programme avec les données de test

	if(argc == 2)
		if(strcmp ("-t", argv[1]) == 0){
		modifierTableau();
	} else if (strcmp ("-f", argv[1]) == 0){
		lireDonnes();
		isRandom = 0;
	}
	//on trouve le pgcd des priorités définies pour déterminer la taille du tourniquet (qui sera de 100/pgcd)
	int pgcd = superPGCD(priorites);

	//on génère le tourniquet
	int* roundRobin = genereRoundRobin(100/pgcd);
	
	genereProcessus();
	
	tourniquet(roundRobin,pgcd);
	msgctl(msgid, IPC_RMID,NULL);
	return 0;
}
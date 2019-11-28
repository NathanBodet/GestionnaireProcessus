#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define TAILLETAB 10
#define NBPROCESSUS 10

int tabPid[NBPROCESSUS];
int prioriteProcessus[NBPROCESSUS];
int tempsProcessus[NBPROCESSUS];
int priorites[TAILLETAB]; //Table d'allocation des priorités (choisie avec ces pourcentages-là par défaut)
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
			if(prioriteTemp[0] + prioriteTemp[1] + prioriteTemp[2] + prioriteTemp[3] == 100){
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

int superPGCD(int tab[]){
	int pgcd = tab[0];
	for (int i = 0; i < TAILLETAB-1; ++i)
	{
		pgcd = PGCD(pgcd,tab[i+1]);
	}
	return pgcd;
}

int* genereRoundRobin(int taille){
	int *round = malloc(taille * sizeof(int));
	int compteur[taille];
	int increment = 0;
	for (int i = 0; i < TAILLETAB; ++i)
	{	
		compteur[i] = priorites[i]/taille;
	}
	for (int i = 0; i < taille; ++i)
	{
		while(compteur[increment] == 0){
			increment++;
			if(increment > TAILLETAB){
				increment = 0;
			}
		}
		round[i] = increment;
		compteur[increment]--;
		increment++;
	}
	return round;
}

int genereProcessus(int nombre,char* arg){
	int pid,key;
	int msgid;
	char buffer [100];
	if ((key = ftok("main.exe", 'A')) == -1) {
		perror("Erreur de creation de la clé \n");
		exit(1);
    }
    if ((msgid = msgget(key, 0750 | IPC_CREAT )) == -1) {
		perror("Erreur de creation de la file\n");
		exit(1);
    }
    sprintf(buffer, "%d",msgid);

	for (int i = 0; i < NBPROCESSUS; ++i)
	{
		pid = fork();
		tabPid[i] = pid;
		prioriteProcessus[i] = i;
		tempsProcessus[i] = rand();
		printf("%d\n", tempsProcessus[i]);
		if(pid == 0){
			if (execl("test","test",buffer,NULL) == -1){
				printf("erreur de execl\n");
	       		fflush(stdout);
			}
		}
	}
	return msgid;
}

int tourniquet(int msgid){

}

int main(int argc, char const *argv[])
{
	if(argc == 1){
		//différentes options de lancement : pour l'instant il n'y a que -t
		printf("Vous n'avez pas entré d'options. Options disponibles :\n -t : afficher ou modifier la table d'allocation CPU\n");
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
	printf("%s\n", argv[0]);
	tourniquet(genereProcessus(10,argv[0]));
	return 0;
}

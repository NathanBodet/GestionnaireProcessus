#include <stdio.h>
#include <string.h>
int main(int argc, char const *argv[])
{
	char reponse[8];
	char *o = "o";
	char *n = "n";
	//Table d'allocation des priorités (choisie avec ces pourcentages-là par défaut)
	int priorites[4] = {40,30,20,10};

	//Si il y a un argument, on affiche la table d'allocation (il faut tester pour que ça soit l'argrument 't', j'ai pas encore fait)
	if(argc == 2){
		printf("****************************\nTableau des priorités:\n");
		printf("———————————————————————————————————\n|numéro de priorité | pourcentage |\n");
		for (int i = 0; i < 4; ++i)
		{
			printf("| %d                 | %d          |\n",i,priorites[i]);
		}
		printf("———————————————————————————————————\n");

		printf("\nVoulez-vous modifier le tableau des priorités? (o/n)\n");
		fflush(stdout);
		//entrée de l'utilisateur, test de réponse qui marche pas encore :/
		/*while(reponse != o && reponse != n){
			scanf("%s",reponse);//on récupère l'input de l'utilisateur

			if(reponse != o && reponse != n){
				printf("entrée incorrecte : entrez o pour oui et n pour non\n");
				fflush(stdout);
			}
		}*/
		

		return 0;
	}
	
	
	return 0;
}
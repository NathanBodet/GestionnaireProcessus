#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <string.h>

//structure recevant le message de la file de messages
typedef struct {
    long type;			
    pid_t pid;			
} treponse;

int main(int argc, char *argv[])
{
	//récupération des arguments : id de la file de messages, id du semaphore, temps demandé par ce processus
	char* args = strtok(argv[1]," ");
	treponse reponse;
	int msgid = atoi(&args[0]);
	int temps = atoi(&args[1]);

	//boucle dans laquelle on va attendre un message du gestionnaire pour s'éxecuter pendant 1 quantum de temps
	for(int i = 0; i<temps; i++){
		msgrcv(msgid, &reponse, sizeof(treponse)-4,getpid(),0);

	}

	return 0;
}
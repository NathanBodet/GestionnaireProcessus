#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <string.h>


int sem_id ;
int* tempsProcessus;
struct sembuf sem_oper_P ;  /* Operation P */
struct sembuf sem_oper_V ; /* Operation V */

typedef struct {
    long type;			
    pid_t pid;			
} treponse;

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

int main(int argc, char *argv[])
{
	char* args = strtok(argv[1]," ");
	treponse reponse;
	int msgid = atoi(&args[0]);
	sem_id = atoi(&args[1]);
	int temps = atoi(&args[2]);

	for(int i = 0; i<temps; i++){
		msgrcv(msgid, &reponse, sizeof(treponse)-4,getpid(),0);
		printf("decrementation : %d quantums ont été effectués\n",i);
		fflush(stdout);
		//V(1);
	}

	return 0;
}
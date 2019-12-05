#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main(int argc, char const *argv[])
{
	int reponse;
	int msgid = atoi(argv[1]);
	msgrcv(msgid, &reponse, sizeof(int),getpid(),0);
	return 0;
}
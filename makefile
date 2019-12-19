all:
	gcc -o processus Processus.c
	gcc -o main Main.c

clean:
	rm -f .nfs*
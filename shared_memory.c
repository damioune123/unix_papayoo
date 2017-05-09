#include "shared_memory.h"
key_t key = 5678;

int semaphore;
semaphore mutex = 1;			// controle d'acces a "rc"
semaphore mem = 1;			// controle d'acces a la memoire partagee
int rc = 0;				// nombre de processus lisant ou voulant lire

// TODO add down and up functions
// returns a field from the shared memory
void* s_read(int code){
	void* ret;
	down(&mutex);			// get exclusive access to rc
	rc = rc + 1;			// un lecteur de plus
	if(rc == 1) down(&mem);		// si c'est le premier lecteur...
	up(&mutex);			// liberer l'acces exclusif a rc
	// TODO lire données, operation critique
	switch(code){
		case CODE_READ_NAMES:
			ret = locate_segment().names;	
			break;
		case CODE_READ_SCORES:
			ret = locate_segment().scores;
			break;
		case CODE_READ_CURRENT_CARDS:
			// TODO complete once card structure has been implemented
			break;
		default:
			fprintf(stderr, "Incorrect read operation code\n");
			break;
	}
	down(&mutex);			// obtenir l'acces exclusif a rc
	rc = rc - 1			// un lecteur de moins
		if(rc == 0) up(&mem);		// si c'est le dernier lecteur
	up(&mutxe);			// liberer l'acces exclusif a rc
	return ret; // return the extracted data
}

void s_write(void* s_mem){
	// TODO créer données, operation non critique
	down(&mem);			// obtenir acces exclusif
	// TODO ecrire données, operation critique
	up(&mem);			// liberer acces exclusif
}


void* create_segment(){
	char *shm;
	int shmid;
	if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(EXIT_FAILURE);
	}

	/*
	 * Now we attach the segment to our data space.
	 */
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("shmat");
		exit(1);
	}
	return shm;
}

void* locate_segment(){
	int shmid;
	char *shm;
	/*
	 * Locate the segment.
	 */
	if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	/*
	 * Now we attach the segment to our data space.
	 */
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("shmat");
		exit(1);
	}
	return shm;
}


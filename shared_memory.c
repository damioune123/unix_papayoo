#include "shared_memory.h"
key_t key = 5678;
s_mem *board;

struct sembuf sop;
int sem_id_mutex_mem; // locking shared memory access
int sem_id_mutex_rc; // locking shared reader count access
int * rc;// amount of readers

// TODO add down and up functions
// returns a field from the shared memory
// TODO display extracted information instead of returning it?
void *s_read(int code){
	void * ret = NULL;
	down(sem_id_mutex_rc);			// get exclusive access to rc
	*rc = *rc + 1;			// one more reader
	if(*rc == 1) down(sem_id_mutex_mem);		// if first reader...
	up(sem_id_mutex_rc);			// release exclusive access to rc
	// TODO read data, critical operation
	switch(code){
		case CODE_READ_NAMES:
			ret = getNames();
			break;
		case CODE_READ_SCORES:
			ret = getScores();
			break;
		case CODE_READ_CURRENT_CARDS:
			// TODO complete once card structure has been implemented
			break;
		default:
			fprintf(stderr, "Incorrect read operation code\n");
			break;
	}
	down(sem_id_mutex_rc);			// get exclusive access to rc
	*rc = *rc - 1			// one less reader
	if(*rc == 0) up(mem);	// if last reader
	up(sem_id_mutex_rc);			// release exclusive access to rc
	return ret;

}

void s_write(void* data){
	// TODO create data, non critical operation
	down(sem_id_mem);			// get exclusive access
	// TODO write data, critical operation
	up(sem_id_mem);			// release exclusive access
}

// ONLY use when mutex to access board memory is down
// returns a two dimentional table containing the names of all players
char** getNames(){
	char[4][255] ret;
	memcpy(ret, board.names, sizeof(ret));
	return ret;
}

// ONLY use when mutex to access rc is down
// returns a table containing the scores of all players
int* getScores(){
	int[4] ret;
	memcpy(ret, board.scores, sizeof(ret));
	return ret;
}

void create_segment(){
	int shmid_mem;
	int shmid_rc;
	if ((shmid_mem = shmget(key, sizeof(s_mem), IPC_CREAT | 0666)) < 0) {
		perror("shmget s_mem");
		exit(EXIT_FAILURE);
	}
	if ((shmid_rc = shmget(key, sizeof(int), IPC_CREAT | 0666)) < 0) {
		perror("shmget rc");
		exit(EXIT_FAILURE);
	}


	/*
	 * Now we attach the segment to our data space.
	 */
	if ((board = shmat(shmid_mem, NULL, 0)) == (s_mem *) -1) {
		perror("shmat mem");
		exit(EXIT_FAILURE);
	}
	if ((rc = shmat(shmid_rc, NULL, 0)) == (int *) -1) {
		perror("shmat rc");
		exit(EXIT_FAILURE);
	}

}
void init_semaphores(){
	if( (sem_id_mutex_mem = semget(TOKEN_MUT_MEM,1,IPC_CREAT|0666)) < 0 )//TO DO nsems
	{	perror("semget");
		exit(1);
	}	
	if( (sem_id_mutex_rc = semget(TOKEN_MUT_RC,1,IPC_CREAT|0666)) < 0 )//TO DO nsems
	{	perror("semget");
		exit(1);
	}	
}
void down(int semid){
	sop.sem_op = -1;
	if( semop(semid,&sop,1) < 0 )
	{	perror("Error down semaphore");
		exit(EXIT_FAILURE);
	}
}
void up(int semid){
	sop.sem_op = -1;
	if( semop(semid,&sop,1) < 0 )
	{	perror("Error up semaphore");
		exit(EXIT_FAILURE);
	}
}

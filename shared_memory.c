#include "shared_memory.h"
s_mem *board;

struct sembuf sop[1];
int sem_id_mutex_mem; // locking shared memory access
int sem_id_mutex_rc; // locking shared reader count access
int * rc;// amount of readers

// TODO add down and up functions
// returns a field from the shared memory
// TODO display extracted information instead of returning it?
void s_read(int code, int idx, void **data){
	void * ret = NULL;
	down(sem_id_mutex_rc);			// get exclusive access to rc
	*rc = *rc + 1;			// one more reader
	if(*rc == 1) down(sem_id_mutex_mem);		// if first reader...
	up(sem_id_mutex_rc);			// release exclusive access to rc
	// TODO read data, critical operation
	switch(code){
		case CODE_READ_NAMES:
			memcpy((char **)*data, board->names, sizeof(board->names));
			break;
		case CODE_READ_SCORES:
			memcpy((int *)*data, board->scores, sizeof(board->scores));
			break;
		case CODE_READ_CURRENT_CARDS:
			// TODO complete once card structure has been implemented
			break;
		default:
			fprintf(stderr, "Incorrect read operation code\n");
			break;
	}
	down(sem_id_mutex_rc);			// get exclusive access to rc
	*rc = *rc - 1;			// one less reader
	if(*rc == 0) 
            up(sem_id_mutex_mem);	// if last reader
	up(sem_id_mutex_rc);			// release exclusive access to rc

}

void s_write(int code,int idx, void* data){
        printf("debug s_write %s\n", data);
	down(sem_id_mutex_mem);			// get exclusive access
        switch(code){
            case CODE_WRITE_NAME :
                strcpy(board->names[idx],(char *)data);
                break;
            case CODE_WRITE_SCORE:
                board->scores[idx]=(int) data;
                break;
            default:
                fprintf(stderr, "Incorrect write operation code\n");
                break;
        }
	up(sem_id_mutex_mem);			// release exclusive access

}


void create_segment(){
	int shmid_mem;
	int shmid_rc;
	if ((shmid_mem = shmget(TOKEN_SEG_MEM, sizeof(s_mem), IPC_CREAT | 0666)) < 0) {
		perror("shmget s_mem");
		exit(EXIT_FAILURE);
	}
	if ((shmid_rc = shmget(TOKEN_SEG_RC, sizeof(int), IPC_CREAT | 0666)) < 0) {
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
        sop[0].sem_num=0;
        sop[0].sem_op=1;
        sop[0].sem_flg=0;
	if( semop(sem_id_mutex_mem,sop,1) < 0 )
	{	perror("Error init semaphore");
		exit(EXIT_FAILURE);
	}
	if( semop(sem_id_mutex_rc,sop,1) < 0 )
	{	perror("Error down semaphore");
		exit(EXIT_FAILURE);
	}

}
void down(int semid){
	sop[0].sem_op = -1;
	if( semop(semid,sop,1) < 0 )
	{	perror("Error down semaphore");
		exit(EXIT_FAILURE);
	}
}
void up(int semid){
	sop[0].sem_op = 1;
	if( semop(semid,sop,1) < 0 )
	{	perror("Error up semaphore");
		exit(EXIT_FAILURE);
	}
}

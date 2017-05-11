#include "shared_memory.h"
#include <unistd.h>
s_mem *board;

struct sembuf sop[1];
int sem_id_mutex_mem; // locking shared memory access
int sem_id_mutex_rc; // locking shared reader count access
int * rc;// amount of readers

void s_read_scores(int **data){
	void * ret = NULL;
	down(sem_id_mutex_rc);			// get exclusive access to rc
	*rc = *rc + 1;			// one more reader
	if(*rc == 1) down(sem_id_mutex_mem);		// if first reader...
	up(sem_id_mutex_rc);			// release exclusive access to rc
	// TODO read data, critical operation
        printf("Enter read scores crticial\n");
        memcpy(data,board->scores, sizeof(board->scores)); 
	down(sem_id_mutex_rc);			// get exclusive access to rc
	*rc = *rc - 1;			// one less reader
	if(*rc == 0) 
            up(sem_id_mutex_mem);	// if last reader
	up(sem_id_mutex_rc);			// release exclusive access to rc
        printf("Leave read scores crticial\n");

}

void s_read_names(char **data){
	void * ret = NULL;
	down(sem_id_mutex_rc);			// get exclusive access to rc
	*rc = *rc + 1;			// one more reader
	if(*rc == 1) down(sem_id_mutex_mem);		// if first reader...
	up(sem_id_mutex_rc);			// release exclusive access to rc
	// TODO read data, critical operation
        memcpy(data,board->names, sizeof(board->names)); 
	down(sem_id_mutex_rc);			// get exclusive access to rc
	*rc = *rc - 1;			// one less reader
	if(*rc == 0) 
            up(sem_id_mutex_mem);	// if last reader
	up(sem_id_mutex_rc);			// release exclusive access to rc

}
void s_write_name(int idx, char* data){
    down(sem_id_mutex_mem);			// get exclusive access
    printf("Enter write name critical\n");
    sleep(6);
    strcpy(board->names[idx],data);
    up(sem_id_mutex_mem);			// release exclusive access
    printf("Leave write name critical\n");

}

void s_write_score(int idx, int data){
    down(sem_id_mutex_mem);			// get exclusive access
    board->scores[idx]=data;
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
    printf("shmid serveur %i\n", shmid_mem);

}
void locate_segment(){
    int shmid_mem;
    int shmid_rc;
    if ((shmid_mem = shmget(TOKEN_SEG_MEM, sizeof(s_mem), 0666)) < 0) {
        perror("shmget s_mem");
        exit(EXIT_FAILURE);
    }
    if ((shmid_rc = shmget(TOKEN_SEG_RC, sizeof(int), 0666)) < 0) {
        perror("shmget rc");
        exit(EXIT_FAILURE);
    }
    printf("shmid joueur %i\n", shmid_mem);
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
        exit(EXIT_FAILURE);
    }	
    if( (sem_id_mutex_rc = semget(TOKEN_MUT_RC,1,IPC_CREAT|0666)) < 0 )//TO DO nsems
    {	perror("semget");
        exit(EXIT_FAILURE);
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
void locate_semaphores(){
    if( (sem_id_mutex_mem = semget(TOKEN_MUT_MEM,1,0666)) < 0 )//TO DO nsems
    {	perror("semget");
        exit(EXIT_FAILURE);
    }	
    if( (sem_id_mutex_rc = semget(TOKEN_MUT_RC,1,0666)) < 0 )//TO DO nsems
    {	perror("semget");
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

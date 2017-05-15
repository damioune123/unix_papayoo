/**
 *
 *  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie 2)
 *  This file contains all the  implemented functions needed for ipc handling
 *
 */
#include "ipc.h"
static s_mem *board;//the pointer to the structure used to read and write stored in the shared memory
static int * rc; // pointer to the amount of readers stored in shared memory
static struct sembuf sop;// this is a structure used to make operation on the System V's semaphores (mainly up and down)
static int sem_id_mutex_mem; // the identifier of the mutex to lock access the s_mem var (board)  in shared memory
static int sem_id_mutex_rc; // the identifier of the mutex to lock access to the amount or readers (rc)  of the s_mem var stored in shared memory
static int shmid_mem; //the shmid identifier of the s_mem var (board) stored in shared_memory
static int shmid_rc;//the shmid identifier of the amount of reader (rc) stored in shared_memory
/**
 *
 * This function is used to read the array of scores contained in the board var stored in shared memory. Access is protected with Courtois's semaphores algorithm.
 *  Many users can read the s_mem data at once, but only one writer is allowed at once.
 *
 *  @param int ** data : This is the pointer to the array of int that is going to be filled with scores array.
 *
 */
void s_read_scores(int **data){
    void * ret = NULL;
    down(sem_id_mutex_rc);			// get exclusive access to rc
    *rc = *rc + 1;			// one more reader
    if(*rc == 1) down(sem_id_mutex_mem);		// if first reader...
    up(sem_id_mutex_rc);			// release exclusive access to rc
    memcpy(data,board->scores, sizeof(board->scores));
    down(sem_id_mutex_rc);			// get exclusive access to rc
    *rc = *rc - 1;			// one less reader
    if(*rc == 0)
        up(sem_id_mutex_mem);	// if last reader
    up(sem_id_mutex_rc);			// release exclusive access to rc
}
/**
 *
 * This function is used to read the array of players' names contained in the board var stored in shared memory. Access is protected with Courtois's semaphores algorithm.
 *  Many users can read the s_mem data at once, but only one writer is allowed at once.
 *
 *  @param char ** data : This is the array  of char * that is going to be filled with names' array.
 */
void s_read_names(char **data){
    void * ret = NULL;
    down(sem_id_mutex_rc);			// get exclusive access to rc
    *rc = *rc + 1;			// one more reader
    if(*rc == 1) down(sem_id_mutex_mem);		// if first reader...
    up(sem_id_mutex_rc);			// release exclusive access to rc
    memcpy(data,board->names, sizeof(board->names));
    down(sem_id_mutex_rc);			// get exclusive access to rc
    *rc = *rc - 1;			// one less reader
    if(*rc == 0)
        up(sem_id_mutex_mem);	// if last reader
    up(sem_id_mutex_rc);			// release exclusive access to rc
}
/**
 *
 * This function is used to read the array of cards of current pli contained in the board var stored in shared memory. Access is protected with Courtois's semaphores algorithm.
 *  Many users can read the s_mem data at once, but only one writer is allowed at once.
 *
 *  @param card ** data : This is the array  of card * that is going to be filled with cards' array.
 */
void s_read_cards(card **data){
    void * ret = NULL;
    down(sem_id_mutex_rc);			// get exclusive access to rc
    *rc = *rc + 1;			// one more reader
    if(*rc == 1) down(sem_id_mutex_mem);		// if first reader...
    up(sem_id_mutex_rc);			// release exclusive access to rc
    memcpy(data,board->pli, sizeof(board->pli));
    down(sem_id_mutex_rc);			// get exclusive access to rc
    *rc = *rc - 1;			// one less reader
    if(*rc == 0)
        up(sem_id_mutex_mem);	// if last reader
    up(sem_id_mutex_rc);			// release exclusive access to rc
}
/**
 *
 * This function is used to write a name at an known index in the array of names stored in the board var (in shared memory)
 *
 * @param int idx : the index of the player 
 *
 * @param char * data : the name of the player
 *
 */
void s_write_name(int idx, char* data){
    if(idx<0 || idx >= MAX_PLAYERS){
        fprintf(stderr, "Wrong index\n");
        exit(EXIT_FAILURE);
    }
    down(sem_id_mutex_mem);			// get exclusive access
    strcpy(board->names[idx],data);
    up(sem_id_mutex_mem);			// release exclusive access
}

/**
 *
 * This function is used to write a score at an known index in the array of scores stored in the board var (in shared memory)
 *
 * @param int idx : the index of the player 
 *
 * @param int data : the score of the player
 */
void s_write_score(int idx, int data){
    if(idx<0 || idx >= MAX_PLAYERS){
        fprintf(stderr, "Wrong index\n");
        exit(EXIT_FAILURE);
    }
    down(sem_id_mutex_mem);			// get exclusive access
    board->scores[idx]=data;
    up(sem_id_mutex_mem);			// release exclusive access
}
/**
 *
 * This function is used to write a card at an known index in the array of cards stored in the board var (in shared memory)
 *
 * @param int idx : the index of the player
 *
 * @param card data : the card of the player
 */
void s_write_card(int idx, card data){
    if(idx<0 || idx >= MAX_PLAYERS){
        fprintf(stderr, "Wrong index\n");
        exit(EXIT_FAILURE);
    }
    down(sem_id_mutex_mem);			// get exclusive access
    memcpy(&board->pli[idx],&data, sizeof(card));
    up(sem_id_mutex_mem);			// release exclusive access
}
/**
 *
 * This function ought to be used by server side only and aims to create segments in shared memory both for the structure s_mem (containing names, scores and cards of the current "pli" ) and the amount of players also stored in memory.
 * After creating the segments, both segment shmid identifiers (shmid_mem and shmid_rc) are initialized for the server and the board and rc  pointers refer to the shared memories created.
 *
 *
 */

void create_segment(){
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
/**
 *
 * This function ought to be used by client side only when all the shared memories segments have already been created with the void create_segement() function.
 * It allows the current player to init the static vars of this file : shmid_mem, shmid_rc, board and rc with the same values of the ones created during the creation of the segments.
 *
 */
void locate_segment(){
    if ((shmid_mem = shmget(TOKEN_SEG_MEM, sizeof(s_mem), 0666)) < 0) {
        perror("shmget s_mem");
        exit(EXIT_FAILURE);
    }
    if ((shmid_rc = shmget(TOKEN_SEG_RC, sizeof(int), 0666)) < 0) {
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
/**
 *
 * This function ought to be used by server side only and aims to create the System V's semaphores necessary to lock access to the segments of shared memory pointed by board and rc.
 * The static vars of this file : sem_id_mutex_mem (to protect access to board) and sem_id_mutex_rc (to protect access to rc) are initalized with the identifiers of the newly created semaphores.
 * Both semaphores are initialized with a value of 1.
 *
 */
void init_semaphores(){
    if( (sem_id_mutex_mem = semget(TOKEN_MUT_MEM,1,IPC_CREAT|0666)) < 0 )
    {	perror("semget");
        exit(EXIT_FAILURE);
    }
    if( (sem_id_mutex_rc = semget(TOKEN_MUT_RC,1,IPC_CREAT|0666)) < 0 )
    {	perror("semget");
        exit(EXIT_FAILURE);
    }
    semctl(sem_id_mutex_mem, 0, SETVAL, 1);
    semctl(sem_id_mutex_rc, 0, SETVAL, 1);
}
/**
 *
 * This function ought to be used by client side only.It allows the current player to init the static vars of this file : sem_id_mutex_mem (to protect access to board)  and sem_id_mutex_rc (to protect access to rc) with the identifiers of the semaphores created by the server.
 *
 *
 */
void locate_semaphores(){
    if( (sem_id_mutex_mem = semget(TOKEN_MUT_MEM,1,0666)) < 0 )
    {	perror("semget");
        exit(EXIT_FAILURE);
    }
    if( (sem_id_mutex_rc = semget(TOKEN_MUT_RC,1,0666)) < 0 )
    {	perror("semget");
        exit(EXIT_FAILURE);
    }
}
/**
 * This function allows to do a standard down operation on a System V's semaphore with its identifier.
 *
 * @param int semid : the identifer of the semaphore.
 *
 */
void down(int semid){
    sop.sem_op = -1;
    if( semop(semid,&sop,1) < 0 )
    {	perror("Error down semaphore");
        exit(EXIT_FAILURE);
    }
}
/**
 *
 * This function allows to do a standard up operation on a System V's semaphore with its identifier.
 *
 * @param int semid : the identifer of the semaphore.
 *
 */
void up(int semid){
    sop.sem_op = 1;
    if( semop(semid,&sop,1) < 0 )
    {	perror("Error up semaphore");
        exit(EXIT_FAILURE);
    }
}
/**
 *
 * This function allows to kill all remaining ipcs (shared memory segments and semaphores) when the server program is shutting down.
 *
 */
void kill_ipcs(){
    semctl(sem_id_mutex_mem, 0, IPC_RMID, NULL);
    semctl(sem_id_mutex_rc, 0, IPC_RMID, NULL);
    shmctl(shmid_mem, IPC_RMID, NULL);
    shmctl(shmid_rc, IPC_RMID, NULL);
}

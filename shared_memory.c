#include "shared_memory.h"
key_t key = 5678;

int semaphore;
semaphore mutex = 1;			// access control to a rc
semaphore mem = 1;			// access control to shared memory
int rc = 0;				// number of processes wanting to read

// TODO add down and up functions
// returns a field from the shared memory
// TODO display extracted information instead of returning it?
void* s_read(int code){
	void* ret;
	down(&mutex);			// get exclusive access to rc
	rc = rc + 1;			// one more reader
	if(rc == 1) down(&mem);		// if first reader...
	up(&mutex);			// release exclusive access to rc
	// TODO read data, critical operation
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
	down(&mutex);			// get exclusive access to rc
	rc = rc - 1			// one less reader
		if(rc == 0) up(&mem);	// if last reader
	up(&mutxe);			// release exclusive access to rc
	return ret; // return the extracted data
}

void s_write(void* s_mem){
	// TODO create data, non critical operation
	down(&mem);			// get exclusive access
	// TODO write data, critical operation
	up(&mem);			// release exclusive access
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


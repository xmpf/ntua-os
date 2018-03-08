/*
 * simplesync.c
 *
 * A simple synchronization exercise.
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 * Operating Systems course, ECE, NTUA
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/* 
 * POSIX thread functions do not return error numbers in errno,
 * but in the actual return value of the function call instead.
 * This macro helps with error reporting in this case.
 */
#define perror_pthread(ret, msg) \
	do { errno = ret; perror(msg); } while (0)

#define MUTEX_LOCK(MUTEX_ADDR) \
	do { \
		if ( pthread_mutex_lock ( (MUTEX_ADDR) ) != 0 ) \
			perror ("mutex_lock"); \
	} while(0)

#define N 10000000
#define ATOMIC_FLAG 0

/* section: new-code */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int val = 0;
/* end-of-section */

/* Dots indicate lines where you are free to insert code at will */
/* ... */
#if defined(SYNC_ATOMIC) ^ defined(SYNC_MUTEX) == 0
# error You must #define exactly one of SYNC_ATOMIC or SYNC_MUTEX.
#endif

#if defined(SYNC_ATOMIC)
# define USE_ATOMIC_OPS 1
#else
# define USE_ATOMIC_OPS 0
#endif

void *increase_fn(void *arg)
{
	int i;
	// ip holds memory address of `val`
	volatile int *ip = arg;

	fprintf(stderr, "About to increase variable %d times\n", N);
	
	for (i = 0; i < N; i++) {
		if (USE_ATOMIC_OPS) {
			// atomic built-in load and increment 
			// __atomic_fetch_add (ip, 1, ATOMIC_FLAG);
			__sync_fetch_and_add (ip, 1);
			//++(*ip);
			
		} else {
			// acquire lock
			MUTEX_LOCK (&mutex);
			// critical section : increment 
			++(*ip);
			// release lock
			pthread_mutex_unlock (&mutex);
		}
	}
	fprintf(stderr, "Done increasing variable.\n");
	
	return NULL;
}

void *decrease_fn(void *arg)
{
	int i;
	volatile int *ip = arg;

	fprintf(stderr, "About to decrease variable %d times\n", N);
	for (i = 0; i < N; i++) {
		if (USE_ATOMIC_OPS) {
			// atomic built-in load and decrement
			// __atomic_fetch_sub (ip, 1, ATOMIC_FLAG);
			__sync_fetch_and_sub (ip, 1);
			//--(*ip);
			
		} else {
			// acquire lock
			MUTEX_LOCK (&mutex);
			// critical section : decrement
			--(*ip);
			// release lock
			pthread_mutex_unlock (&mutex);
		}
	}
	fprintf(stderr, "Done decreasing variable.\n");
	
	return NULL;
}


int main(int argc, char *argv[])
{
	int ret, ok;
	pthread_t t1, t2;


	/*
	 * Initial value
	 */
	val = 0;

	/*
	 * Create threads
	 * Thread 1: `t1` calls `increase_fn` on variable `val`
	 * Thread 2: `t2` calls `decrease_fn` on variable `val`
	 */
	
	ret = pthread_create(&t1, NULL, increase_fn, &val);	
	if (ret) {
		perror_pthread(ret, "pthread_create");
		exit(1);
	}


	ret = pthread_create(&t2, NULL, decrease_fn, &val);
	if (ret) {
		perror_pthread(ret, "pthread_create");
		exit(1);
	}

	/*
	 * Wait for threads to terminate
	 */
	ret = pthread_join(t1, NULL);
	if (ret)
		perror_pthread(ret, "pthread_join");
	ret = pthread_join(t2, NULL);
	if (ret)
		perror_pthread(ret, "pthread_join");

	/*
	 * Is everything OK?
	 */
	ok = (val == 0);

	printf("%sOK, val = %d.\n", ok ? "" : "NOT ", val);

	return ok;
}

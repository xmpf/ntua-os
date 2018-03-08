#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  5

/*
 * Create this process tree:
 * A-+-B---D
 *   `-C
 */
void fork_procs(void)
{
	int status;	
	pid_t b, c, d;
	/*
	 * initial process is A.
	 */

	printf("PID = %ld, name %s, starting...\n",
			(long)getpid(), "A");
	change_pname("A");
	
	b = fork();
	if (b < 0) {
		perror("main: fork");
		exit(EXIT_FAILURE);
	}
	if (b == 0) {
		/* Child */
		printf("PID = %ld, name %s, starting...\n",
			(long)getpid(), "B");
		change_pname("B");

		d = fork ();
		if (d < 0)  { perror ("fork"); exit (EXIT_FAILURE); }
		if (d == 0) { 
			printf("PID = %ld, name %s, starting...\n",
			(long)getpid(), "D");
			change_pname ("D");

			printf("D: Sleeping...\n");
			sleep(SLEEP_PROC_SEC);
			
			printf("D: exiting...\n");
			exit (13 /* 'D' */);
		}
		printf("B: Waiting for child %ld...\n", (long)d);
		wait (&status);
		explain_wait_status (d, status);

		printf("B: exiting...\n");
		exit(19 /* B */);
	}

	c = fork ();
	if (c < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (c == 0) {
		printf("PID = %ld, name %s, starting...\n",
			(long)getpid(), "C");
		change_pname ("C");

		printf("C: sleeping...\n");
		sleep(SLEEP_PROC_SEC + 2);
		
		printf("C: exiting...\n");
		exit (17 /* C */);
	}
	
	
	printf("A: Waiting for children...\n");
	//wait_for_ready_children (2);
	wait (&status);
	explain_wait_status (b, status);
	
	wait (&status);
	explain_wait_status (c, status);

	printf("A: Exiting...\n");
	exit(16 /*'A'*/);
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * How to wait for the process tree to be ready?
 * In ask2-{fork, tree}:
 *      wait for a few seconds, hope for the best.
 * In ask2-signals:
 *      use wait_for_ready_children() to wait until
 *      the first process raises SIGSTOP.
 */
int main(void)
{
	pid_t pid;
	int status;

	/* Fork root of process tree */
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Child */
		fork_procs();
		exit(1);
	}

	/* for ask2-{fork, tree} */
	sleep(SLEEP_TREE_SEC);

	/* Print the process tree root at pid */
	show_pstree(getpid());


	/* Wait for the root of the process tree to terminate */
	pid = wait(&status);
	explain_wait_status(pid, status);

	return 0;
}

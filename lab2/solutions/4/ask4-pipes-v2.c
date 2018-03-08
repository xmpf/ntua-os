/*
 * pipe-example.c
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <string.h>

#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

void fork_procs(struct tree_node *root, int fd)
{
	int i;
	/*
	 * Start
	 */
	printf("PID = %ld, name %s, starting...\n",
			(long)getpid(), root->name);
	change_pname(root->name);

	/* ... */
	pid_t *pid;
	int pfd[2];

	pid = malloc(sizeof(pid_t) * root->nr_children);
	if (pid == NULL) exit (1);

	if (pipe(pfd) < 0) {
			perror("pipe");
			exit(1);
	}

	/*We suppose that every process creates all subprocesses 
	without waiting for them to suspend individually. Then 
	waits for all of them.
	*/
	for(/* int */ i = 0; i < root->nr_children; i++){
		pid[i] = fork();
		if (pid[i] < 0) {
			perror("fork");
			exit(1);
		}
		if (pid[i] == 0) {
			/* Child */
			fork_procs(root->children + i, pfd[1]);
			exit(1);
		}	
	}

	wait_for_ready_children(root->nr_children);

	/*
	 * Suspend Self
	 */
	raise(SIGSTOP);
	printf("PID = %ld, name = %s is awake\n",
		(long)getpid(), root->name);

	/* ... */
	int status, result;

	if (root->nr_children == 0){
		result = atoi(root->name);
		// use writep from ex1
		if (write(fd, &result, sizeof(result)) != sizeof(result)) {
			perror("parent: write to pipe");
			exit(1);
		}
	}
	else {
		if (strcmp(root->name, "+") == 0){
			result = 0;
			for(/* int */ i = 0; i < root->nr_children; i++){
				kill(pid[i], SIGCONT);

				int temp;
				if (read(pfd[0], &temp, sizeof(temp)) != sizeof(temp)) {
					perror("parent: write to pipe");
					exit(1);
				}
				result += temp;

				wait(&status);
				explain_wait_status(pid[i], status);
			}
		}
		else if (strcmp(root->name, "*") == 0){
			result = 1;
			for(/* int */ i = 0; i < root->nr_children; i++){
				kill(pid[i], SIGCONT); // deliver SIGCONT signal to process pid[i]
				
				int temp;
				if (read(pfd[0], &temp, sizeof(temp)) != sizeof(temp)) {
					perror("parent: write to pipe");
					exit(1);
				}
				result *= temp;
				
				pid[i] = wait(&status);
				explain_wait_status(pid[i], status);
			}
		}
		if (write(fd, &result, sizeof(result)) != sizeof(result)) {
			perror("parent: write to pipe");
			exit(1);
		}
	}
	

	// close pipe fd and in children

	/*
	 * Exit
	 */
	free (pid);
	exit(0);
}

int main(int argc, char *argv[])
{
	pid_t p;
	int pfd[2];		// pipe fd[0,1]
	int status;
	int result;
	struct tree_node *root;

	if (argc < 2){
		fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}

	/* Read tree into memory */
	root = get_tree_from_file(argv[1]);
	
	printf("Parent: Creating pipe...\n");
	if (pipe(pfd) < 0) {
		perror("pipe");
		exit(1);
	}

	printf("Parent: Creating child...\n");
	
	p = fork();
	if (p < 0) {
		/* fork failed */
		perror("main: fork");
		exit(1);
	}
	if (p == 0) {
		/* Child */
		fork_procs(root, pfd[1]);
		exit(1);
	}

	/*
	 * In parent process.
	 */

	wait_for_ready_children(1);

	/* Print the process tree root at pid */
	show_pstree(p);

	// deliver signal SIGCONT to proces p
	kill(p, SIGCONT);

	/* Read result from the pipe */
	if (read(pfd[0], &result, sizeof(result)) != sizeof(result)) {
		perror("parent: write to pipe");
		exit(1);
	}

	/* Wait for the root of the process tree to terminate */
	p = wait(&status);
	explain_wait_status(p, status);

	printf("Result of tree is: %d\n", result);

	return 0;
}

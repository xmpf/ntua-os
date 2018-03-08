#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

void fork_procs(struct tree_node *root)
{
	int i;
	/*
	 * Start
	 */
	printf("PID = %ld, name %s, starting...\n",
			(long)getpid(), root->name);
	change_pname(root->name);
	
	/* My code for forks */
	pid_t *pid;
	
	pid = malloc(sizeof(pid_t) * root->nr_children);
	if (pid == NULL) exit (1);

	/*We suppose that every process creates all subprocesses 
	without waiting for them to suspend individually. Then 
	waits for all of them.
	*/
	for( /* int */ i = 0; i < root->nr_children; i++){
		pid[i] = fork();
		if (pid[i] < 0) {
			perror("fork");
			exit(1);
		}
		if (pid[i] == 0) { // create processes by BFS traversal
			/* Child */
			fork_procs(root->children + i);
			exit(1);
		}	
	}

	wait_for_ready_children(root->nr_children);
	/*
	 * Suspend Self
	 */
	raise(SIGSTOP); // put current process to sleep by sending SIGSTOP to the current process id
CONTINUE:
	printf("PID = %ld, name = %s is awake\n",
		(long)getpid(), root->name);

	/* ... */
	int status;

	for(/* int */ i = 0; i < root->nr_children; i++){
		kill(pid[i], SIGCONT); // send awake signal to children => foreach (child) { goto CONTINUE; }
		pid[i] = wait(&status);
		explain_wait_status(pid[i], status);
	}

	/*
	 * Exit
	 */
	free (pid);
	exit(0);
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

int main(int argc, char *argv[])
{
	pid_t pid;
	int status;
	struct tree_node *root;

	if (argc < 2){
		fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}

	/* Read tree into memory */
	root = get_tree_from_file(argv[1]);

	/* Fork root of process tree */
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Child */
		fork_procs(root);
		exit(1);
	}

	/*
	 * Father
	 */
	/* for ask2-signals */
	wait_for_ready_children(1);

	/* for ask2-{fork, tree} */
	/* sleep(SLEEP_TREE_SEC); */

	/* Print the process tree root at pid */
	show_pstree(pid);

	/* for ask2-signals */
	kill(pid, SIGCONT);

	/* Wait for the root of the process tree to terminate */
	wait(&status);
	explain_wait_status(pid, status);

	return 0;
}

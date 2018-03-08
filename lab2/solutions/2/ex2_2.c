#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "tree.h"
#include "proc-common.h"

#define SLEEP_PROC_SEC 6
#define SLEEP_TREE_SEC 2

pid_t *pid;
extern errno;

void creatProcTree (struct tree_node *node)
{

	int i, status;
	pid = (pid_t *)malloc (sizeof (pid_t) * node->nr_children);

	printf("%s: Hello, my pid is %ld\n", node->name, (long)getpid());
	change_pname (node->name);	// change name
	for (i = 0; i < node->nr_children; i++) {
		pid[i] = fork ();
		if (pid[i] < 0) { exit (EXIT_FAILURE); }
		else if (pid[i] == 0) { creatProcTree (&node->children[i]); }
	}
		
	while (i--) { 
		errno = 0;
		if ( -1 == waitpid (pid[i], &status, WUNTRACED))
			fprintf(stderr, "(%ld): %s\n", (long)pid[i], strerror(errno));
	}
 
	if (node->nr_children == 0)  // put leaves for sleep
		sleep(SLEEP_PROC_SEC);

    printf("%s: Goodbye (%ld)\n", node->name, (long)getpid());
    exit(0);
	
	return;
}

int main(int argc, char *argv[])
{
	struct tree_node *root;
	pid_t pid;
	int status;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
		exit(1);
	}

	root = get_tree_from_file(argv[1]);
	// print_tree(root);

	pid = fork();
	switch (pid) {
		case -1:
			perror ("fork");
			exit (EXIT_FAILURE);

		case 0:
			puts ("Creating process tree, recursively by BFS");
			creatProcTree (root);
			break;

		default:
			printf ("parent process: %d\n", getpid());
			break;
	}
	
	sleep (SLEEP_TREE_SEC);
	show_pstree (pid);

	pid = wait (&status);
	explain_wait_status (pid, status);

	return 0;
}

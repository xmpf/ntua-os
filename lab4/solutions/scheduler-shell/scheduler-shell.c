#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

#ifdef DEBUG_INFO
// Compile with -D DEBUG_INFO to enable debugging output
#endif

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */
#define MAX_TASKS	 20				  /* maximum number of tasks allowed to be run */


/* MACROS */
#define NEXT(X,L) (((X) + 1) % (L))


/* Implementation using arrays */

typedef enum {		/* priorities */
	PRIOR_LOW,
	PRIOR_HIGH
} prior_t;

typedef struct {	/* process structure */
	int rid;
	pid_t pid;
	char pname[TASK_NAME_SZ];
	prior_t priority;
} process_t;

process_t * procList[MAX_TASKS];		/* array of pointers to process_t struct */
process_t * highPriority[MAX_TASKS];	/* array of pointers to process_t struct */
process_t * lowPriority[MAX_TASKS];		/* array of pointers to process_t struct */

int runningID = 0;
int highProcs = 0;
int lowProcs  = 0;
int total_number_of_tasks = 0;
int flag = 0;

static void signals_enable(void);
static void signals_disable(void);
void init_queues (int);

void print_proc_details (process_t *proc)
{
	printf ("RID: %d\tPID: %ld\tNAME: %s\t", proc->rid, (long)proc->pid, proc->pname);
	if (proc->rid == runningID) printf("--running");
	printf("\n");
}

/* Print a list of all tasks currently being scheduled.  */
	static void
sched_print_tasks(void)
{
	int i = 0;
	puts ("HIGH PRIORITY");
	for (i = 0; i < MAX_TASKS; i++) {
		if (highPriority[i] != NULL) {
			print_proc_details (highPriority[i]);
		}
	}

	puts ("LOW PRIORITY");
	for (i = 0; i < MAX_TASKS; i++) {
		if (lowPriority[i] != NULL) {
			print_proc_details (lowPriority[i]);
		}
	}

}

/* Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
	static int
sched_kill_task_by_id(int id)
{
	if (id > MAX_TASKS) {
		fprintf (stderr, "Error: id out of bounds\n");
		exit (EXIT_FAILURE);
	}

	int has_company = lowProcs | highProcs;
	if (id == 0 && has_company) {
		fprintf (stderr, "Warning: There are still active jobs.");
		return 0;
	}

	if (!procList[id]) {
		exit (EXIT_FAILURE);
	}

	switch (procList[id]->priority) {
		case PRIOR_LOW:
			lowPriority[id] = NULL; // invalidate entry
			lowProcs -= 1;
			break;
		case PRIOR_HIGH:
			highPriority[id] = NULL; // invalidate entry
			highProcs -= 1;
			break;

		default:
			fprintf (stderr, "Internal error: Undefined priority!\n");
			exit (EXIT_FAILURE);
	}

	/* SIGKILL: terminates a process. Signal cannot be catched using a signal handler */
	if ( kill (procList[id]->pid, SIGKILL) < 0 ) {
		perror ("kill");
	}
	free (procList[id]);	// remove entry from memory
	procList[id] = NULL;	// invalidate entry
	fprintf (stderr, "INFO: Process %d killed!\n", id);
	return 0;
}


/* Create a new task.  */
	static void
sched_create_task(char *executable)
{
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

	/* first fit */
	int i = 0;
	while (procList[i] != NULL)
		i++;

	/* i holds the new id of the task created */
	process_t *newproc = (process_t *)malloc(sizeof(*newproc));
	newproc->rid = i;
	newproc->priority = PRIOR_LOW;
	strcpy (newproc->pname, executable);
	newproc->pname[TASK_NAME_SZ - 1] = '\0';
	newproc->priority = PRIOR_LOW;

	errno = 0;
	pid_t pid = fork ();	/* consider using vfork () */
	switch (pid) {
		case -1: // error
			fprintf (stderr, "Error: %s\n", strerror (errno));
			exit (EXIT_FAILURE);

		case 0: // child
			raise (SIGSTOP);
			errno = 0;
			execve (executable, newargv, newenviron);
			/* execve() returns only on error */
			fprintf (stderr, "Error: %s\n", strerror (errno));
			exit (EXIT_FAILURE);
			break;

		default: // parent
			newproc->pid = pid;
			lowPriority[i] = procList[i] = newproc;
			break;
	}
	total_number_of_tasks++;
}

	void
sched_change_priority (int rid, prior_t priority)
{
	if (!procList[rid]) {
		return;
	}

	procList[rid]->priority = priority;
	if (priority == PRIOR_LOW) {
		lowPriority[rid] = procList[rid];
		if (highPriority[rid]) {
			lowProcs += 1;
			highProcs -= 1;
		}
		highPriority[rid] = NULL;
	} else {
		highPriority[rid] = procList[rid];
		if (lowPriority[rid]) {
			lowProcs -= 1;
			highProcs += 1;
		}
		lowPriority[rid] = NULL;
	}

}

void destroy_universe (void)
{
	int i = 1;
	for (i = 1; i < MAX_TASKS; i++) {
		if (procList[i] != NULL) {
			kill (procList[i]->pid, SIGKILL);
			free (procList[i]);
			procList[i] = NULL;
			lowPriority[i] = NULL;
			highPriority[i] = NULL;
		}
	}

	lowProcs = 0;
	highProcs = 0;

}


/* Process requests by the shell.  */
static int
process_request(struct request_struct *rq)
{
	switch (rq->request_no) {
		case REQ_PRINT_TASKS:
			sched_print_tasks();
			return 0;

		case REQ_KILL_TASK:
			return sched_kill_task_by_id(rq->task_arg);

		case REQ_EXEC_TASK:
			sched_create_task(rq->exec_task_arg);
			return 0;

		case REQ_HIGH_TASK:
			sched_change_priority (rq->task_arg, PRIOR_HIGH);
			return 0;

		case REQ_LOW_TASK:
			sched_change_priority (rq->task_arg, PRIOR_LOW);
			return 0;

		case REQ_KILL_SHELL:
			signals_disable();
			if (lowProcs > 0 || highProcs > 0) {
				fprintf (stderr, "Sending SIGKILL to all processes...\n");
				destroy_universe ();
			}

			fprintf(stderr, "Shell: Exiting. Goodbye.\n");

			kill (procList[0]->pid, SIGKILL);

			free (procList[0]);
			procList[0] = NULL;
			signals_enable();
			return 0;
	}

	return -1;
}

/* 
 * SIGALRM handler
 */
	static void
sigalrm_handler(int signum)
{
	if (signum != SIGALRM) { // shouldn't happen
		fprintf (stderr, "Internal error: Called for signum %d, not SIGALRM\n", signum);
		exit (EXIT_FAILURE);
	}


	flag = 1;

	printf("ALARM! %d seconds have passed.\n", SCHED_TQ_SEC);
	
	if (!procList[runningID]) {
		exit (EXIT_FAILURE);
	}
	
	puts ("SIGALRM HANDLER.......");

	errno = 0;
	if ( kill(procList[runningID]->pid, SIGSTOP) < 0 ) {	// Timer expired => send SIGSTOP signal
		fprintf (stderr, "Error: %s (sigalrm_handler)\n", strerror (errno));
		exit (EXIT_FAILURE);
	}

	if (alarm(SCHED_TQ_SEC) < 0) {
		perror("alarm");
		exit(1);
	}

#ifdef DEBUG_INFO
	fprintf(stderr,"INFO: Timer has been reset!\n");
#endif
}

int sfind (void)
{

	int i = runningID;
	while (1) {
		i = NEXT (i, MAX_TASKS);
		
		/*
		if (i == 0 && procList[0]) {
			return 0;
		}
		*/
		if (highProcs > 0 && highPriority[i] != NULL) {
			return i;
		}

		if (highProcs == 0 && lowPriority[i] != NULL) {
			return i;
		}

	}
}

/* 
 * SIGCHLD handler
 */
	static void
sigchld_handler(int signum)
{
	/* SIGCHLD: 
	   The SIGCHLD signal is sent to the parent of a child process
	   when it exits, is interrupted, or resumes after being interrupted.
	   By default the signal is simply ignored.
	   */
	pid_t p;		/* process id */
	int status;	
	
	if (signum != SIGCHLD) {
		fprintf (stderr, "Internal error: Called for signum %d, not SIGCHLD\n", signum);
		exit (EXIT_FAILURE);
	}
	for (;;) {
		/* WNOHANG: return immediately if no child has exited */
		/* WUNTRACED: also  return  if  a  child  has stopped */
		p = waitpid(-1, &status, WUNTRACED | WNOHANG);
		if (p < 0) {
			perror("waitpid");
			exit(1);
		}
		if (p == 0) {
			/*
			   waitpid(): on success, returns the process ID of the child whose  state
			   has changed; if WNOHANG was specified and one or more child(ren) speci‐
			   fied by pid exist, but have not yet changed state, then 0 is  returned.
			   On error, -1 is returned.
			   */
			break;
		}

		explain_wait_status(p, status);

		/* WIFEXITED(wstatus):
		   returns true if the child terminated normally, that is, by calling exit() or by returning from main().

		   WIFSIGNALED(wstatus):
		   returns true if the child process was terminated by a signal.
		   */
		if ((WIFEXITED(status) || WIFSIGNALED(status)) && flag ) {
			/* A child has died */
			printf("Parent: Received SIGCHLD, child is dead.\nExiting...\n");
	
		
			int j;
			for (j = 1; j < MAX_TASKS; j++) {
				if (!procList[j]) continue;
				if (procList[j]->pid == p) break;
			}
			if (j == MAX_TASKS) break;
			if (!procList[j]) {
				exit (EXIT_FAILURE);
			}
	
			switch (procList[j]->priority) {
				case PRIOR_LOW:
					lowPriority[j] = NULL;
					lowProcs -= 1;
					break;
				case PRIOR_HIGH:
					highPriority[j] = NULL;
					highProcs -= 1;
					break;

				default: // error
					exit (EXIT_FAILURE);
			}
			

			procList[j] = NULL; // invalidate process

			if (!procList[0] && !lowProcs && !highProcs) { // there is nothing to do...
				exit (0);
			}
			
			if (j == runningID){
				runningID = sfind ();
	
			errno = 0;
			if ( kill(procList[runningID]->pid, SIGCONT) < 0 ) {
				fprintf (stderr, "Error: %s (sigchld_handler)\n", strerror (errno));
				exit (EXIT_FAILURE);
			}
			}

		}

		/*
		   WIFSTOPPED(wstatus):
		   returns  true  if the child process was stopped by delivery of a
		   signal; this is possible only if the call was  done  using  WUNTRACED
		   or when the child is being traced (see ptrace(2)).
		   */
		if (WIFSTOPPED(status) && flag) {
			/* A child has stopped due to SIGSTOP/SIGTSTP, etc... */
			printf("Parent: Child has been stopped. Moving right along...\n");

			runningID = sfind ();   // returns next process

			errno = 0;
			if ( kill(procList[runningID]->pid, SIGCONT) < 0 ) {
				fprintf(stderr, "Error: %s\n (sigchld_handler)", strerror(errno));
				exit(EXIT_FAILURE);
			}

		}
	}

}

/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
	static void
install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGALRM);
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		perror("sigaction: sigalrm");
		exit(1);
	}

	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	if (signal(SIGPIPE, SIG_IGN) < 0) {
		perror("signal: sigpipe");
		exit(1);
	}
}

	static void
do_shell(char *executable, int wfd, int rfd)
{
	char arg1[10], arg2[10];
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

	sprintf(arg1, "%05d", wfd);
	sprintf(arg2, "%05d", rfd);
	newargv[1] = arg1;
	newargv[2] = arg2;

	raise(SIGSTOP);
	execve(executable, newargv, newenviron);

	/* execve() only returns on error */
	perror("scheduler: child: execve");
	exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
	static void
sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
	pid_t p;
	int pfds_rq[2], pfds_ret[2];

	if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
		perror("pipe");
		exit(1);
	}

	p = fork();
	if (p < 0) {
		perror("scheduler: fork");
		exit(1);
	}

	if (p == 0) {
		/* Child */
		close(pfds_rq[0]);
		close(pfds_ret[1]);
		do_shell(executable, pfds_rq[1], pfds_ret[0]);
		assert(0);
	}
	/* Parent */

	errno = 0;
	process_t *shellp = (process_t *)malloc(sizeof(*shellp));
	if (shellp == NULL) {
		fprintf (stderr, "Error: %s\n", strerror (errno));
		exit (EXIT_FAILURE);
	}
	shellp->rid = 0;
	shellp->pid = p;
	strcpy (shellp->pname, "shell");
	shellp->pname[TASK_NAME_SZ - 1] = '\0';
	shellp->priority = PRIOR_LOW;
	procList[0] = shellp;

	close(pfds_rq[1]);
	close(pfds_ret[0]);
	*request_fd = pfds_rq[0];
	*return_fd = pfds_ret[1];
}

	static void
shell_request_loop(int request_fd, int return_fd)
{
	int ret;
	struct request_struct rq;

	/*
	 * Keep receiving requests from the shell.
	 */
	for (;;) {
		if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
			perror("scheduler: read from shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}

		signals_disable();	// disable signals
		ret = process_request(&rq);	// process request
		signals_enable(); 	// enable signals

		if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret)) {
			perror("scheduler: write to shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}
	}
}

void update_priority_queues (int limit)
{
	int i = 0;
	for (i = 0; i < limit; i++) {

		if (!procList[i]) {
			lowPriority[i] = highPriority[i] = NULL;
			continue;
		}

		switch (procList[i]->priority) {
			case PRIOR_LOW:
				lowPriority[i] = procList[i];
				break;

			case PRIOR_HIGH:
				highPriority[i] = procList[i];
				break;

			default:
				fprintf (stderr, "Internal error: Unexpected priority value!\n");
				exit (EXIT_FAILURE);
		}
	}
}

int main(int argc, char *argv[])
{
	int i, nproc;
	/* Two file descriptors for communication with the shell */
	static int request_fd, return_fd;

	/* Create the shell. */
	sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);

	/* TODO: add the shell to the scheduler's tasks */

	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	lowProcs = nproc = argc - 1;
	total_number_of_tasks = argc;

	errno = 0;
	process_t *procadd = (process_t *)malloc(sizeof(*procadd));
	if (procadd == NULL) {
		fprintf (stderr, "Error: %s\n", strerror (errno));
		exit (EXIT_FAILURE);
	}

	for (i = 1; i <= nproc; i++) {
		/* default options */
		errno = 0;
		process_t *procadd = (process_t *)malloc(sizeof(*procadd));
		if (procadd == NULL) {
			fprintf (stderr, "Error: %s\n", strerror (errno));
			exit (EXIT_FAILURE);
		}
		char *newargv[] = { argv[i], NULL, NULL, NULL };
		char *newenviron[] = { NULL };
		errno = 0;
		pid_t pid = fork ();	/* consider vfork () as we call execve () right away */
		switch (pid) {
			case -1: // error
				fprintf (stderr, "Error: %s\n", strerror (errno));
				exit (EXIT_FAILURE);
			case 0: // child
				/* initial state */
				raise (SIGSTOP);
				errno = 0;
				execve (argv[i], newargv, newenviron);
				/* execve() returns only on error */
				fprintf (stderr, "Error: %s\n", strerror (errno));
				exit (EXIT_FAILURE);
				break;
			default:
				procadd->rid = i;
				procadd->pid = pid;
				strcpy (procadd->pname, argv[i]);
				procadd->pname[TASK_NAME_SZ - 1] = '\0';
				procadd->priority = PRIOR_LOW;
				procList[i] = procadd;
				break;
		}
	}
	update_priority_queues (MAX_TASKS);

	/* Wait for all children to raise SIGSTOP before exec()ing. */
	wait_for_ready_children(nproc);

	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();
	
	if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}

	
	runningID = 0;
	kill(procList[0]->pid, SIGCONT); /* send SIGCONT to shell's process */
	

	if (alarm(SCHED_TQ_SEC) < 0) {
	 perror("alarm");
	 exit (EXIT_FAILURE);
	}

	shell_request_loop(request_fd, return_fd);
	

	/* Now that the shell is gone, just loop forever
	 * until we exit from inside a signal handler.
	 */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}

/* Disable delivery of SIGALRM and SIGCHLD. */
	static void
signals_disable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("signals_disable: sigprocmask");
		exit(1);
	}
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
	static void
signals_enable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
	}
}

void init_queues (int nproc)
{
	while (nproc > 0) {
		procList[nproc] = NULL;
		highPriority[nproc] = NULL;
		lowPriority[nproc] = NULL;
	}
}

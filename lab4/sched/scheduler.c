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

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */


/* SIGALRM handler: Gets called whenever alarm expires.
 * The time quantum has expired => send SIGSTOP.
 * Process next in cycle (RR)
 */
static void
sigalrm_handler(int signum)
{
	/* alarm()  arranges  for  a SIGALRM signal to be delivered to the calling
     * process in seconds seconds.
	 */
	alarm (SCHED_TQ_SEC);

	/*
	   The  kill()  system  call can be used to send any signal to any process
       group or process. If pid is positive, then signal sig is sent to the process with the  ID
       specified by pid.
	*/
	errno = 0;
	if (kill (pid, SIGSTOP) < 0) { 
		fprintf (stderr, "SIGSTOP: %s\n", strerror_r (errno));
	}
}

/* 
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum)
{
	/*	SIGCHLD:
		This signal is sent (by the kernel) to a parent process when one of its children
		terminates (either by calling exit() or as a result of being killed by a signal). It
		may also be sent to a process when one of its children is stopped or
		resumed by a signal.
		
		SIGCONT:
		When sent to a stopped process, this signal causes the process to resume
		(i.e., to be rescheduled to run at some later time). When received by a pro-
		cess that is not currently stopped, this signal is ignored by default. A process
		may catch this signal, so that it carries out some action when it resumes.
	*/
	assert(0 && "Please fill me!");
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
	sigaddset(&sigset, SIGCHLD);	// SIGCHLD
	sigaddset(&sigset, SIGALRM);	// SIGALRM
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {	// SIGCHLD
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {	// SIGALRM
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

void infoMsg (void)
{
	fprintf(stderr, "%s\n", "INFO");
	return 1;
}

int main(int argc, char *argv[])
{
	int nproc;
	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	if (argc < 2) { 
		infoMsg ();
	}

	nproc = argc - 1;
	for (int id = 0; id < nproc; id++) {
		errno = 0;
		pid_t pid = fork ();
		switch (pid) {
			-1: // ERROR
				fprintf(stderr, "%s\n", strerr (errno) );
				return -1;
			0:	// CHILD
				execve (NULL);
				break;
			default: // PARENT
			#ifdef DEBUG_INFO
				fprintf(stderr, "Parent: Child %ld created succesfully\n", (long)pid);
			#endif
		}
	}

	/* Wait for all children to raise SIGSTOP before exec()ing. */
	wait_for_ready_children(nproc);

	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

	if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}


	/* loop forever  until we exit from inside a signal handler. */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}

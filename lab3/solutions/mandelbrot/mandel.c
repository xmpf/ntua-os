/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

#define perror_pthread(ret, msg) \
	do { errno = ret; perror(msg); } while (0)

typedef struct {
    pthread_t tid;  /* thread id as given by pthread_create */
    int       rid;  /* relative id */
} pthread_info_t;\

pthread_info_t *threads;
sem_t *semaphores;

int total_threads = 1;
/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 50;
int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;
	
/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
	/*
	 * x and y traverse the complex plane.
	 */
	double x, y;

	int n;
	int val;

	/* Find out the y value corresponding to this line */
	y = ymax - ystep * line;

	/* and iterate for all points on this line */
	for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

		/* Compute the point's color value */
		val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
		if (val > 255)
			val = 255;

		/* And store it in the color_val[] array */
		val = xterm_color(val);
		color_val[n] = val;
	}
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
	int i;
	
	char point ='@';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[i]);
		if (write(fd, &point, 1) != 1) {
			perror("compute_and_output_mandel_line: write point");
			exit(1);
		}
	}

	/* Now that the line is done, output a newline character */
	if (write(fd, &newline, 1) != 1) {
		perror("compute_and_output_mandel_line: write newline");
		exit(1);
	}
}

void *start_fn(void *arg)
{
    pthread_info_t thread = *(pthread_info_t *)arg;
	int color_val[x_chars];

    for (int i = thread.rid; i < y_chars; i += total_threads) {
        compute_mandel_line(i, color_val);
        sem_wait(&semaphores[thread.rid % total_threads]);    /* block on current line */
        output_mandel_line(1, color_val);
        sem_post(&semaphores[(thread.rid + 1) % total_threads]);
    }
    return NULL;
}

/*********************************************************/
void usage (void);
void *safe_malloc(size_t);
int safe_atoi(char *, int *);
/*********************************************************/
int main(int argc, char *argv[])
{

    if (argc != 2 || strcmp (argv[1], "--help") == 0) {
        usage ();
    }

	int i;

    int ret = safe_atoi(argv[1], &total_threads);
    if (ret < 0 || total_threads < 0) { usage(); }

    xstep = (xmax - xmin) / x_chars;
    ystep = (ymax - ymin) / y_chars;

    threads = (pthread_info_t *)safe_malloc (total_threads * sizeof (*threads));
    semaphores = (sem_t *)safe_malloc(total_threads * sizeof (*semaphores));

    /* initialize all semaphores to 0 */
    for (i = 1; i < total_threads; i++)
        sem_init (&semaphores[i], 0 /* not shared */, 0 /* initial value */);
    /*
     * control access to critical section (output lines in corresponding sequence)
     * block until semaphore has positive value
     * circular chain reaction
     */
    sem_init (&semaphores[i], 0 /* not shared */, 1 /* initial value */);

    for (i = 0; i < total_threads; i++) {
        threads[i].rid = i;
        ret = pthread_create(&threads[i].tid, NULL, start_fn, &threads[i]);
        if (ret < 0) {
            perror_pthread(ret, "pthread_create");
        }
    }

    for (i = 0; i < total_threads; i++) {
        pthread_join(threads[i].tid, NULL);
    }

    free (threads);
    free (semaphores);
	reset_xterm_color(1);
	return 0;
}

void usage (void) {

    const char red_color[] = "\x1b[31m";
    const char rst_color[] = "\x1b[30m;";
    const char *help_msg = "Error: ./<progname> <total_threads>";
    fprintf (
            stderr,
            "%s%s%s\n",
            red_color, help_msg, rst_color
    );
    exit (EXIT_FAILURE);
}

void *safe_malloc(size_t size)
{
    void *p;

    if ((p = malloc(size)) == NULL) {
        fprintf(stderr, "Out of memory, failed to allocate %zd bytes\n",
                size);
        exit(1);
    }

    return p;
}

int safe_atoi(char *s, int *val)
{
    long l;
    char *endp;

    l = strtol(s, &endp, 10);
    if (s != endp && *endp == '\0') {
        *val = l;
        return 0;
    } else
        return -1;
}
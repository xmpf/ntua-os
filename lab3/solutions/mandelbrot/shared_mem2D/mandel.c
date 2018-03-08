/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "mandel-lib.h"

void *safe_malloc(size_t);
int safe_atoi(char *, int *);
void *start_fn (void *);

typedef struct {
    pthread_t tid;      /* thread ID */
    int       rid;      /* relative ID */
    int       index;    /* private copy of index */
} pthread_pool_t;

#define MANDEL_MAX_ITERATION 100000

#ifdef DEBUG
#define printd(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#endif

/*
 * POSIX thread functions do not return error numbers in errno,
 * but in the actual return value of the function call instead.
 * This macro helps with error reporting in this case.
 */
#define perror_pthread(ret, msg) \
	do { errno = ret; perror(msg); } while (0)


int total_threads = 1;
pthread_pool_t *threads;

/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
const int y_chars = 50;
const int x_chars = 90;

static int **color_val;

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
void compute_mandel_line(int line)
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
		val = xterm_color(val); // return approximation
		color_val[line][n] = val;		// store color code
	}
#ifdef DEBUG
    printd ("Line %d computed\n", line);
#endif
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int line)
{
	int i;
	
	char point ='@';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[line][i]);
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

void usage (void) {

    const char red_color[] = "^[[0;31;0m";
    const char rst_color[] = "^[[0;30;0m";
    const char *help_msg = "Error: ./<progname> <total_threads>";
    fprintf (
               stderr,
               "%s%s%s\n",
               red_color, help_msg, rst_color
            );
    exit (EXIT_FAILURE);
}

double time_spent (clock_t);

int main(int argc, char *argv[])
{

    if (argc != 2 || strcmp (argv[1], "--help") == 0) {
        usage ();
    }
    char filename[25] = "time_outputs/";
    strcat (filename, argv[0]);
    strcat (filename, "_");
    strcat (filename, argv[1]);
    FILE *output = fopen (filename, "w");

    clock_t tstart[4]; // { program, thread creation, wait_threads_to_complete, mem_deallocation }

    tstart[0] = clock();    /* whole program */
	int i,      /* index */
        ret;    /* return values */

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

    // we assume that argv[1] is an integer as string
    ret = safe_atoi(argv[1], &total_threads);
    if (ret < 0 || total_threads < 0) { usage(); }

    /* allocate memory for shared variable `color_val[][]` */
    color_val = (int **)safe_malloc(y_chars * sizeof (*color_val));
    for (i = 0; i < y_chars; i++) {
        color_val[i] = (int *)safe_malloc(x_chars * sizeof (**color_val));
    }

    /* allocate space for thread pool */
    threads = (pthread_pool_t *)safe_malloc(total_threads * sizeof (pthread_pool_t));

	/*
	 *  - create threads
	 *  - initialize per thread data
	 */
    tstart[1] = clock();    /* creation of threads */
    for (i = 0; i < total_threads; i++) {
        threads[i].rid = i;
        ret = pthread_create(&(threads[i].tid), NULL, start_fn, &(threads[i]));
        if (ret < 0) {
            perror_pthread(ret, "pthread_create");
        }
    }
    fprintf (output, "Created threads to compute colors.\nTime spent: %lf\n", time_spent (tstart[1]));

    /*
	 *  - call kernel::start_fn, from each thread
	 *      - kernel::start_fn, should:
	 *          - call compute_mandel_line for number of lines (y_chars / total_threads)
	 *          - for (i = threadID; i < y_chars; i += total_threads) { compute_mandel_line(); }
	 */


    /*
     * Wait for all threads to terminate
     */
    tstart[2] = clock();    /* waiting threads to finish execution */
    for (i = 0; i < total_threads; i++) {
        // obtain return status of thread `tid`
        ret = pthread_join(threads[i].tid, NULL);
        if (ret) {
            perror_pthread(ret, "pthread_join");
            exit(1);
        }
    }
    fprintf (output, "\nWaiting threads to finish.\nTime spent: %lf\n", time_spent (tstart[2]));

    /* output mandel image line by line */
    for (i = 0; i < y_chars; i++) {
        output_mandel_line(1, i);
    }

    tstart[3] = clock(); /* deallocate memory */
    for (i = 0; i < y_chars; i++)
        free (color_val[i]);
    free (color_val);
    free (threads);
    fprintf (output, "\nDeallocating memory.\nTime spent: %lf\n", time_spent (tstart[3]));


    // write time to file
    fprintf (output, "\nProgram finished.\nTime spent: %lf\n", time_spent (tstart[0]));

    fclose (output);    /* close file handler */
    reset_xterm_color(1);
    return 0;
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

void *start_fn (void *arg)
{
    pthread_pool_t *thr = (pthread_pool_t *) arg;
    for (thr->index = thr->rid; thr->index < y_chars; thr->index += total_threads) {
        compute_mandel_line(thr->index);
    }
    pthread_exit (NULL);
}

double time_spent (clock_t start) {
    clock_t now = clock ();
    return ((double) now - start) / CLOCKS_PER_SEC;
}

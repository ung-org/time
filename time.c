/*
 * UNG's Not GNU
 *
 * Copyright (c) 2020, Jakob Kaivo <jkk@ung.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _XOPEN_SOURCE 700
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <unistd.h>

#define UTILITY_NOT_INVOKED	(126)
#define UTILITY_NOT_FOUND	(127)

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	int c;
	while ((c = getopt(argc, argv, "p")) != -1) {
		switch (c) {
		case 'p':
			/* ignored, only use POSIX output format */
			break;

		default:
			return 1;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "time: missing operand\n");
		return UTILITY_NOT_FOUND;
	}

	struct tms tmstart;
	clock_t start = times(&tmstart);

	/*
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
	*/

	pid_t pid = fork();
	if (pid == -1) {
		perror("time");
		return UTILITY_NOT_INVOKED;
	}

	if (pid == 0) {
		execvp(argv[optind], argv + optind);
		fprintf(stderr, "time: %s: %s\n", argv[optind], strerror(errno));
		return errno == ENOENT ? UTILITY_NOT_FOUND : UTILITY_NOT_INVOKED;
	}

	int status;
	waitpid(pid, &status, 0);

	/*
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	*/

	struct tms tmend;
	clock_t end = times(&tmend);

	status = WEXITSTATUS(status);
	if (status == UTILITY_NOT_FOUND || status == UTILITY_NOT_INVOKED) {
		return status;
	}

	/*
	double real = tstod(&end) - tstod(&start);
	double user = (double)tms.tms_cutime / (double)CLOCKS_PER_SEC;
	double sys = (double)tms.tms_cstime / (double)CLOCKS_PER_SEC;
	*/

	double ticks = (double)sysconf(_SC_CLK_TCK);
	double real = (double)(end - start) / ticks;
	double user = (double)(tmend.tms_cutime - tmstart.tms_cutime) / ticks;
	double sys = (double)(tmend.tms_cstime - tmstart.tms_cstime) / ticks;

	fprintf(stderr, "real %.2f\n", real);
	fprintf(stderr, "user %.2f\n", user);
	fprintf(stderr, "sys %.2f\n", sys);
	return status;
}

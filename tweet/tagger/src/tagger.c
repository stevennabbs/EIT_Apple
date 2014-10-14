/*
 *      Tagger -- A simple tool for manual corpus tagging
 *
 * Copyright (c) 2014  Thomas Lavergne <thomas.lavergne@reveurs.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#define _XOPEN_SOURCE
#define _POSIX_SOURCE

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>

/* fatal:
 *   This is the main error function, it will print the given message with same
 *   formating than the printf family and exit program with an error. We let the
 *   OS care about freeing ressources.
 */
void fatal(const char *msg, ...) {
	va_list args;
	fprintf(stderr, "error: ");
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

/* pfatal:
 *   This one is very similar to the fatal function but print an additional
 *   system error message depending on the errno. This can be used when a
 *   function who set the errno fail to print more detailed informations. You
 *   must be carefull to not call other functino that might reset it before
 *   calling pfatal.
 */
void pfatal(const char *msg, ...) {
	const char *err = strerror(errno);
	va_list args;
	fprintf(stderr, "error: ");
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
	fprintf(stderr, "\n\t<%s>\n", err);
	exit(EXIT_FAILURE);
}

/* strndup:
 *   Duplicate the string [s] taking at most [n] characters from it. What the
 *   fuck we have to redefine this function ? Some systems still doesn't provide
 *   it...
 */
/*static*/
char *strndup(const char *s, size_t n) {
	size_t l = 0;
	while (l < n && s[l] != '\0')
		l++;
	char *r = malloc(l + 1);
	if (r == NULL)
		fatal("out of memory");
	memcpy(r, s, l);
	r[l] = '\0';
	return r;
}

/* decode:
 *   Decode and return the next UTF-8 encode unicode codepoint from the given
 *   string and update the position. Return -1 on end of string.
 */
static inline
int decode(const char *str, size_t len, size_t *pos) {
	static const unsigned char utf8d[] = {
	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	    8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	    0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3,
	    0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,
	    0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1,
	    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,
	    1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
	    1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1,
	    1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	};
	const unsigned char *raw = (const unsigned char *)str;
	if (*pos >= len)
		return -1;
	int st = utf8d[256 + utf8d[raw[*pos]]];
	int cp = raw[*pos] & (255 >> utf8d[raw[*pos]]);
	for ((*pos)++ ; st > 1 && *pos < len; (*pos)++) {
		st = utf8d[256 + st * 16 + utf8d[raw[*pos]]];
		cp = (cp << 6) | (raw[*pos] & 63);
	}
	if (st != 0)
		fatal("invalid UTF-8 sequence");
	return cp;
}

static struct termios new_term;
static struct termios old_term;

/* cleanup:
 *   This program take the full control of the TTY instead of using the system
 *   facility, this mean that it have to be very carefull about restoring the
 *   terminal in it initial state. This function take care of doing this in
 *   almost all exit condition.
 */
static
void cleanup(void) {
	printf("\x1b[?25h\r\n"); fflush(stdout);
	tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

/* sigtstp:
 *   This is the signal handler for the TSTP signal. It take care of restoring
 *   the terminal in a clean state before going to sleep and retaking full
 *   control when returning to normal operation. This try to honor the sleep
 *   request but if anything fail it will ignore it for safety.
 */
static
void sigtstp(int sig) {
	(void)sig; // Unused
	// Save old value of errno in case it is modified here and restore the
	// terminal in its initial configuration. This ensure that the shell
	// will work correctly.
	const int old_errno = errno;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &old_term) == -1)
		goto error;
	// Next, the SIGTSTP handler is restored to its default value and raised
	// again so this process will go to sleep until it is restored.
	if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
		goto error;
	raise(SIGTSTP);
	sigset_t mask, prev;
	sigemptyset(&mask);
	sigaddset(&mask, SIGTSTP);
	if (sigprocmask(SIG_UNBLOCK, &mask, &prev) == -1)
		goto error;
	// At this point, the process go to sleep and execution will continue
	// only when it is restored. The first thing to do is to put back this
	// handler in place to wait for the next TSTP signal.
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
		goto error;
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags   = SA_RESTART;
	sa.sa_handler = sigtstp;
	if (sigaction(SIGTSTP, &sa, NULL) == -1)
		goto error;
	// Before returning to the normal operation, the terminal should be put
	// back in our controled mode and errno variable restored. This is also
	// here that a full redraw should be triggered later.
	if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1)
		goto error;
	errno = old_errno;
	return;
    error:
	signal(SIGTSTP, SIG_IGN);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
	errno = old_errno;
}

typedef struct spl_s spl_t;
struct spl_s {
	spl_t *prv, *nxt;
	char  *sid, *cat;
	char  *lbl, *raw;
	int    nln,  len[4], ett;
	char        *lns[4];
};

static const char *lbls[][3] = {
	{"???", "\x1b[1;37m", "\x1b[1;37;7m"},
	{"pos", "\x1b[1;32m", "\x1b[1;32;7m"},
	{"neu", "\x1b[1;34m", "\x1b[1;34;7m"},
	{"neg", "\x1b[1;31m", "\x1b[1;31;7m"},
	{"irr", "\x1b[1;33m", "\x1b[1;33;7m"},
};
static const int nlbls = sizeof(lbls) / sizeof(lbls[0]);

static
void save(const spl_t *spl, const char *fname) {
	FILE *file = fopen(fname, "w");
	while (spl != NULL) {
		fprintf(file, "(%s,%s,%s) %s\n", spl->sid, lbls[spl->ett][0],
		       spl->cat, spl->raw);
		spl = spl->nxt;
	}
	fclose(file);
}

int main(int argc, char *argv[argc]) {
	const char *iname = NULL;
	const char *oname = NULL;
	if (argc > 1) {
		iname = argv[1];
		if (argc > 2) oname = argv[2];
		else          oname = iname;
	} else {
		fatal("missing arguments");
	}
	// Setup the terminal. This put the term in raw mode to ensure we have
	// full control and clear the screen. A cleanup function is registered
	// to restore the terminal in a workable state on exit.
	static const char *err_term = "error: cannot setup the term\n\t%s\n";
	if (tcgetattr(STDIN_FILENO, &old_term) == -1) {
		fprintf(stderr, err_term, strerror(errno));
		return EXIT_FAILURE;
	}
	atexit(cleanup);
	new_term = old_term;
	new_term.c_iflag &= ~(BRKINT | INPCK | IXOFF | IXON);
	new_term.c_iflag &= ~(ISTRIP | ICRNL | IGNCR | INLCR);
	new_term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	new_term.c_oflag &= ~(OPOST);
	new_term.c_cflag |=  (CS8);
	new_term.c_cc[VMIN]  = 1;
	new_term.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1) {
		fprintf(stderr, err_term, strerror(errno));
		return EXIT_FAILURE;
	}
	// Setup the TSTP handler. This will ensure that terminal is correctly
	// restored when going to sleep and returning. Even if ^Z is disabled
	// this is needed as user may send the signal in another way.
	static const char *err_tstp = "error: cannot set TSTP handler\n\t%s\n";
	struct sigaction sa;
	if (sigaction(SIGTSTP, NULL, &sa) == -1) {
		fprintf(stderr, err_tstp, strerror(errno));
		return EXIT_FAILURE;
	}
	if (sa.sa_handler != SIG_IGN) {
		sigemptyset(&sa.sa_mask);
		sa.sa_flags   = SA_RESTART;
		sa.sa_handler = sigtstp;
		if (sigaction(SIGTSTP, &sa, NULL) == -1) {
			fprintf(stderr, err_tstp, strerror(errno));
			return EXIT_FAILURE;
		}
	}
	// Open and map the file in memory. This make loading faster and code a
	// lot simpler as this don't have to care about variably sized buffer
	// for storing lines.
	int file = open(iname, O_RDONLY);
	if (file < 0)
		pfatal("failed to open file \"%s\"", iname);
	struct stat stat;
	if (fstat(file, &stat) != 0)
		pfatal("failed to stat file \"%s\"", iname);
	const size_t size = stat.st_size;
	char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, file, 0);
	if (data == NULL)
		pfatal("failed to map file \"%s\"", iname);
	// Next parse the corpus, this split the data in lines and allocate
	// sample objects. The code stop on the first error and don't try to
	// recover.
	int nspl = 0;
	spl_t *head = NULL, *tail = NULL;
	for (size_t pos = 0; pos < size; ) {
		nspl++;
		while (pos < size && isspace(data[pos]))
			pos++;
		const char *str = data + pos;
		while (pos < size && data[pos] != '\n')
			pos++;
		size_t len = pos++ - (str - data);
		// Now that a new line is found, a new sample object is setup
		// and inserted in the linked-list. After this point in case of
		// failure, it will be freed with the list.
		spl_t *spl = malloc(sizeof(spl_t));
		if (spl == NULL)
			fatal("out of memory");
		spl->sid    = spl->cat    = NULL;
		spl->lbl    = spl->raw    = NULL;
		spl->lns[0] = spl->lns[1] = NULL;
		spl->lns[2] = spl->lns[3] = NULL;
		spl->nln    =  0;
		spl->ett    = -1;
		if (head == NULL) head      = spl;
		else              tail->nxt = spl;
		spl->nxt = NULL;
		spl->prv = tail;
		tail = spl;
		// At this point, the parsing of the sample can start as the
		// sample is safely inserted in the list and no memory loss can
		// happen in case of error.
		if (len-- == 0 || *(str++) != '(')
			fatal("invalid sample at line %d", nspl);
		const char *sid = str;
		while (len != 0 && (isalnum(*str) || *str == '?'))
			str++, len--;
		spl->sid = strndup(sid, str - sid);
		if (spl->sid == NULL || len-- == 0 || *(str++) != ',')
			fatal("invalid sample at line %d", nspl);
		const char *lbl = str;
		while (len != 0 && (isalnum(*str) || *str == '?'))
			str++, len--;
		spl->lbl = strndup(lbl, str - lbl);
		if (spl->lbl == NULL || len-- == 0 || *(str++) != ',')
			fatal("invalid sample at line %d", nspl);
		const char *cat = str;
		while (len != 0 && *str != ')')
			str++, len--;
		spl->cat = strndup(cat, str - cat);
		if (spl->cat == NULL || len-- == 0 || *(str++) != ')')
			fatal("invalid sample at line %d", nspl);
		while (len != 0 && isspace(*str))
			str++, len--;
		spl->raw = strndup(str, len);
		if (spl->raw == NULL)
			fatal("invalid sample at line %d", nspl);
		for (int i = 0; spl->ett < 0 && i < 5; i++)
			if (!strncmp(spl->lbl, lbls[i][0], strlen(spl->lbl)))
				spl->ett = i;
	}
	// Cleanup things and return the linked list of samples. Here errors are
	// simply ignored as the have been read without problems.
	if (munmap(data, size) == -1)
		pfatal("failed to unmap file \"%s\"", iname);
	if (close(file) == -1)
		pfatal("failed to close file \"%s\"", iname);
	// Split samples from the given list in a set of strings of a reasonable
	// number of unicode codepoints. This is far from perfect as it doesn't
	// work directly on grapheme clusters and doesn't handle double size
	// characters but its ok for now.
	for (spl_t *spl = head; spl != NULL; spl = spl->nxt) {
		for (int ln = 0; ln < spl->nln; ln++) {
			free(spl->lns[ln]);
			spl->lns[ln] = NULL;
		}
		spl->nln = 0;
		const char *str = spl->raw;
		size_t pos = 0, len = strlen(str);
		for (int ln = 0; ln < 4 && pos < len; ln++) {
			size_t brk = (size_t)-1, off = pos, ncp = 0;
			for (int cnt = 0; cnt < 50; cnt++) {
				const int chr = decode(str, len, &pos);
				if (chr == -1 || isspace(chr))
					brk = pos, ncp = cnt;
				if (chr == -1) {
					ncp = cnt;
					break;
				}
			}
			if (brk == (size_t)-1) brk = pos, ncp = 50;
			spl->lns[ln] = strndup(str + off, brk - off);
			spl->len[ln] = ncp;
			if (spl->lns[ln] == NULL)
				fatal("out of memory");
			spl->nln++;
			pos = brk;
		}
	}
	char *err  = NULL;
	spl_t *spl = head;
	printf("\x1b[?25l\x1b[2J");
	while (1) {
		int stt[nlbls]; memset(stt, 0, nlbls * sizeof(int));
		for (spl_t *p = head; p != NULL; p = p->nxt)
			stt[p->ett]++;
		struct winsize size;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
		const int orig = 8, cntr = size.ws_col / 2;
		for (int i = 0; i < nlbls; i++) {
			printf("\x1b[%d;H\x1b[K%s", i + 1, lbls[i][1]);
			printf("%s\x1b[0m : %d   ", lbls[i][0], stt[i]);
		}
		for (int l = -2; l < 4; l++)
			printf("\x1b[%d;1H\x1b[K", orig + l);
		printf("\x1b[%d;%dH", orig - 2, cntr - 20);
		printf("[%s]  %s", spl->sid, spl->cat);
		for (int l = 0; l < spl->nln; l++) {
			printf("\x1b[%d;%dH", orig + l, cntr - spl->len[l] / 2);
			printf("%s", spl->lns[l]);
		}
		const int off = (nlbls * 4 - 1) / 2;
		printf("\x1b[%d;%dH", orig + 5, cntr - off);
		for (int i = 0; i < nlbls; i++) {
			const int is = spl->ett == i;
			printf("%s%s\x1b[0m ", lbls[i][1 + is], lbls[i][0]);
		}
		printf("\x1b[%d;4H\x1b[K", orig + 8);
		if (err != NULL) {
			printf("\x1b[41m%s\x1b[0m", err);
			err = NULL;
		}
		fflush(stdout);
		// Read next key code from the TTY. This is a bit tricky as
		// sequence representing characters have no easy to detect
		// termination mark.
		char raw[32];
		int  len = 1;
		while (read(STDIN_FILENO, raw, 1) != 1)
			;
		while (1) {
			struct pollfd pfd = {0, POLLIN, 0};
			const int res = poll(&pfd, 1, 50);
			if (res < 0) {
				if (errno == EINTR || errno == ENOMEM)
					continue;
				pfatal("cannot read input from keyboard");
			} else if (res == 0) {
				break;
			}
			while (read(STDIN_FILENO, raw + len++, 1) != 1)
				;
		}
		raw[len] = '\0';
		// Now that a key have been received, it should be handled
		// correctly. This code do a very simple dispatch as speed is
		// not something to care about here.
		int key = raw[0];
		if (len > 1) {
			     if (!strcmp(raw, "\x1b[A")) key = 200; // up
			else if (!strcmp(raw, "\x1b[B")) key = 201; // down
			else if (!strcmp(raw, "\x1b[C")) key = 202; // right
			else if (!strcmp(raw, "\x1b[D")) key = 203; // left
			else                             key = 0;
		}
		switch (key) {
			case 'Q' - '@':
			case 'C' - '@':
				printf("\x1b[%d;4H\x1b[K", orig + 8);
				printf("\x1b[41mDo you want to save ?\x1b[0m");
				while (1) {
					int chr = getchar();
					if (chr == 'y' || chr == 'Y') {
						save(head, oname); break;
					} else if (chr == 'n' || chr == 'N') {
						break;
					}
				}
				goto exit;
			case 'L' - '@':
				printf("\x1b[2J");
				break;
			// Saving. This just dump the content of the sample list
			// to the output file.
			case 'S' - '@':
				save(head, oname);
				break;
			// Move to prev sample. There is two case here, either
			// we want the true prev sample or the prev one with no
			// label.
			case 'k':
			case 200:
				while (spl->prv != NULL) {
					spl = spl->prv;
					if (spl->ett == 0)
						break;
				}
				break;
			case 'K':
				if (spl->prv != NULL) {
					spl = spl->prv;
					break;
				}
				err = "Already on first sample";
				break;
			// The next sample case now, like for the prev one
			// there is two case depending on do we skip the already
			// labeled ones.
			case 'j':
			case 201:
				while (spl->nxt != NULL) {
					spl = spl->nxt;
					if (spl->ett == 0)
						break;
				}
				break;
			case 'J':
				if (spl->nxt != NULL)
					spl = spl->nxt;
				else
					err = "Already on last sample";
				break;
			// Tagging. The label can be selected either by moving
			// left and right in the list or by directly selecting
			// the good label.
			case 'l': case 'L': case 202:
				if (spl->ett != nlbls - 1) spl->ett++;
				break;
			case 'h': case 'H': case 203:
				if (spl->ett != 0) spl->ett--;
				break;
			case 'q': spl->ett = 0; break;
			case 's': spl->ett = 1; break;
			case 'd': spl->ett = 2; break;
			case 'f': spl->ett = 3; break;
			case 'g': spl->ett = 4; break;
			// Space is the reset and skip command, it set the label
			// to unknown and go to the next unlabelled sample.
			case ' ':
				spl->ett = 0;
				while (spl->nxt != NULL) {
					spl = spl->nxt;
					if (spl->ett == 0)
						break;
				}
				break;
		}
	}
    exit:
	return EXIT_SUCCESS;
}

/******************************************************************************
 * This is the end...
 ******************************************************************************/


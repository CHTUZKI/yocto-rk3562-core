/*
 * touch-test.c — Read touch coordinates via Linux evdev (GT911 / goodix).
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(const char *argv0)
{
	fprintf(stderr,
		"Usage: %s [options] [device]\n"
		"  Default device: /dev/input/event0\n"
		"  Override with TOUCH_DEV\n"
		"Options:\n"
		"  -l  List /dev/input/event* and exit\n"
		"  -h  Show this help\n",
		argv0);
}

static int list_events(void)
{
	const char *fmt = "/dev/input/event%d";
	char path[64];
	int n = 0;

	for (int i = 0; i < 32; i++) {
		snprintf(path, sizeof(path), fmt, i);
		if (access(path, R_OK) == 0) {
			printf("%s\n", path);
			n++;
		}
	}
	return n > 0 ? 0 : 1;
}

int main(int argc, char **argv)
{
	const char *path = getenv("TOUCH_DEV");
	int opt;

	while ((opt = getopt(argc, argv, "lh")) != -1) {
		switch (opt) {
		case 'l':
			return list_events();
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (!path)
		path = (optind < argc) ? argv[optind] : "/dev/input/event0";

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "touch-test: cannot open %s: %s\n", path, strerror(errno));
		return 1;
	}

	fprintf(stderr, "Listening on %s (Ctrl+C to quit)\n", path);

	struct input_event ev;
	int x = -1;
	int y = -1;
	int btn = 0;

	for (;;) {
		ssize_t n = read(fd, &ev, sizeof(ev));
		if (n != (ssize_t)sizeof(ev)) {
			if (n < 0 && errno == EINTR)
				continue;
			break;
		}

		if (ev.type == EV_ABS) {
			if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X)
				x = ev.value;
			else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y)
				y = ev.value;
		} else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
			btn = ev.value;
		} else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
			if (btn && x >= 0 && y >= 0)
				printf("touch: x=%d y=%d\n", x, y);
			fflush(stdout);
		}
	}

	close(fd);
	return 0;
}

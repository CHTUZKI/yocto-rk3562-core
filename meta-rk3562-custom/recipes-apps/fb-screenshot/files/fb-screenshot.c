/*
 * fb-screenshot.c — Capture /dev/fb0 to an uncompressed BMP file.
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#pragma pack(push, 1)
typedef struct {
	uint16_t type;
	uint32_t size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;
} BmpFileHeader;

typedef struct {
	uint32_t header_size;
	int32_t width;
	int32_t height;
	uint16_t planes;
	uint16_t bpp;
	uint32_t compression;
	uint32_t image_size;
	int32_t x_ppm;
	int32_t y_ppm;
	uint32_t colors_used;
	uint32_t colors_important;
} BmpInfoHeader;
#pragma pack(pop)

static void usage(const char *argv0)
{
	fprintf(stderr,
		"Usage: %s [options]\n"
		"  Capture the Linux framebuffer to a BMP image.\n"
		"\n"
		"Options:\n"
		"  -d DEV   Framebuffer device (default: /dev/fb0)\n"
		"  -o FILE  Output path (default: $HOME/screenshot-YYYYmmdd-HHMMSS.bmp)\n"
		"  -h       Show this help\n"
		"\n"
		"If $HOME is unset, files are saved under /root/.\n",
		argv0);
}

static int default_output_path(char *buf, size_t buflen)
{
	const char *home = getenv("HOME");
	time_t now;
	struct tm tm;

	if (!home || home[0] == '\0') {
		home = "/root";
	}

	now = time(NULL);
	if (!localtime_r(&now, &tm)) {
		return -1;
	}

	return snprintf(buf, buflen, "%s/screenshot-%04d%02d%02d-%02d%02d%02d.bmp",
			home,
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec) >= (int)buflen ? -1 : 0;
}

static int write_bmp24(const char *path, const uint8_t *fb, int width, int height,
		       int line_length, int bpp)
{
	FILE *fp;
	BmpFileHeader fh;
	BmpInfoHeader ih;
	const int row_bytes = ((width * 3 + 3) / 4) * 4;
	uint8_t *row;
	int y;

	if (bpp != 16 && bpp != 24 && bpp != 32) {
		fprintf(stderr, "fb-screenshot: unsupported bpp %d\n", bpp);
		return -1;
	}

	row = calloc(1, (size_t)row_bytes);
	if (!row) {
		return -1;
	}

	fp = fopen(path, "wb");
	if (!fp) {
		fprintf(stderr, "fb-screenshot: cannot open %s: %s\n", path, strerror(errno));
		free(row);
		return -1;
	}

	memset(&fh, 0, sizeof(fh));
	memset(&ih, 0, sizeof(ih));
	fh.type = 0x4D42;
	fh.offset = sizeof(fh) + sizeof(ih);
	fh.size = fh.offset + (uint32_t)(row_bytes * height);
	ih.header_size = sizeof(ih);
	ih.width = width;
	ih.height = height;
	ih.planes = 1;
	ih.bpp = 24;
	ih.image_size = (uint32_t)(row_bytes * height);

	if (fwrite(&fh, sizeof(fh), 1, fp) != 1 ||
	    fwrite(&ih, sizeof(ih), 1, fp) != 1) {
		fprintf(stderr, "fb-screenshot: write header failed\n");
		fclose(fp);
		free(row);
		return -1;
	}

	for (y = height - 1; y >= 0; --y) {
		const uint8_t *src = fb + (size_t)y * (size_t)line_length;
		int x;

		for (x = 0; x < width; ++x) {
			uint8_t r;
			uint8_t g;
			uint8_t b;

			if (bpp == 32 || bpp == 24) {
				const int off = x * (bpp / 8);
				b = src[off + 0];
				g = src[off + 1];
				r = src[off + 2];
			} else {
				const uint16_t px = (uint16_t)src[x * 2] | ((uint16_t)src[x * 2 + 1] << 8);
				r = (uint8_t)(((px >> 11) & 0x1F) * 255 / 31);
				g = (uint8_t)(((px >> 5) & 0x3F) * 255 / 63);
				b = (uint8_t)((px & 0x1F) * 255 / 31);
			}

			row[x * 3 + 0] = b;
			row[x * 3 + 1] = g;
			row[x * 3 + 2] = r;
		}

		if (fwrite(row, (size_t)row_bytes, 1, fp) != 1) {
			fprintf(stderr, "fb-screenshot: write pixels failed\n");
			fclose(fp);
			free(row);
			return -1;
		}
	}

	fclose(fp);
	free(row);
	return 0;
}

int main(int argc, char **argv)
{
	const char *fbdev = "/dev/fb0";
	char outpath[512];
	const char *out_arg = NULL;
	int opt;
	int fd;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	uint8_t *fbp = NULL;
	size_t screensize;

	while ((opt = getopt(argc, argv, "d:o:h")) != -1) {
		switch (opt) {
		case 'd':
			fbdev = optarg;
			break;
		case 'o':
			out_arg = optarg;
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (out_arg) {
		snprintf(outpath, sizeof(outpath), "%s", out_arg);
	} else if (default_output_path(outpath, sizeof(outpath)) != 0) {
		fprintf(stderr, "fb-screenshot: output path too long\n");
		return 1;
	}

	fd = open(fbdev, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "fb-screenshot: cannot open %s: %s\n", fbdev, strerror(errno));
		return 1;
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0 ||
	    ioctl(fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
		fprintf(stderr, "fb-screenshot: ioctl failed: %s\n", strerror(errno));
		close(fd);
		return 1;
	}

	screensize = finfo.smem_len;
	fbp = mmap(NULL, screensize, PROT_READ, MAP_SHARED, fd, 0);
	if (fbp == MAP_FAILED) {
		fprintf(stderr, "fb-screenshot: mmap failed: %s\n", strerror(errno));
		close(fd);
		return 1;
	}

	if (write_bmp24(outpath, fbp, (int)vinfo.xres, (int)vinfo.yres,
			  (int)finfo.line_length, (int)vinfo.bits_per_pixel) != 0) {
		munmap(fbp, screensize);
		close(fd);
		return 1;
	}

	munmap(fbp, screensize);
	close(fd);

	printf("%s\n", outpath);
	return 0;
}

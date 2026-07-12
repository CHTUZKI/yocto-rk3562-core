/*
 * drm-colorblocks.c
 *
 * Minimal DRM/KMS test utility for AM335x tilcdc:
 * - opens DRM card
 * - discovers connector/crtc/plane (or uses env overrides)
 * - creates one BG24 dumb buffer
 * - default: solid full-screen red (RRGGBB FF0000)
 * - optional: vertical color blocks from DRM_COLORBLOCKS (comma-separated RRGGBB)
 *   DRM_COLORBLOCKS=FF0000,00FF00,0000FF drm-colorblocks
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

struct drm_buffer {
    uint32_t handle;
    uint32_t pitch;
    uint64_t size;
    uint8_t *map;
    uint32_t fb_id;
};

struct drm_state {
    int fd;
    uint32_t conn_id;
    uint32_t crtc_id;
    uint32_t plane_id;
    drmModeModeInfo mode;
    uint32_t width;
    uint32_t height;
    struct drm_buffer buf;
    drmModeCrtc *saved_crtc;
};

struct rgb8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

static void msleep(unsigned int ms)
{
    usleep(ms * 1000u);
}

static int parse_hex_nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    c = (char)tolower((unsigned char)c);
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

static int parse_hex_byte(const char *s, uint8_t *out)
{
    int hi = parse_hex_nibble(s[0]);
    int lo = parse_hex_nibble(s[1]);
    if (hi < 0 || lo < 0) return -1;
    *out = (uint8_t)((hi << 4) | lo);
    return 0;
}

static size_t parse_color_list(const char *input, struct rgb8 *out, size_t max_colors)
{
    if (!input || !*input || !out || max_colors == 0)
        return 0;

    size_t n = 0;
    const char *p = input;
    while (*p && n < max_colors) {
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        if (!*p) break;

        if (!isxdigit((unsigned char)p[0]) || !isxdigit((unsigned char)p[1]) ||
            !isxdigit((unsigned char)p[2]) || !isxdigit((unsigned char)p[3]) ||
            !isxdigit((unsigned char)p[4]) || !isxdigit((unsigned char)p[5])) {
            break;
        }

        if (parse_hex_byte(p, &out[n].r) != 0 ||
            parse_hex_byte(p + 2, &out[n].g) != 0 ||
            parse_hex_byte(p + 4, &out[n].b) != 0) {
            break;
        }

        n++;
        p += 6;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ',') p++;
    }

    return n;
}

static int create_dumb_buffer(struct drm_state *s)
{
    struct drm_mode_create_dumb create = {0};
    create.width = s->width;
    create.height = s->height;
    create.bpp = 24;

    if (ioctl(s->fd, DRM_IOCTL_MODE_CREATE_DUMB, &create) < 0) {
        perror("DRM_IOCTL_MODE_CREATE_DUMB");
        return -1;
    }

    s->buf.handle = create.handle;
    s->buf.pitch = create.pitch;
    s->buf.size = create.size;

    uint32_t handles[4] = { s->buf.handle, 0, 0, 0 };
    uint32_t pitches[4] = { s->buf.pitch, 0, 0, 0 };
    uint32_t offsets[4] = { 0, 0, 0, 0 };

    if (drmModeAddFB2(s->fd, s->width, s->height, DRM_FORMAT_BGR888,
                      handles, pitches, offsets, &s->buf.fb_id, 0) != 0) {
        perror("drmModeAddFB2");
        return -1;
    }

    struct drm_mode_map_dumb map = {0};
    map.handle = s->buf.handle;
    if (ioctl(s->fd, DRM_IOCTL_MODE_MAP_DUMB, &map) < 0) {
        perror("DRM_IOCTL_MODE_MAP_DUMB");
        return -1;
    }

    s->buf.map = mmap(NULL, s->buf.size, PROT_READ | PROT_WRITE, MAP_SHARED, s->fd, map.offset);
    if (s->buf.map == MAP_FAILED) {
        perror("mmap");
        s->buf.map = NULL;
        return -1;
    }

    memset(s->buf.map, 0, s->buf.size);
    return 0;
}

static void paint_solid(struct drm_state *s, const struct rgb8 *c)
{
    if (!s || !s->buf.map || !c || s->width == 0 || s->height == 0)
        return;

    for (uint32_t y = 0; y < s->height; y++) {
        uint8_t *row = s->buf.map + (size_t)y * s->buf.pitch;
        for (uint32_t x = 0; x < s->width; x++) {
            row[x * 3 + 0] = c->b;
            row[x * 3 + 1] = c->g;
            row[x * 3 + 2] = c->r;
        }
    }
}

static void paint_blocks(struct drm_state *s, const struct rgb8 *colors, size_t ncolors)
{
    if (!colors || ncolors == 0)
        return;

    for (uint32_t y = 0; y < s->height; y++) {
        uint8_t *row = s->buf.map + (size_t)y * s->buf.pitch;
        for (uint32_t x = 0; x < s->width; x++) {
            size_t idx = ((size_t)x * ncolors) / s->width;
            if (idx >= ncolors) idx = ncolors - 1;
            row[x * 3 + 0] = colors[idx].b;
            row[x * 3 + 1] = colors[idx].g;
            row[x * 3 + 2] = colors[idx].r;
        }
    }
}

static int read_env_u32(const char *name, uint32_t *out)
{
    const char *value = getenv(name);
    if (!value || value[0] == '\0')
        return -1;

    char *end = NULL;
    unsigned long parsed = strtoul(value, &end, 10);
    if (!end || *end != '\0')
        return -1;

    *out = (uint32_t)parsed;
    return 0;
}

static int find_connector_mode_only(struct drm_state *s, int64_t want_conn)
{
    drmModeRes *res = drmModeGetResources(s->fd);
    if (!res) return -1;

    for (int i = 0; i < res->count_connectors; i++) {
        drmModeConnector *conn = drmModeGetConnector(s->fd, res->connectors[i]);
        if (!conn) continue;

        if (want_conn >= 0 && conn->connector_id != (uint32_t)want_conn) {
            drmModeFreeConnector(conn);
            continue;
        }

        if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            s->conn_id = conn->connector_id;
            s->mode = conn->modes[0];
            s->width = conn->modes[0].hdisplay;
            s->height = conn->modes[0].vdisplay;
            drmModeFreeConnector(conn);
            drmModeFreeResources(res);
            return 0;
        }
        drmModeFreeConnector(conn);
    }

    drmModeFreeResources(res);
    return -1;
}

static int find_connector_and_mode(struct drm_state *s, int64_t want_conn)
{
    drmModeRes *res = drmModeGetResources(s->fd);
    if (!res) return -1;

    for (int i = 0; i < res->count_connectors; i++) {
        drmModeConnector *conn = drmModeGetConnector(s->fd, res->connectors[i]);
        if (!conn) continue;

        if (want_conn >= 0 && conn->connector_id != (uint32_t)want_conn) {
            drmModeFreeConnector(conn);
            continue;
        }

        if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            s->conn_id = conn->connector_id;
            s->mode = conn->modes[0];
            s->width = conn->modes[0].hdisplay;
            s->height = conn->modes[0].vdisplay;

            drmModeEncoder *enc = NULL;
            if (conn->encoder_id) enc = drmModeGetEncoder(s->fd, conn->encoder_id);
            if (!enc && conn->count_encoders > 0) {
                for (int j = 0; j < conn->count_encoders; j++) {
                    enc = drmModeGetEncoder(s->fd, conn->encoders[j]);
                    if (enc && enc->crtc_id != 0) break;
                    if (enc) {
                        drmModeFreeEncoder(enc);
                        enc = NULL;
                    }
                }
            }
            if (!enc) {
                /* Atomic KMS: connector may report encoder_id=0 until a mode is set; pick first CRTC */
                if (res->count_crtcs > 0) {
                    s->crtc_id = res->crtcs[0];
                    drmModeFreeConnector(conn);
                    drmModeFreeResources(res);
                    return s->crtc_id ? 0 : -1;
                }
                drmModeFreeConnector(conn);
                drmModeFreeResources(res);
                return -1;
            }

            s->crtc_id = enc->crtc_id;
            if (s->crtc_id == 0) {
                for (int j = 0; j < res->count_crtcs; j++) {
                    if (enc->possible_crtcs & (1u << j)) {
                        s->crtc_id = res->crtcs[j];
                        break;
                    }
                }
            }

            drmModeFreeEncoder(enc);
            drmModeFreeConnector(conn);
            drmModeFreeResources(res);
            return s->crtc_id ? 0 : -1;
        }
        drmModeFreeConnector(conn);
    }

    drmModeFreeResources(res);
    return -1;
}

static int find_plane(struct drm_state *s)
{
    drmModeRes *res = drmModeGetResources(s->fd);
    drmModePlaneRes *planes = drmModeGetPlaneResources(s->fd);
    if (!res || !planes) {
        drmModeFreePlaneResources(planes);
        drmModeFreeResources(res);
        return -1;
    }

    unsigned int crtc_index = 0;
    for (; crtc_index < (unsigned int)res->count_crtcs; crtc_index++) {
        if (res->crtcs[crtc_index] == s->crtc_id) break;
    }

    for (uint32_t i = 0; i < planes->count_planes; i++) {
        drmModePlane *plane = drmModeGetPlane(s->fd, planes->planes[i]);
        if (!plane) continue;

        if (!(plane->possible_crtcs & (1u << crtc_index))) {
            drmModeFreePlane(plane);
            continue;
        }

        for (uint32_t j = 0; j < plane->count_formats; j++) {
            if (plane->formats[j] == DRM_FORMAT_BGR888) {
                s->plane_id = plane->plane_id;
                drmModeFreePlane(plane);
                drmModeFreePlaneResources(planes);
                drmModeFreeResources(res);
                return 0;
            }
        }
        drmModeFreePlane(plane);
    }

    drmModeFreePlaneResources(planes);
    drmModeFreeResources(res);
    return -1;
}

static void cleanup(struct drm_state *s)
{
    if (s->saved_crtc) {
        drmModeSetCrtc(s->fd, s->saved_crtc->crtc_id, s->saved_crtc->buffer_id,
                       s->saved_crtc->x, s->saved_crtc->y, &s->conn_id, 1, &s->saved_crtc->mode);
        drmModeFreeCrtc(s->saved_crtc);
    }

    if (s->buf.map) munmap(s->buf.map, s->buf.size);
    if (s->buf.fb_id) drmModeRmFB(s->fd, s->buf.fb_id);

    if (s->buf.handle) {
        struct drm_mode_destroy_dumb destroy = {0};
        destroy.handle = s->buf.handle;
        ioctl(s->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
    }

    if (s->fd >= 0) close(s->fd);
}

int main(int argc, char **argv)
{
    const char *card = getenv("LV_DRM_CARD");
    const char *connector = getenv("LV_DRM_CONNECTOR");
    const char *color_list = getenv("DRM_COLORBLOCKS");
    uint32_t env_crtc = 0;
    uint32_t env_plane = 0;
    uint32_t ms = 4294967295u;
    int64_t want_conn = -1;
    struct rgb8 colors[32];
    size_t ncolors = 0;
    struct drm_state s;
    memset(&s, 0, sizeof(s));
    s.fd = -1;

    if (argc > 1) ms = (uint32_t)strtoul(argv[1], NULL, 10);
    if (!card || card[0] == '\0') card = "/dev/dri/card0";
    if (connector && connector[0] != '\0') want_conn = strtoll(connector, NULL, 10);

    (void)read_env_u32("LV_DRM_CRTC", &env_crtc);
    (void)read_env_u32("LV_DRM_PLANE", &env_plane);

    ncolors = parse_color_list(color_list, colors, 32);
    s.fd = open(card, O_RDWR | O_CLOEXEC);
    if (s.fd < 0) {
        fprintf(stderr, "drm-colorblocks: open %s failed: %s\n", card, strerror(errno));
        return 1;
    }

    if (drmSetClientCap(s.fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) != 0) {
        perror("drmSetClientCap");
        cleanup(&s);
        return 1;
    }

    if (env_crtc != 0) {
        if (find_connector_mode_only(&s, want_conn) != 0) {
            fprintf(stderr, "drm-colorblocks: failed to find connector/mode\n");
            cleanup(&s);
            return 1;
        }
    } else if (find_connector_and_mode(&s, want_conn) != 0) {
        fprintf(stderr, "drm-colorblocks: failed to find connector/mode/crtc\n");
        cleanup(&s);
        return 1;
    }

    if (env_crtc != 0) s.crtc_id = env_crtc;
    if (env_plane != 0) {
        s.plane_id = env_plane;
    } else if (find_plane(&s) != 0) {
        fprintf(stderr, "drm-colorblocks: failed to find BG24 plane\n");
        cleanup(&s);
        return 1;
    }

    s.saved_crtc = drmModeGetCrtc(s.fd, s.crtc_id);
    if (!s.saved_crtc || create_dumb_buffer(&s) != 0) {
        cleanup(&s);
        return 1;
    }

    if (ncolors == 0) {
        static const struct rgb8 solid_red = { 255, 0, 0 };
        paint_solid(&s, &solid_red);
        fprintf(stderr, "drm-colorblocks: connector=%u crtc=%u plane=%u %ux%u BG24 solid=FF0000\n",
                s.conn_id, s.crtc_id, s.plane_id, s.width, s.height);
    } else {
        paint_blocks(&s, colors, ncolors);
        fprintf(stderr, "drm-colorblocks: connector=%u crtc=%u plane=%u %ux%u BG24 custom-blocks=%lu\n",
                s.conn_id, s.crtc_id, s.plane_id, s.width, s.height, (unsigned long)ncolors);
    }

    if (drmModeSetCrtc(s.fd, s.crtc_id, s.buf.fb_id, 0, 0, &s.conn_id, 1, &s.mode) != 0) {
        perror("drmModeSetCrtc");
        cleanup(&s);
        return 1;
    }

    if (drmModeSetPlane(s.fd, s.plane_id, s.crtc_id, s.buf.fb_id, 0,
                        0, 0, s.width, s.height, 0, 0, s.width << 16, s.height << 16) != 0) {
        perror("drmModeSetPlane");
        cleanup(&s);
        return 1;
    }

    msleep(ms);
    cleanup(&s);
    return 0;
}

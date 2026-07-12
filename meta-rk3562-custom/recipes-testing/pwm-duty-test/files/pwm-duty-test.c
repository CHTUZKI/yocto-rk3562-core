// SPDX-License-Identifier: MIT
/*
 * Sweep duty cycle on HD-RK3506B PWM0_CH0 / PWM0_CH1 (module pins 106/107).
 * Uses Linux sysfs PWM. Stop with Ctrl+C.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PWM_PERIOD_NS	1000000ULL	/* 1 kHz */
#define MIN_DUTY_PCT	5
#define MAX_DUTY_PCT	95
#define SWEEP_MS	2500
#define STEP_MS		20

static const char *const PWM_ALIASES[] = {
	"pwm0",	/* GPIO0_B1 / pin 106 / PWM0_CH0 -> pwm@ff930000 */
	"pwm1",	/* GPIO0_B2 / pin 107 / PWM0_CH1 -> pwm@ff931000 */
};

struct pwm_channel {
	char chip_path[128];
	char pwm_path[160];
	char label[32];
	bool exported_by_us;
	bool enabled;
};

static volatile sig_atomic_t keep_running = 1;

static void on_signal(int sig)
{
	(void)sig;
	keep_running = 0;
}

static int write_sysfs(const char *path, const char *value)
{
	int fd = open(path, O_WRONLY);
	ssize_t len;
	ssize_t ret;

	if (fd < 0)
		return -1;

	len = (ssize_t)strlen(value);
	ret = write(fd, value, len);
	close(fd);

	return (ret == len) ? 0 : -1;
}

static int read_dt_alias(const char *alias, char *path, size_t len)
{
	char alias_path[128];
	int fd;
	ssize_t n;

	snprintf(alias_path, sizeof(alias_path), "/proc/device-tree/aliases/%s", alias);
	fd = open(alias_path, O_RDONLY);
	if (fd < 0)
		return -1;

	n = read(fd, path, len - 1);
	close(fd);
	if (n <= 0)
		return -1;

	path[n] = '\0';
	/* trim trailing NUL from DT property */
	if (n > 0 && path[n - 1] == '\0')
		path[n - 1] = '\0';

	return path[0] ? 0 : -1;
}

static int reg_from_dt_path(const char *dt_path, char *reg, size_t reglen)
{
	const char *at = strrchr(dt_path, '@');

	if (!at || !at[1])
		return -1;

	snprintf(reg, reglen, "%s", at + 1);
	return 0;
}

static int read_device_link(const char *chip_path, char *target, size_t len)
{
	char path[256];
	ssize_t n;

	snprintf(path, sizeof(path), "%s/device", chip_path);
	n = readlink(path, target, len - 1);
	if (n < 0)
		return -1;

	target[n] = '\0';
	return 0;
}

static int find_pwmchip(const char *alias, char *chip_path, size_t len)
{
	char dt_path[128];
	char reg[16];
	DIR *dir = opendir("/sys/class/pwm");
	struct dirent *ent;

	if (read_dt_alias(alias, dt_path, sizeof(dt_path)) != 0)
		return -1;
	if (reg_from_dt_path(dt_path, reg, sizeof(reg)) != 0)
		return -1;

	if (!dir)
		return -1;

	while ((ent = readdir(dir)) != NULL) {
		char path[256];
		char devlink[256];

		if (strncmp(ent->d_name, "pwmchip", 7) != 0)
			continue;

		snprintf(path, sizeof(path), "/sys/class/pwm/%s", ent->d_name);
		if (read_device_link(path, devlink, sizeof(devlink)) != 0)
			continue;

		if (strstr(devlink, reg) != NULL) {
			snprintf(chip_path, len, "/sys/class/pwm/%s", ent->d_name);
			closedir(dir);
			return 0;
		}
	}

	closedir(dir);
	errno = ENOENT;
	return -1;
}

static int pwm_setup(struct pwm_channel *ch, const char *alias, const char *desc)
{
	char path[256];
	char value[32];

	if (find_pwmchip(alias, ch->chip_path, sizeof(ch->chip_path)) != 0) {
		fprintf(stderr, "未找到 %s (%s)，请确认设备树已启用 PWM\n",
			alias, desc);
		return -1;
	}

	snprintf(ch->label, sizeof(ch->label), "%s", desc);
	snprintf(ch->pwm_path, sizeof(ch->pwm_path), "%s/pwm0", ch->chip_path);

	if (access(ch->pwm_path, F_OK) != 0) {
		snprintf(path, sizeof(path), "%s/export", ch->chip_path);
		if (write_sysfs(path, "0") != 0) {
			fprintf(stderr, "export %s 失败: %s\n", alias, strerror(errno));
			return -1;
		}
		ch->exported_by_us = true;
	}

	snprintf(path, sizeof(path), "%s/period", ch->pwm_path);
	snprintf(value, sizeof(value), "%llu", (unsigned long long)PWM_PERIOD_NS);
	if (write_sysfs(path, value) != 0) {
		fprintf(stderr, "设置 %s period 失败: %s\n", desc, strerror(errno));
		return -1;
	}

	snprintf(path, sizeof(path), "%s/enable", ch->pwm_path);
	if (write_sysfs(path, "1") != 0) {
		fprintf(stderr, "启用 %s 失败: %s\n", desc, strerror(errno));
		return -1;
	}

	ch->enabled = true;
	printf("已启用 %s -> %s\n", desc, ch->chip_path);
	return 0;
}

static int pwm_set_duty(struct pwm_channel *ch, unsigned int duty_ns)
{
	char path[256];
	char value[32];

	snprintf(path, sizeof(path), "%s/duty_cycle", ch->pwm_path);
	snprintf(value, sizeof(value), "%u", duty_ns);
	return write_sysfs(path, value);
}

static void pwm_cleanup(struct pwm_channel *ch)
{
	char path[256];

	if (!ch->chip_path[0])
		return;

	if (ch->enabled) {
		snprintf(path, sizeof(path), "%s/enable", ch->pwm_path);
		write_sysfs(path, "0");
		ch->enabled = false;
	}

	if (ch->exported_by_us) {
		snprintf(path, sizeof(path), "%s/unexport", ch->chip_path);
		write_sysfs(path, "0");
		ch->exported_by_us = false;
	}
}

static unsigned int duty_from_step(unsigned int step, unsigned int steps, bool invert)
{
	unsigned int pct;
	unsigned int span = MAX_DUTY_PCT - MIN_DUTY_PCT;
	unsigned int pos = (step % (steps + 1)) * span / steps;

	if (invert)
		pct = MAX_DUTY_PCT - pos;
	else
		pct = MIN_DUTY_PCT + pos;

	return (unsigned int)(PWM_PERIOD_NS * pct / 100U);
}

int main(void)
{
	struct pwm_channel ch[2] = { 0 };
	const unsigned int steps = SWEEP_MS / STEP_MS;
	unsigned int step = 0;
	const char *desc[] = {
		"PWM0_CH0 (引脚106 / GPIO0_B1)",
		"PWM0_CH1 (引脚107 / GPIO0_B2)",
	};

	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);

	printf("HD-RK3506B 双路 PWM 占空比扫描测试\n");
	printf("频率: %llu Hz, 占空比: %u%%..%u%%, 周期 %u ms\n",
	       (unsigned long long)(1000000000ULL / PWM_PERIOD_NS),
	       MIN_DUTY_PCT, MAX_DUTY_PCT, SWEEP_MS);
	printf("按 Ctrl+C 停止\n\n");

	if (pwm_setup(&ch[0], PWM_ALIASES[0], desc[0]) != 0)
		goto fail;
	if (pwm_setup(&ch[1], PWM_ALIASES[1], desc[1]) != 0)
		goto fail;

	while (keep_running) {
		unsigned int duty0 = duty_from_step(step, steps, false);
		unsigned int duty1 = duty_from_step(step, steps, true);

		if (pwm_set_duty(&ch[0], duty0) != 0 ||
		    pwm_set_duty(&ch[1], duty1) != 0) {
			fprintf(stderr, "更新占空比失败: %s\n", strerror(errno));
			break;
		}

		if ((step % (steps / 5 + 1)) == 0) {
			printf("\rCH0: %3u%%  CH1: %3u%%   ",
			       (unsigned int)(duty0 * 100ULL / PWM_PERIOD_NS),
			       (unsigned int)(duty1 * 100ULL / PWM_PERIOD_NS));
			fflush(stdout);
		}

		step++;
		usleep(STEP_MS * 1000U);
	}

	printf("\n正在关闭 PWM...\n");

fail:
	pwm_cleanup(&ch[0]);
	pwm_cleanup(&ch[1]);
	return 0;
}

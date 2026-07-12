SUMMARY = "Dual PWM duty-cycle sweep test for HD-RK3562"
DESCRIPTION = "Sweep duty on PWM0_CH0/CH1 (module pins 106/107) via sysfs until Ctrl+C."
LICENSE = "MIT"

LIC_FILES_CHKSUM = "file://pwm-duty-test.c;beginline=1;endline=5;md5=22417f34484038376f79c444e7c6725d"

SRC_URI = "file://pwm-duty-test.c"

S = "${WORKDIR}"

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} -o pwm-duty-test ${WORKDIR}/pwm-duty-test.c
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/pwm-duty-test ${D}${bindir}/
}

FILES:${PN} = "${bindir}/pwm-duty-test"

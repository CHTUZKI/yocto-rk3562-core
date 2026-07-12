SUMMARY = "Touchscreen coordinate test (evdev)"
DESCRIPTION = "Reads Linux input events and prints touch x/y (Goodix GT911)."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://touch-test.c;beginline=1;endline=15;md5=9b45cc6c591d29005129d34acd21e4fc"

SRC_URI = "file://touch-test.c"

S = "${WORKDIR}"

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} -o touch-test ${WORKDIR}/touch-test.c
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/touch-test ${D}${bindir}/touch-test
}

FILES:${PN} = "${bindir}/touch-test"

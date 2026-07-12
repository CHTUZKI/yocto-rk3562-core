SUMMARY = "Framebuffer screenshot utility"
DESCRIPTION = "Capture /dev/fb0 to a BMP file under $HOME or /root."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://fb-screenshot.c;beginline=1;endline=5;md5=1bb9449bb4f4023c0eccc6a36d6fd8d7"

SRC_URI = "file://fb-screenshot.c"

S = "${WORKDIR}"

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} -o fb-screenshot ${WORKDIR}/fb-screenshot.c
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/fb-screenshot ${D}${bindir}/fb-screenshot
}

FILES:${PN} = "${bindir}/fb-screenshot"

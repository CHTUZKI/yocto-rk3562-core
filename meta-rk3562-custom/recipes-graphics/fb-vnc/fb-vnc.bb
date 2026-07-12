SUMMARY = "Framebuffer VNC screen mirror for Qt linuxfb"
DESCRIPTION = "Stream /dev/fb0 over VNC with optional uinput remote touch."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://fb-vnc.sh \
    file://fb-vnc.init \
    file://fb-vnc.default \
"

S = "${WORKDIR}"

inherit update-rc.d

INITSCRIPT_NAME = "fb-vnc"
INITSCRIPT_PARAMS = "defaults 90"

RDEPENDS:${PN} += "x11vnc"

RRECOMMENDS:${PN} += "kernel-module-uinput"

do_install() {
	install -d ${D}${sbindir}
	install -m 0755 ${WORKDIR}/fb-vnc.sh ${D}${sbindir}/

	install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${WORKDIR}/fb-vnc.init ${D}${sysconfdir}/init.d/fb-vnc

	install -d ${D}${sysconfdir}/default
	install -m 0644 ${WORKDIR}/fb-vnc.default ${D}${sysconfdir}/default/fb-vnc
}

FILES:${PN} += " \
    ${sbindir}/fb-vnc.sh \
    ${sysconfdir}/init.d/fb-vnc \
    ${sysconfdir}/default/fb-vnc \
"

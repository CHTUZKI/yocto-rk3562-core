SUMMARY = "RK3562 RGB panel display setup"
DESCRIPTION = "Activates DRM KMS mode and waits for /dev/fb0 before Qt HMI starts."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://rk3562-display-init.sh \
    file://rk3562-display-init.init \
"

S = "${WORKDIR}"

inherit update-rc.d

INITSCRIPT_NAME = "rk3562-display-init"
INITSCRIPT_PARAMS = "defaults 89"

do_install() {
	install -d ${D}${sbindir}
	install -m 0755 ${WORKDIR}/rk3562-display-init.sh ${D}${sbindir}/

	install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${WORKDIR}/rk3562-display-init.init ${D}${sysconfdir}/init.d/rk3562-display-init
}

FILES:${PN} += " \
    ${sbindir}/rk3562-display-init.sh \
    ${sysconfdir}/init.d/rk3562-display-init \
"

RDEPENDS:${PN} += "libdrm-tests"

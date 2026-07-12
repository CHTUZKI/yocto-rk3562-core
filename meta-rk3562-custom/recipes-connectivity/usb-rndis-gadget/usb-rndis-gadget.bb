SUMMARY = "USB OTG0 RNDIS network gadget"
DESCRIPTION = "Expose OTG0 as a USB RNDIS NIC (192.168.7.2) for PC tethering and SSH."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://usb-rndis-gadget.sh \
    file://usb-rndis-gadget.init \
    file://usb-rndis-gadget.default \
"

S = "${WORKDIR}"

inherit update-rc.d

INITSCRIPT_NAME = "usb-rndis-gadget"
INITSCRIPT_PARAMS = "defaults 45"

FILES:${PN} += " \
    ${sbindir}/usb-rndis-gadget.sh \
    ${sysconfdir}/init.d/usb-rndis-gadget \
    ${sysconfdir}/default/usb-rndis-gadget \
"

RDEPENDS:${PN} += "busybox"

RRECOMMENDS:${PN} += " \
    kernel-module-g-ether \
    kernel-module-libcomposite \
    kernel-module-usb-f-rndis \
    kernel-module-u-ether \
    kernel-module-dwc2 \
    kernel-module-udc-core \
"

do_install() {
	install -d ${D}${sbindir}
	install -m 0755 ${WORKDIR}/usb-rndis-gadget.sh ${D}${sbindir}/

	install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${WORKDIR}/usb-rndis-gadget.init ${D}${sysconfdir}/init.d/usb-rndis-gadget

	install -d ${D}${sysconfdir}/default
	install -m 0644 ${WORKDIR}/usb-rndis-gadget.default ${D}${sysconfdir}/default/usb-rndis-gadget
}

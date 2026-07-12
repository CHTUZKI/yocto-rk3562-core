SUMMARY = "Auto-connect USB WiFi (MT7601U) on boot"
DESCRIPTION = "SysV init script: wait for wlan0, connect via wpa_supplicant + udhcpc."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://wifi-autoconnect.sh;beginline=1;endline=5;md5=91fb3f886518b1db09eaa1420f530dd7"

SRC_URI = " \
    file://wifi-autoconnect.sh \
    file://wifi-autoconnect.init \
"

S = "${WORKDIR}"

inherit update-rc.d

INITSCRIPT_NAME = "wifi-autoconnect"
INITSCRIPT_PARAMS = "defaults 99"

FILES:${PN} += " \
    ${bindir}/wifi-autoconnect.sh \
    ${sysconfdir}/init.d/wifi-autoconnect \
"

RDEPENDS:${PN} += "wpa-supplicant iw busybox kernel-module-mt7601u wireless-regdb"

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${WORKDIR}/wifi-autoconnect.sh ${D}${bindir}/

	install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${WORKDIR}/wifi-autoconnect.init ${D}${sysconfdir}/init.d/wifi-autoconnect
}

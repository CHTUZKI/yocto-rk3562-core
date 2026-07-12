SUMMARY = "DRM color blocks test for Rockchip RGB panels"
DESCRIPTION = "Render solid colors or vertical color blocks via DRM/KMS (rockchip driver)."
LICENSE = "MIT"

LIC_FILES_CHKSUM = "file://drm-colorblocks.c;beginline=1;endline=22;md5=9ff8e0763bdca9dbc93803fbef7abc5b"

DEPENDS = "libdrm"

RDEPENDS:${PN} = "libdrm-tests"

SRC_URI = " \
    file://drm-colorblocks.c \
    file://drm-colorblocks.sh \
"

S = "${WORKDIR}"

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} \
		-I${STAGING_INCDIR}/libdrm \
		-o drm-colorblocks.bin \
		${WORKDIR}/drm-colorblocks.c \
		-ldrm
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/drm-colorblocks.bin ${D}${bindir}/
	install -m 0755 ${WORKDIR}/drm-colorblocks.sh ${D}${bindir}/drm-colorblocks
}

FILES:${PN} = "${bindir}/drm-colorblocks ${bindir}/drm-colorblocks.bin"

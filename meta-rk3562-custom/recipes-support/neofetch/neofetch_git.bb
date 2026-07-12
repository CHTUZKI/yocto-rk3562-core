SUMMARY = "CLI system information tool"
DESCRIPTION = "Neofetch displays system information alongside an ASCII OS logo."
HOMEPAGE = "https://github.com/dylanaraps/neofetch"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=d300b86297c170b6498705fbb6794e3f"

PV = "7.1.0"
SRC_URI = "git://github.com/dylanaraps/neofetch.git;protocol=https;nobranch=1"
SRCREV = "60d07dee6b76769d8c487a40639fb7b5a1a7bc85"

S = "${WORKDIR}/git"

inherit allarch

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/neofetch ${D}${bindir}/neofetch

	install -d ${D}${mandir}/man1
	install -m 0644 ${S}/neofetch.1 ${D}${mandir}/man1/neofetch.1
}

RDEPENDS:${PN} = "bash coreutils gawk grep sed procps util-linux"

# Rockchip/Yocto openssh fixes for RK3562 (ARM64):
# 1. make install leaves a 0-byte ssh-keygen in ${D}
# 2. seccomp sandbox kills the privsep preauth child during KEX (Connection reset)

EXTRA_OECONF:append = " --without-sandbox"

do_install:append() {
	install -m 0755 ${B}/ssh-keygen ${D}${bindir}/ssh-keygen
}

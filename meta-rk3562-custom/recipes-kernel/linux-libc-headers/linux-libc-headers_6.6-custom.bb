# Copyright (C) 2024, Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)
#
# Kernel headers from Rockchip official BSP (same tree as linux-rockchip 6.6)

require recipes-kernel/linux-libc-headers/linux-libc-headers.inc

inherit local-git

SRCREV = "${AUTOREV}"
KBRANCH = "develop-6.6"
SRC_URI = " \
	git://github.com/rockchip-linux/kernel.git;protocol=https;branch=${KBRANCH} \
"

S = "${WORKDIR}/git"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

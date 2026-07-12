# Copyright (C) 2024, Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)
#
# Rockchip official BSP kernel for RK3562 (develop-6.6)

require recipes-kernel/linux/linux-yocto.inc
require linux-rockchip.inc

inherit local-git

SRCREV = "${AUTOREV}"
KBRANCH = "develop-6.6"
SRC_URI = " \
	git://github.com/rockchip-linux/kernel.git;protocol=https;branch=${KBRANCH} \
	file://${THISDIR}/files/cgroups.cfg \
"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

KERNEL_VERSION_SANITY_SKIP = "1"
LINUX_VERSION ?= "6.6"

SRC_URI:append = " \
    file://${THISDIR}/files/ext4.cfg \
    file://${THISDIR}/files/display.cfg \
    file://${THISDIR}/files/wifi_usb.cfg \
    file://${THISDIR}/files/gt911.cfg \
    file://${THISDIR}/files/goodix-keep-panel-resolution.patch \
    file://${THISDIR}/files/usb-rndis.cfg \
    file://${THISDIR}/files/input-uinput.cfg \
"

COMPATIBLE_MACHINE = "hd-rk3562-.*"

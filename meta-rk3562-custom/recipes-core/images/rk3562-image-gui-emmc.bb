SUMMARY = "GUI eMMC image for HD-RK3562-CORE"
DESCRIPTION = "Extends the minimal eMMC image with Qt5 (linuxfb) and Python 3."

require recipes-core/images/rk3562-image-minimal-emmc.bb

IMAGE_INSTALL += "packagegroup-rk3562-qt-python rk3562-display-init"

IMAGE_ROOTFS_EXTRA_SPACE ?= "524288"

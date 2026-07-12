SUMMARY = "GUI eMMC image for HD-RK3562-CORE"
DESCRIPTION = "Extends the minimal eMMC image with Qt5 and Python 3."

require recipes-core/images/rk3562-image-minimal-emmc.bb

IMAGE_INSTALL += "packagegroup-rk3562-qt-python"

IMAGE_ROOTFS_EXTRA_SPACE ?= "524288"

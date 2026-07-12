SUMMARY = "Common debug and hardware test tools for HD-RK3562-CORE"
DESCRIPTION = "Shell utilities and GPIO/I2C tools."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit packagegroup

RDEPENDS:${PN} = "\
    curl \
    wget \
    nano \
    htop \
    tree \
    lsof \
    ethtool \
    usbutils \
    i2c-tools \
    libgpiod-tools \
"

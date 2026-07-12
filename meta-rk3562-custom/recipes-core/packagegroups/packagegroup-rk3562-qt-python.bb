SUMMARY = "Qt5 and Python runtime for HD-RK3562-CORE"
DESCRIPTION = "Qt5 base/QML plus Python 3 (Yocto scarthgap default) for application development."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Must be set before inherit packagegroup (otherwise allarch is selected and
# Debian-renamed Qt/fontconfig deps fail at package time).
PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS:${PN} = "\
    python3 \
    python3-pip \
    python3-setuptools \
    python3-json \
    python3-sqlite3 \
    python3-threading \
    python3-multiprocessing \
    python3-compression \
    python3-misc \
    qtbase \
    qtbase-plugins \
    qtbase-tools \
    qtdeclarative \
    qtdeclarative-plugins \
    qtquickcontrols2 \
    qtsvg \
    qtsvg-plugins \
    qttranslations \
    fontconfig \
    ttf-dejavu-sans \
    ttf-wqy-zenhei \
"

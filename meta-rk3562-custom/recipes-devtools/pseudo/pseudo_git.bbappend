# Host GNU tar 1.34+ uses openat2 (CVE-2025-45582); pseudo 1.9.0 breaks
# perform_packagecopy on WSL2 and recent distros.  Yocto 5.0.16 ships 1.9.3.
PV = "1.9.3+git"
SRCREV = "9ab513512d8b5180a430ae4fa738cb531154cdef"

SRC_URI:remove = " \
    file://0001-configure-Prune-PIE-flags.patch \
    file://glibc238.patch \
    file://older-glibc-symbols.patch \
"
SRC_URI:append:class-native = " file://older-glibc-symbols.patch"
SRC_URI:append:class-nativesdk = " file://older-glibc-symbols.patch"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

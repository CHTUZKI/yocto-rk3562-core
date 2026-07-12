# HD-RK3562-CORE U-Boot (AArch32 SPL on AArch64 Linux machine)

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://disable-arm64.cfg"

UBOOT_ARCH = "arm"

EXTRA_OEMAKE:append = " ARCH=arm"
# GCC 13+ (Scarthgap): vendor U-Boot triggers new -Werror diagnostics
EXTRA_OEMAKE:append = ' KCFLAGS="-Wno-error"'

do_configure:append() {
    if [ -f "${B}/.config" ] && grep -q '^CONFIG_ARM64=y' "${B}/.config"; then
        sed -i '/^CONFIG_ARM64=/d' "${B}/.config"
        echo "# CONFIG_ARM64 is not set" >> "${B}/.config"
        oe_runmake -C ${S} O=${B} olddefconfig
    fi
}

do_deploy:append() {
    cd "${DEPLOYDIR}" || exit 1
    for link in loader.bin idblock.img; do
        [ -L "${link}" ] && [ ! -e "${link}" ] && rm -f "${link}"
        versioned=$(ls -1 "${link}-"* 2>/dev/null | head -1)
        if [ -n "${versioned}" ] && [ ! -e "${link}" ]; then
            ln -sf "$(basename "${versioned}")" "${link}"
        fi
    done
    if [ -L uboot.img ] && [ ! -e uboot.img ]; then
        rm -f uboot.img
    fi
    if [ ! -e uboot.img ]; then
        uboot_cand=$(ls -1 uboot*.img u-boot*.img 2>/dev/null | grep -v '^uboot.img$' | head -1)
        [ -n "${uboot_cand}" ] && ln -sf "$(basename "${uboot_cand}")" uboot.img
    fi
}

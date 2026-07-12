# Vanxak HD-RK3562-CORE custom device tree

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
    file://hd-rk3562-core.dts;subdir=git/arch/arm64/boot/dts/rockchip \
    file://hd-rk3562-core.dtsi;subdir=git/arch/arm64/boot/dts/rockchip \
"

do_configure:append() {
    makefile="${S}/arch/${ARCH}/boot/dts/rockchip/Makefile"
    [ -f "${makefile}" ] || return 0

    sed -i '/^[[:space:]]*hd-rk3562-core\.dtb[[:space:]]*$/d' "${makefile}"
    sed -i '/^hd-rk3562-core\.dtb/d' "${makefile}"

    grep -q $'\thd-rk3562-core.dtb \\' "${makefile}" && return 0

    awk '
        /^[[:space:]]*rk3562-evb1-lp4x-v10\.dtb \\$/ && !done {
            print "\thd-rk3562-core.dtb \\"
            done=1
        }
        { print }
    ' "${makefile}" > "${makefile}.tmp" && mv "${makefile}.tmp" "${makefile}"
}

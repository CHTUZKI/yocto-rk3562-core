# HD-RK3562-CORE U-Boot deploy symlink fixes

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

SUMMARY = "Minimal eMMC image for HD-RK3562-CORE"
DESCRIPTION = "Minimal Linux eMMC image for Vanxak HD-RK3562-CORE with update.img output"

inherit core-image
inherit rk3562-image
inherit rockchip-updateimg

IMAGE_INSTALL = " \
    packagegroup-core-boot \
    kernel-modules \
    kernel-image \
    kernel-devicetree \
    ${CORE_IMAGE_EXTRA_INSTALL} \
"

IMAGE_INSTALL += "u-boot-rockchip neofetch packagegroup-rk3562-devtools drm-colorblocks touch-test \
    linux-firmware-mt7601u wpa-supplicant iw wifi-autoconnect usb-rndis-gadget fb-vnc ssh-pregen-hostkeys \
"

IMAGE_FEATURES += " \
    debug-tweaks \
    ssh-server-openssh \
"

IMAGE_FSTYPES = "wic wic.bmap ext4"

# Optional debug packages that break or bloat the minimal rootfs
PACKAGE_EXCLUDE += "strace mdadm"

WKS_FILE = "rk3562-gptdisk.wks.in"

# SPL+FIT: trust is embedded in uboot.img, no separate trust.img
# Parameter follows Firefly RK3562 eMMC FIT layout (uboot/misc/boot/rootfs).
# idblock.img must be in update.img — Loader-mode upgrade does not refresh SPL otherwise.
RK_UPDATEIMG_SOC = "auto"
RK_UPDATEIMG_FLASH_IDBLOCK = "1"
RK_UPDATEIMG_PARAMETER_MODE = "manual"
# RKDevTool 整包升级会跳过 :grow / 剩余空间分区，rootfs 必须写固定扇区数。
# IMAGE_ROOTFS_SIZE=2097152 KiB → rootfs.img 2 GiB → 0x400000 扇区 (512B)
RK_UPDATEIMG_PARAMETER_CMDLINE = "mtdparts=rk29xxnand:0x00002000@0x00002000(uboot),0x00000800@0x00004000(misc),0x00010000@0x00004800(boot),0x00400000@0x00014800(rootfs)"
RK_UPDATEIMG_ROOTDEV = "PARTLABEL=rootfs"
RK_UPDATEIMG_REQUIRED_IMAGES = "loader.bin idblock.img uboot.img boot.img misc.img rootfs.img"
RK_UPDATEIMG_OPTIONAL_IMAGES = "trust.img"

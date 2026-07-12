# Common configuration for HD-RK3562-CORE (self-contained, no meta-rockchip)

require conf/machine/include/arm/armv8a/tune-cortexa53.inc

TARGET_ARCH = "aarch64"
MACHINEOVERRIDES .= ":rockchip"

# Bootloader / kernel providers
PREFERRED_PROVIDER_virtual/bootloader = "u-boot-rockchip"
PREFERRED_PROVIDER_virtual/kernel = "linux-rockchip"
PREFERRED_VERSION_linux-rockchip ?= "6.6%"

# Rockchip SoC
RK_SOC_FAMILY = "rk3562"
RK_UBOOT_SPL = "1"
KBUILD_DEFCONFIG = "hd-rk3562-core_defconfig"
KBUILD_DEFCONFIGS = "rockchip_linux_defconfig rk3562.config"

# RK3562 U-Boot is AArch32-only (arch/arm); Linux runs AArch64.
UBOOT_ARCH = "arm"

# Rockchip fiq-debugger (ttyFIQ0) 默认 1500000；PC 串口工具须设相同波特率
SERIAL_CONSOLES = "1500000;ttyFIQ0"

# No virtual VT on headless core module; avoid respawning getty on missing /dev/tty1
USE_VT = "0"

# Kernel (aarch64)
KERNEL_IMAGETYPE = "Image"
ROCKCHIP_KERNEL_IMAGES = "1"
ROCKCHIP_KERNEL_COMPRESSED = "0"
LINUXLIBCVERSION = "6.6-custom%"
OLDEST_KERNEL = "${@d.getVar('LINUXLIBCVERSION').split('-')[0]}"

# Boot device / image layout
BOOT_DEVICE = "mmc"
UBOOT_SUFFIX = "img"
UBOOT_BINARY = "uboot.img"

WKS_FILE = "rk3562-gptdisk.wks.in"
RK_ROOTFS_TYPE = "ext4"
RK_ROOTFS_EXTRAOPTS = "-F -i 8192 -b 4096"
RK_ROOTDEV_UUID = "614e0000-0000-4b53-8000-1d28000054a9"
RK_PARTITION_GROW = "1"

MACHINE_EXTRA_RRECOMMENDS += "kernel-modules"

# Bypass patch QA noise from vendor trees
ERROR_QA:remove = "patch-status patch-fuzz"
WARN_QA:remove = "patch-fuzz"

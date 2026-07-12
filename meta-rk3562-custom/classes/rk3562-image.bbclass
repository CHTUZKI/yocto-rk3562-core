# HD-RK3562-CORE image convenience helpers.

export RK_ROOTDEV_UUID ?= "614e0000-0000-4b53-8000-1d28000054a9"
export RK_PARTITION_GROW ?= "1"

ROCKCHIP_KERNEL_IMAGES = "1"

# misc.img is identical empty padding; multiple image recipes share DEPLOY_DIR_IMAGE.
SSTATE_ALLOW_OVERLAP_FILES += "${DEPLOY_DIR_IMAGE}/misc.img"

IMAGE_POSTPROCESS_COMMAND:append = " link_latest_image;"
link_latest_image() {
    rm -rf "${TOPDIR}/latest"
    ln -sf "${DEPLOY_DIR_IMAGE}" "${TOPDIR}/latest"
}

# RK3562 CORE Yocto 项目

基于 Yocto Project **Scarthgap 5.0 LTS** 的 **Vanxak HD-RK3562-CORE** 核心板嵌入式 Linux 构建环境。

与同系列的 [yocto-rk3506b-core](https://github.com/CHTUZKI/yocto-rk3506b-core) 相比，本项目面向 **RK3562 核心板模组**（LP4X + eMMC），采用 **自包含 BSP**，不依赖 `meta-rockchip`，并通过子模块 `meta-rockchip-updateimg` 生成 RKDevTool 可用的 `update.img`。

## 硬件参数

**HD-RK3562-CORE**

| 项目 | 参数 |
|------|------|
| 处理器 | Rockchip RK3562（4× Cortex-A53） |
| 架构 | AArch64 |
| 内存 | LP4X（由 rkbin DDR 初始化） |
| 存储 | eMMC（板载，非 SD 卡） |
| 调试串口 | UART0 / ttyFIQ0 @ **1500000** |
| 烧录口 | USB2.0 OTG（Maskrom / Loader） |
| 设备树 | `rockchip/hd-rk3562-core.dtb` |

核心板设备树仅启用模组必要外设（eMMC、串口、USB OTG 等）；载板外设（显示、以太网、WiFi 等）默认关闭，可在载板 DTS 中按需开启。

## 项目结构

```
yocto-rk3562-core/
├── poky/                      # Yocto 官方发行版核心层 (scarthgap / 5.0.9)
├── meta-openembedded/         # OpenEmbedded 元层 (scarthgap)
├── meta-qt5/                  # Qt5 层 (scarthgap)
├── meta-rk3562-custom/        # HD-RK3562-CORE 自包含 BSP 层
├── meta-rockchip-updateimg/   # update.img 打包层（git 子模块）
└── build/                     # BitBake 构建目录
```

### 层版本对齐

| 层 | 分支/标签 | 系列 |
|----|-----------|------|
| poky | `yocto-5.0.9` | scarthgap |
| meta-openembedded | scarthgap | scarthgap |
| meta-qt5 | scarthgap | scarthgap |
| meta-rockchip-updateimg | main | scarthgap+ |
| meta-rk3562-custom | main | scarthgap |

## 启动链

```
idblock → uboot.img (含 trust) → boot.img → rootfs (ext4)
```

烧录工具使用 `update.img`（由 meta-rockchip-updateimg 生成）。芯片标识为 **RK3562**。

## 快速开始

### 构建准备

```bash
sudo apt-get update
sudo apt-get install build-essential chrpath cpio debianutils diffstat file gawk gcc git \
  iputils-ping libacl1 lz4 locales python3 python3-jinja2 python3-pexpect python3-pip \
  python3-subunit socat texinfo unzip wget xz-utils zstd
```

### 克隆与子模块

```bash
git clone --recursive https://github.com/CHTUZKI/yocto-rk3562-core.git
cd yocto-rk3562-core
git submodule update --init --recursive
```

若子模块未初始化，可手动克隆：

```bash
git clone --branch scarthgap https://git.yoctoproject.org/poky
cd poky && git checkout yocto-5.0.9 && cd ..
git clone --branch scarthgap https://github.com/openembedded/meta-openembedded.git
git clone --branch scarthgap https://github.com/meta-qt5/meta-qt5.git
git clone https://github.com/CHTUZKI/meta-rockchip-updateimg.git
```

### 构建

```bash
source poky/oe-init-build-env build
bitbake rk3562-image-minimal-emmc    # 最小验证镜像
bitbake rk3562-image-gui-emmc        # GUI 镜像（Qt5 + Python3）
```

### 构建产物

```
build/tmp/deploy/images/hd-rk3562-core/
├── update.img          # RKDevTool 整包烧录
├── wic/                # WIC 磁盘镜像
├── boot.img
├── uboot.img
├── idblock.img
└── loader.bin
```

## 串口配置

波特率须设为 **1500000**（Rockchip fiq-debugger 默认）：

```bash
screen /dev/ttyUSB0 1500000
```

## 设备树

| 文件 | 说明 |
|------|------|
| `meta-rk3562-custom/recipes-kernel/linux/files/hd-rk3562-core.dts` | 顶层设备树 |
| `meta-rk3562-custom/recipes-kernel/linux/files/hd-rk3562-core.dtsi` | 核心板外设配置 |
| `meta-rk3562-custom/conf/machine/hd-rk3562-core.conf` | 机器配置 |

启动成功后串口应显示：`Machine model: Vanxak HD-RK3562-CORE`

## 已知说明

- **设备树**：当前为通用核心板最小配置，若你的模组 DDR/外设与 EVB 不同，请修改 `hd-rk3562-core.dtsi` 或联系硬件厂商确认引脚。
- **镜像体积**：`update.img` 中 rootfs 分区预分配 2 GiB（RKDevTool 整包烧录需固定扇区）。
- **内核/U-Boot**：使用 `AUTOREV` 跟踪 Rockchip 官方 `develop-6.6` / `next-dev` 分支。

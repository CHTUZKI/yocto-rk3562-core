# RK3562 linuxfb: framebuffer-only VNC, no X11 desktop stack.
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " file://x11vnc-uinput-multitouch.patch"

PACKAGECONFIG:remove = "x11 xinerama avahi"

# RK3562: linuxfb on DRM fbdev (/dev/fb0), no OpenGL (simpler embedded display path).

PACKAGECONFIG_GL = "no-opengl"
PACKAGECONFIG_DISTRO = "linuxfb sql-sqlite"
PACKAGECONFIG:append = " linuxfb fontconfig udev evdev"

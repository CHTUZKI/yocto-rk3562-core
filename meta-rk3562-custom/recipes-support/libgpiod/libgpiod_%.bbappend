# Tools only — skip C++/Python bindings to keep rootfs smaller.
PACKAGECONFIG = ""
# Distro enables ptest by default; without PACKAGECONFIG[tests] no gpiod-test is built.
PTEST_ENABLED = "0"

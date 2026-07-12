# mdadm-ptest pulls strace, which does not build against linux 6.6 headers.
# Minimal image does not ship ptest packages.
RDEPENDS:${PN}-ptest:remove = "strace"

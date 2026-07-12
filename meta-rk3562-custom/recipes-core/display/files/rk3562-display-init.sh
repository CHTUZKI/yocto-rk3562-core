#!/bin/sh
# Prepare Rockchip DRM RGB panel and wait for /dev/fb0 (Qt linuxfb).

wait_fb() {
	i=0
	while [ ! -c /dev/fb0 ] 2>/dev/null && [ "$i" -lt 30 ]; do
		sleep 1
		i=$((i + 1))
	done
	[ -c /dev/fb0 ]
}

# RGB panel / DRM may finish probing shortly after local_fs.
sleep 2

wait_fb && exit 0

if [ ! -c /dev/dri/card0 ]; then
	echo "rk3562-display-init: /dev/dri/card0 missing" >&2
	exit 1
fi

if command -v modetest >/dev/null 2>&1; then
	conn=$(modetest -M rockchip -c 2>/dev/null | awk '$3 == "connected" { print $1; exit }')
	enc=$(modetest -M rockchip -c 2>/dev/null | awk '$3 == "connected" { print $2; exit }')
	if [ -n "$conn" ] && [ -n "$enc" ]; then
		modetest -M rockchip -s "${conn}@${enc}:1024x600" 2>/dev/null || true
		sleep 1
	fi
fi

if wait_fb; then
	exit 0
fi

echo "rk3562-display-init: /dev/fb0 not available" >&2
exit 1

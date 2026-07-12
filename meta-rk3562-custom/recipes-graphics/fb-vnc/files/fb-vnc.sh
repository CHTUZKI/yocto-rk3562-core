#!/bin/sh
# Mirror /dev/fb0 (1024x600 Qt linuxfb) to VNC clients.
# Remote mouse in the VNC viewer is injected as a touchscreen via uinput.
# Connect: <board-ip>:5900 (e.g. 192.168.7.2 over USB RNDIS).

FB_DEV=${FB_DEV:-/dev/fb0}
VNC_PORT=${VNC_PORT:-5900}
VNC_FPS=${VNC_FPS:-12}
VNC_WIDTH=${VNC_WIDTH:-1024}
VNC_HEIGHT=${VNC_HEIGHT:-600}
VNC_TOUCH=${VNC_TOUCH:-1}
PIDFILE=/var/run/fb-vnc.pid

if [ -f /etc/default/fb-vnc ]; then
	# shellcheck disable=SC1091
	. /etc/default/fb-vnc
fi

wait_for_fb() {
	local i=0
	while [ "$i" -lt 60 ]; do
		if [ -c "$FB_DEV" ]; then
			return 0
		fi
		i=$((i + 1))
		sleep 0.5
	done
	echo "fb-vnc: framebuffer $FB_DEV not ready" >&2
	return 1
}

ensure_uinput() {
	modprobe uinput 2>/dev/null || true
	if [ ! -e /dev/input/uinput ] && [ ! -e /dev/uinput ]; then
		mkdir -p /dev/input
		mknod -m 0660 /dev/input/uinput c 10 223 2>/dev/null || true
	fi
}

restart_hmi_if_needed() {
	:
}

start_vnc() {
	if [ -f "$PIDFILE" ] && kill -0 "$(cat "$PIDFILE")" 2>/dev/null; then
		echo "fb-vnc: already running (pid $(cat "$PIDFILE"))"
		return 0
	fi

	wait_for_fb || return 1

	WAIT_MS=$((1000 / VNC_FPS))
	[ "$WAIT_MS" -lt 20 ] && WAIT_MS=20

	if [ "$VNC_TOUCH" = "1" ]; then
		ensure_uinput
		# abs= (not touch=): inject BTN_LEFT + ABS_X/Y; Qt reads event1 via evdevmouse.
		x11vnc \
			-rawfb "map:${FB_DEV}@${VNC_WIDTH}x${VNC_HEIGHT}x32" \
			-pipeinput "UINPUT:abs=${VNC_WIDTH}x${VNC_HEIGHT}" \
			-nopw -forever -shared \
			-listen 0.0.0.0 -rfbport "${VNC_PORT}" \
			-noxdamage -nosel -noprimary -noscr -cursor none \
			-nodragging -nowf -threads \
			-wait "${WAIT_MS}" -defer "${WAIT_MS}" \
			-speeds lan -ncache 10 \
			-bg -o "${PIDFILE}" \
			-q -quiet
	else
		x11vnc \
			-rawfb "map:${FB_DEV}@${VNC_WIDTH}x${VNC_HEIGHT}x32" \
			-nopw -forever -shared \
			-listen 0.0.0.0 -rfbport "${VNC_PORT}" \
			-noxdamage -nosel -noprimary -noscr -cursor none \
			-nodragging -nowf -threads \
			-wait "${WAIT_MS}" -defer "${WAIT_MS}" \
			-speeds lan -ncache 10 \
			-bg -o "${PIDFILE}" \
			-q -quiet
	fi

	sleep 1
	restart_hmi_if_needed
}

stop_vnc() {
	if [ -f "$PIDFILE" ]; then
		kill "$(cat "$PIDFILE")" 2>/dev/null || true
		rm -f "$PIDFILE"
	fi
	killall x11vnc 2>/dev/null || true
}

case "$1" in
start)
	start_vnc
	;;
stop)
	stop_vnc
	;;
restart)
	stop_vnc
	start_vnc
	;;
*)
	echo "Usage: $0 {start|stop|restart}" >&2
	exit 1
	;;
esac

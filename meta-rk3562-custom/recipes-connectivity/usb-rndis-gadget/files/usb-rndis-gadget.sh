#!/bin/sh
# USB OTG0 RNDIS via legacy g_ether gadget (uses UDC ff740000.usb).
# Board default: 192.168.7.2/24 — configure PC USB NIC as 192.168.7.1/24.

USB_IFACE=usb0

load_modules() {
	modprobe u_ether 2>/dev/null || true
	modprobe usb_f_rndis 2>/dev/null || true
	modprobe libcomposite 2>/dev/null || true
}

start_gadget() {
	if lsmod | grep -q '^g_ether '; then
		echo "usb-rndis-gadget: g_ether already loaded"
		return 0
	fi

	load_modules
	modprobe g_ether || return 1
	echo "usb-rndis-gadget: g_ether bound to UDC"
}

stop_gadget() {
	rmmod g_ether 2>/dev/null || true
}

configure_iface() {
	local ip i

	if [ -f /etc/default/usb-rndis-gadget ]; then
		# shellcheck disable=SC1091
		. /etc/default/usb-rndis-gadget
	fi

	ip=${USB_IP:-192.168.7.2}

	i=0
	while [ "$i" -lt 30 ]; do
		if [ -d "/sys/class/net/${USB_IFACE}" ]; then
			break
		fi
		i=$((i + 1))
		sleep 0.2
	done

	if [ ! -d "/sys/class/net/${USB_IFACE}" ]; then
		echo "usb-rndis-gadget: ${USB_IFACE} not found (plug USB cable to PC?)" >&2
		return 1
	fi

	ip addr flush dev "${USB_IFACE}" 2>/dev/null || true
	ip link set "${USB_IFACE}" up
	ip addr add "${ip}/24" dev "${USB_IFACE}"
	echo "usb-rndis-gadget: ${USB_IFACE} ${ip}/24"
}

case "$1" in
start)
	start_gadget
	configure_iface
	;;
stop)
	ip link set "${USB_IFACE}" down 2>/dev/null || true
	stop_gadget
	;;
restart)
	$0 stop
	$0 start
	;;
*)
	echo "Usage: $0 {start|stop|restart}" >&2
	exit 1
	;;
esac

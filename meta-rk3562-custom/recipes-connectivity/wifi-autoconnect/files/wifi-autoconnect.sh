#!/bin/sh
# Auto-connect USB WiFi (MT7601U) on USB20_OTG1 — same SSID/PSK as yocto-fet335xs lvgl branch.

SSID="huawei_wifi7"
PSK="w141242343444"
WAIT_SEC=30

modprobe mt7601u 2>/dev/null || true

IFACE=""
i=0
while [ "$i" -lt "$WAIT_SEC" ]; do
	IFACE=$(iw dev 2>/dev/null | awk '/Interface/ {print $2; exit}')
	[ -n "$IFACE" ] && break
	i=$((i + 1))
	sleep 1
done

[ -z "$IFACE" ] && exit 0

CONF="/etc/wpa_supplicant/wpa_supplicant-${IFACE}.conf"
mkdir -p /etc/wpa_supplicant

cat > "${CONF}" <<EOF
ctrl_interface=/run/wpa_supplicant
update_config=1
country=CN

network={
    ssid="${SSID}"
    psk="${PSK}"
    key_mgmt=WPA-PSK
}
EOF

ip link set "${IFACE}" up 2>/dev/null || ifconfig "${IFACE}" up 2>/dev/null || true

killall wpa_supplicant 2>/dev/null || true
wpa_supplicant -B -i "${IFACE}" -c "${CONF}" || exit 0

killall udhcpc 2>/dev/null || true
udhcpc -i "${IFACE}" -t 5 -T 5 -n 2>/dev/null || true

exit 0

#!/bin/sh
set -eu

: "${LV_DRM_CARD:=/dev/dri/card0}"
export LV_DRM_CARD

if [ -z "${LV_DRM_CONNECTOR:-}" ]; then
	LV_DRM_CONNECTOR="$(modetest -M rockchip -c 2>/dev/null | awk '$3 == "connected" { print $1; exit }')"
fi
if [ -z "${LV_DRM_CRTC:-}" ]; then
	LV_DRM_CRTC="$(modetest -M rockchip -p 2>/dev/null | awk '
		/crtc/ && !/^[[:space:]]/ { crtc = $1; sub(/:/, "", crtc) }
		/planes:/ { if (crtc != "") { print crtc; exit } }
	')"
fi
if [ -z "${LV_DRM_PLANE:-}" ]; then
	LV_DRM_PLANE="$(modetest -M rockchip -p 2>/dev/null | awk '
		/plane/ && /formats/ { print $1; sub(/:/, "", $1); exit }
	')"
fi
export LV_DRM_CONNECTOR LV_DRM_CRTC LV_DRM_PLANE

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
	echo "Usage: drm-colorblocks [duration_ms]"
	echo "Default pattern: full-screen solid red (FF0000)"
	echo "Env:"
	echo "  DRM_COLORBLOCKS=RRGGBB[,RRGGBB...]  # vertical color blocks instead"
	echo "  LV_DRM_CARD, LV_DRM_CONNECTOR, LV_DRM_CRTC, LV_DRM_PLANE"
	echo "Examples:"
	echo "  drm-colorblocks"
	echo "  DRM_COLORBLOCKS=0000FF,00FF00,FF0000 drm-colorblocks"
	echo "  modetest -M rockchip -c    # list connectors if auto-detect fails"
	exit 0
fi

if [ -z "${LV_DRM_CONNECTOR:-}" ] || [ -z "${LV_DRM_CRTC:-}" ] || [ -z "${LV_DRM_PLANE:-}" ]; then
	echo "drm-colorblocks: failed to discover connector/crtc/plane (modetest -M rockchip)" >&2
	exit 1
fi

if [ "$#" -ge 1 ]; then
	exec /usr/bin/drm-colorblocks.bin "$1"
fi

exec /usr/bin/drm-colorblocks.bin 4294967295

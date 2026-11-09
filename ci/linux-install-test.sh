#!/bin/bash
# Install a built FT8-GO .deb into a clean container of the target system
# and check that it installs and all its programs can find their libraries.
#
#   ci/linux-install-test.sh dist/ft8-go_<version>_<target-tag>.deb
#
# A self-contained package (one with /usr/lib/ft8-go, see ci/bundle-libs.sh)
# is installed with every package repository switched off, after only its
# base dependencies, to prove it needs no downloads.
set -euo pipefail

DEB=${1:?usage: $0 <package.deb>}

export DEBIAN_FRONTEND=noninteractive
apt-get update

bundled=
if dpkg-deb -c "$DEB" | grep -q '\./usr/lib/ft8-go/'; then
  bundled=1
  # Stand-in for a desktop system: just the base packages the .deb
  # depends on (X11, sound, fonts, ...), none of Qt or the other
  # libraries it bundles.
  base=$(dpkg-deb -f "$DEB" Depends | tr -d ' ' | tr ',' ' ')
  apt-get install -y --no-install-recommends $base
  if dpkg -l | grep -E '^ii +lib(qt5|fftw3|gfortran|icu|usb-1)'; then
    echo "Base dependencies pulled in a library that should be bundled" >&2
    exit 1
  fi
  # No repositories from here on.
  rm -f /etc/apt/sources.list.d/*
  : > /etc/apt/sources.list
  rm -rf /var/lib/apt/lists/*
  apt-get update
fi

apt-get install -y "./$DEB"
dpkg -s ft8-go | grep -E '^(Package|Version|Status|Depends):'

fail=0
check_libs () {
  if ldd "$1" | grep "not found"; then
    echo "UNRESOLVED LIBRARIES: $1"; fail=1
  fi
}
for f in wsjtx jt9 wsprd rigctl-wsjtx rigctld-wsjtx; do
  if [ ! -x "/usr/bin/$f" ]; then
    echo "MISSING: /usr/bin/$f"; fail=1; continue
  fi
  check_libs "/usr/bin/$f"
done

if [ -n "$bundled" ]; then
  while IFS= read -r -d '' f; do
    check_libs "$f"
  done < <(find /usr/lib/ft8-go -name '*.so*' -type f -print0)
  # Start the GUI program far enough to load Qt and a display plugin.
  # Run it as a normal user, the way people will.
  useradd -m ft8test
  out=$(su ft8test -c 'QT_QPA_PLATFORM=offscreen QT_DEBUG_PLUGINS=1 wsjtx --version' 2>&1) || {
    echo "$out"; echo "wsjtx --version failed"; fail=1; }
  echo "$out" | grep -iE 'version|loaded library|wsjt' | head -20
  if ! echo "$out" | grep -q 'loaded library "/usr/lib/ft8-go/plugins/platforms/libqoffscreen.so"'; then
    echo "Qt didn't load its platform plugin from /usr/lib/ft8-go/plugins"; fail=1
  fi
fi
exit $fail

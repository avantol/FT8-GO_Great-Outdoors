#!/bin/bash
# Install a built FT8-GO .deb into a clean container of the target system
# and check that it installs and all its programs can find their libraries.
#
#   ci/linux-install-test.sh dist/ft8-go_<version>_<target-tag>.deb
set -euo pipefail

DEB=${1:?usage: $0 <package.deb>}

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y "./$DEB"
dpkg -s ft8-go | grep -E '^(Package|Version|Status|Depends):'

fail=0
for f in wsjtx jt9 wsprd rigctl-wsjtx rigctld-wsjtx; do
  if [ ! -x "/usr/bin/$f" ]; then
    echo "MISSING: /usr/bin/$f"; fail=1; continue
  fi
  if ldd "/usr/bin/$f" | grep "not found"; then
    echo "UNRESOLVED LIBRARIES: /usr/bin/$f"; fail=1
  fi
done
exit $fail

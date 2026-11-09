#!/bin/bash
# Build, smoke-test and package FT8-GO as a .deb inside a Debian/Ubuntu
# container. Used by .github/workflows/build.yml for every Linux target.
#
#   ci/linux-build.sh <target-tag>     e.g. ubuntu-16.04_i386
#
# Optional: FT8GO_VERSION_SUFFIX (e.g. +ci57) is appended to the package
# revision for test builds.
#
# Run from the source tree root (as root, in a throwaway container).
# The finished package is written to ./dist/ft8-go_<version>_<target-tag>.deb
set -euo pipefail

TAG=${1:?usage: $0 <target-tag>}
SRC=$(pwd)
BUILD=$SRC/build-$TAG
DIST=$SRC/dist

# If a Hamlib source tarball has been placed in the source tree (done by
# the workflow for systems whose packaged Hamlib is too old, e.g. Ubuntu
# 16.04 ships 1.2.15), build it and link it statically instead of using
# the system package.
HAMLIB_TARBALL=$(ls "$SRC"/hamlib-*.tar.gz 2>/dev/null | head -1 || true)
if [ -n "$HAMLIB_TARBALL" ]; then
  hamlib_pkgs=""
else
  hamlib_pkgs="libhamlib-dev libhamlib-utils"
fi

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends \
  build-essential gfortran cmake pkg-config file dpkg-dev \
  qtbase5-dev qtmultimedia5-dev libqt5serialport5-dev \
  qttools5-dev qttools5-dev-tools libqt5sql5-sqlite \
  libfftw3-dev $hamlib_pkgs \
  libusb-1.0-0-dev libudev-dev

extra_cmake_args=()
if [ -n "$HAMLIB_TARBALL" ]; then
  hl_build=$(mktemp -d)
  tar -xzf "$HAMLIB_TARBALL" -C "$hl_build" --strip-components=1
  (
    cd "$hl_build"
    ./configure --prefix=/usr/local \
      --disable-shared --enable-static \
      --without-cxx-binding --without-readline
    make -j"$(nproc)"
    make install
  )
  rm -rf "$hl_build"
  # Static libhamlib.a needs its private dependencies on every link line.
  extra_cmake_args+=(
    "-DCMAKE_CXX_STANDARD_LIBRARIES=$(pkg-config --static --libs-only-l --libs-only-other hamlib | sed 's/-lhamlib//')"
  )
fi

# CMAKE_POLICY_VERSION_MINIMUM lets CMake 4.x accept the old
# cmake_minimum_required(2.8.10); older CMake ignores it.
# (No -S/-B: Ubuntu 16.04 ships CMake 3.5, which predates them.)
mkdir -p "$BUILD"
(
  cd "$BUILD"
  cmake "$SRC" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DWSJT_GENERATE_DOCS=OFF \
    -DWSJT_SKIP_MANPAGES=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DFT8GO_VERSION_SUFFIX="${FT8GO_VERSION_SUFFIX:-}" \
    ${extra_cmake_args[@]+"${extra_cmake_args[@]}"}
)
cmake --build "$BUILD" -- -j"$(nproc)"

# Smoke test: simulate an FT8 signal and check jt9 decodes it.
work=$(mktemp -d)
(
  cd "$work"
  "$BUILD/ft8sim" "CQ K1ABC FN42" 1500.0 0.0 0.1 1.0 0 1 -12
  "$BUILD/jt9" -8 -d 3 000000_000001.wav | tee decode.txt
  grep -q "CQ K1ABC FN42" decode.txt
)
rm -rf "$work"

# Package.
(cd "$BUILD" && cpack -G DEB)
deb=$(ls "$BUILD"/ft8-go_*.deb)
version=$(dpkg-deb -f "$deb" Version)
build_no=$(grep -oE '^[[:space:]]*return "[0-9]+";' "$SRC/revision_utils.cpp" | grep -oE '[0-9]+')
case "$version" in
  *-"$build_no"*) ;;
  *) echo "Package version '$version' lacks the build number $build_no" >&2; exit 1 ;;
esac
mkdir -p "$DIST"
out="$DIST/ft8-go_${version}_${TAG}.deb"
cp "$deb" "$out"
dpkg-deb --info "$out"
echo "Built $out"

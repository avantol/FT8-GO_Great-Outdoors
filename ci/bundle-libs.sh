#!/bin/bash
# Make an FT8-GO .deb self-contained: copy every library it needs, and
# Qt's plugins, into /usr/lib/ft8-go, point the programs at them, and
# depend only on base packages that every desktop install of the target
# system already has. The package then installs with no downloads, so it
# works where the system's package repositories no longer do (Ubuntu
# 16.04 / Linux Lite 3.4) and with no internet connection at all.
#
#   ci/bundle-libs.sh <in.deb> <out.deb>
#
# Run as root inside the build container, after cpack, with the build's
# packages (Qt, FFTW, ...) still installed. wsjtx must have been built
# with -DFT8GO_QT_PLUGIN_DIR=/usr/lib/ft8-go/plugins.
set -euo pipefail

IN=${1:?usage: $0 <in.deb> <out.deb>}
OUT=${2:?usage: $0 <in.deb> <out.deb>}
LIBDIR=usr/lib/ft8-go
PLUGDIR=$LIBDIR/plugins

# Used from the system, never bundled: present on every Ubuntu 16.04 /
# Linux Lite 3.4 desktop, and tied to the running system (C library and
# compiler runtime, X server, graphics drivers, fonts, sound and session
# services), so a private copy could clash with it.
BASE_PKGS="
  libc6 libgcc1 libstdc++6 zlib1g
  libx11-6 libx11-xcb1 libxcb1 libxau6 libxdmcp6 libxext6 libxrender1
  libxi6 libxtst6 libxfixes3 libxdamage1 libxxf86vm1 libxshmfence1
  libsm6 libice6 libuuid1
  libgl1-mesa-glx libglapi-mesa libdrm2
  libxcb-glx0 libxcb-dri2-0 libxcb-dri3-0 libxcb-present0 libxcb-sync1
  libfontconfig1 libfreetype6 libexpat1 libpng12-0 libharfbuzz0b libgraphite2-3
  libglib2.0-0 libpcre3 libselinux1 liblzma5 libbsd0
  libdbus-1-3 libsystemd0 libgcrypt20 libgpg-error0 libudev1 libcap2
  libasound2 libpulse0 libsndfile1 libwrap0 libjson-c2 libasyncns0
  libflac8 libogg0 libvorbis0a libvorbisenc2
"
is_base () { case " $(echo $BASE_PKGS) " in *" $1 "*) return 0 ;; esac; return 1; }

is_elf () { [ "$(head -c4 "$1" 2>/dev/null | od -An -c | tr -d ' ')" = "177ELF" ]; }

stage=$(mktemp -d)
dpkg-deb -R "$IN" "$stage"
mkdir -p "$stage/$LIBDIR" "$stage/$PLUGDIR"

# Qt plugins FT8-GO uses: display (X11, plus offscreen for testing),
# OpenGL integration, audio, SQLite, image formats, input methods and
# network bearers.
qxcb=$(dpkg -L libqt5gui5 | grep '/platforms/libqxcb\.so$')
qt_plugins=$(dirname "$(dirname "$qxcb")")
for p in platforms/libqxcb.so platforms/libqoffscreen.so \
         xcbglintegrations audio sqldrivers/libqsqlite.so \
         imageformats platforminputcontexts bearer iconengines; do
  [ -e "$qt_plugins/$p" ] || continue
  mkdir -p "$stage/$PLUGDIR/$(dirname "$p")"
  cp -a "$qt_plugins/$p" "$stage/$PLUGDIR/$(dirname "$p")/"
done
for p in platforms/libqxcb.so audio sqldrivers/libqsqlite.so; do
  [ -e "$stage/$PLUGDIR/$p" ] || { echo "Qt plugin $p not found in $qt_plugins" >&2; exit 1; }
done

# Every program and plugin, and the libraries they load.
elves=()
while IFS= read -r -d '' f; do
  is_elf "$f" && elves+=("$f")
done < <(find "$stage/usr/bin" "$stage/$PLUGDIR" -type f -print0)

declare -A deps=()
bundled=0
for f in "${elves[@]}"; do
  if ldd "$f" | grep -q "not found"; then
    echo "Unresolved libraries in $f:" >&2; ldd "$f" >&2; exit 1
  fi
  while read -r soname path; do
    [ -f "$stage/$LIBDIR/$soname" ] && continue
    pkg=$(dpkg -S "$path" 2>/dev/null | head -1 | cut -d: -f1) || true
    if [ -z "$pkg" ]; then
      pkg=$(dpkg -S "$(readlink -f "$path")" 2>/dev/null | head -1 | cut -d: -f1) || true
    fi
    [ -n "$pkg" ] || { echo "No package owns $path (needed by $f)" >&2; exit 1; }
    if is_base "$pkg"; then
      deps[$pkg]=1
    else
      cp -L "$path" "$stage/$LIBDIR/$soname"
      echo "Bundled $soname (from $pkg)"
      bundled=$((bundled + 1))
    fi
  done < <(ldd "$f" | awk '$2 == "=>" && $3 ~ /^\// {print $1, $3}')
done

# Point everything at the bundled copies.
patchelf --version
for f in "${elves[@]}"; do
  case "$f" in
    "$stage/usr/bin/"*) patchelf --set-rpath '$ORIGIN/../lib/ft8-go' "$f" ;;
    "$stage/$PLUGDIR/"*) patchelf --set-rpath '$ORIGIN/../..' "$f" ;;
  esac
done
for f in "$stage/$LIBDIR"/*.so*; do
  patchelf --set-rpath '$ORIGIN' "$f"
done

# Depend on the base packages only (plus the keyboard layouts the bundled
# libxkbcommon reads, which every X desktop has).
deps[xkb-data]=1
depends=$(printf '%s\n' "${!deps[@]}" | sort | paste -sd, - | sed 's/,/, /g')
sed -i "s/^Depends:.*/Depends: $depends/" "$stage/DEBIAN/control"
installed_size=$(du -sk --exclude=DEBIAN "$stage" | cut -f1)
sed -i "s/^Installed-Size:.*/Installed-Size: $installed_size/" "$stage/DEBIAN/control"
(cd "$stage" && find . -type f ! -path './DEBIAN/*' -printf '%P\0' | sort -z | xargs -0 md5sum > DEBIAN/md5sums)

dpkg-deb --build "$stage" "$OUT"
rm -rf "$stage"
echo "Bundled $bundled libraries; Depends: $depends"

#!/bin/bash
# Check that every .exe/.dll under a directory can run on Windows 7.
#
#   ci/check-win7.sh <dir>        (OBJDUMP=<objdump able to read PE files>)
#
# Fails if any file
#  - declares a minimum OS (subsystem version) above 6.1 (Windows 7),
#  - links the Universal C Runtime (ucrtbase / api-ms-win-crt-*), which
#    Windows 7 only has with an optional update, or other api-ms-win-*
#    API sets, or
#  - imports a Windows API function that first appeared in Windows 8+.
set -euo pipefail

DIR=${1:?usage: $0 <dir>}
OBJDUMP=${OBJDUMP:-objdump}

# Kernel32/user32 functions new in Windows 8 or later that toolchains and
# libraries are known to pick up.
WIN8PLUS='GetSystemTimePreciseAsFileTime|CreateFile2|WaitOnAddress|WakeByAddressSingle|WakeByAddressAll|PrefetchVirtualMemory|GetCurrentThreadStackLimits|SetThreadDescription|GetThreadDescription|CreateRemoteThreadEx|GetOverlappedResultEx|MapViewOfFileFromApp|RoInitialize|GetPackageFamilyName|GetDpiForWindow|GetDpiForMonitor|SetProcessDpiAwarenessContext|SetThreadDpiAwarenessContext|AreDpiAwarenessContextsEqual|GetSystemMetricsForDpi|AdjustWindowRectExForDpi|EnableNonClientDpiScaling'

fail=0
count=0
while IFS= read -r -d '' f; do
  count=$((count + 1))
  p=$("$OBJDUMP" -p "$f" 2>/dev/null) || { echo "SKIP (not PE?): $f"; continue; }
  major=$(echo "$p" | awk '/MajorSubsystemVersion/{print $2}')
  minor=$(echo "$p" | awk '/MinorSubsystemVersion/{print $2}')
  problems=""
  if [ -n "$major" ] && { [ "$major" -gt 6 ] || { [ "$major" -eq 6 ] && [ "$minor" -gt 1 ]; }; }; then
    problems+=" subsystem=$major.$minor"
  fi
  dlls=$(echo "$p" | awk '/DLL Name:/{print tolower($3)}')
  if grep -qE '^(ucrtbase\.dll|api-ms-win-crt-)' <<< "$dlls"; then
    problems+=" UCRT"
  fi
  apisets=$(echo "$dlls" | grep -E '^api-ms-win-' | grep -vE '^api-ms-win-crt-' | tr '\n' ' ' || true)
  [ -n "$apisets" ] && problems+=" api-sets:[$apisets]"
  w8=$(echo "$p" | grep -oE "\b($WIN8PLUS)\b" | sort -u | tr '\n' ' ' || true)
  [ -n "$w8" ] && problems+=" win8+:[$w8]"
  if [ -n "$problems" ]; then
    echo "NOT WINDOWS 7 COMPATIBLE: ${f#$DIR/}:$problems"
    fail=1
  fi
done < <(find "$DIR" \( -iname '*.exe' -o -iname '*.dll' \) -print0)

echo "Checked $count files"
[ "$count" -gt 0 ] || { echo "no .exe/.dll found under $DIR"; exit 1; }
[ $fail -eq 0 ] && echo "All files Windows 7 compatible"
exit $fail

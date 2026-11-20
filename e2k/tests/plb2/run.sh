#!/bin/bash
# plb2 benchmarks (https://github.com/e2k-community/plb2, src/root) run with
# the cling JIT and with ACLiC (the system compiler); outputs must match.
#
# Usage: bash run.sh ROOT_DIR   (build or install directory with bin/thisroot.sh)
# Prints the wall time of each run (compilation included) and exits non-zero
# if a run fails or JIT and ACLiC results differ.
B=${1:?usage: $0 ROOT_DIR}
. "$B/bin/thisroot.sh" || exit 1
SRC=$(cd "$(dirname "$0")" && pwd)
W=$(mktemp -d "${TMPDIR:-/tmp}/root-e2k-plb2.XXXXXX"); cd "$W" || exit 1
cp "$SRC"/*.C .
echo "work dir: $W"
bad=0
for t in nqueen matmul sudoku bedcov; do
  for mode in jit aclic; do
    suf=""; [ $mode = aclic ] && suf="+"
    o=$t-$mode.txt
    s=$(date +%s.%N)
    root -b -l -q -e ".L $t.C$suf" -e "$t()" > "$o" 2>&1
    rc=$?
    el=$(awk -v a="$s" -v b="$(date +%s.%N)" 'BEGIN{printf "%.2f", b-a}')
    grep -vE '^Info in <TUnixSystem::ACLiC>' "$o" > "$o.tmp" && mv "$o.tmp" "$o"
    echo "=== $t ($mode) rc=$rc  ${el} s  lines=$(wc -l < "$o")  md5=$(md5sum < "$o" | cut -c1-8)  last: $(tail -1 "$o" | cut -c1-60)"
    [ $rc = 0 ] || bad=1
  done
  if cmp -s $t-jit.txt $t-aclic.txt; then echo "=== $t: JIT == ACLiC"; else echo "=== $t: JIT != ACLiC"; bad=1; fi
done
echo "logs: $W"
exit $bad

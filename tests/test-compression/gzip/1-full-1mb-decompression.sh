#!/usr/bin/env bash

echo -n "`basename $0`: "

BINARY="./compression-reader"
TEST_DATA_GZ="test-compression/gzip/test-data-1mb.bin.gz"

TEST_DATA="test-compression/test-data-1mb.bin"

function check {
    length=1048575
    "${BINARY}" "${TEST_DATA_GZ}" 0 $length > "$temp_file" 2> "$LOGFILE"

    cmp -s -n $length "$temp_file" "$TEST_DATA"
}

export LOGFILE="logs/gzip/`basename $0 | cut -d\- -f1`-`date +%y%m%d-%H:%M.%S`"
mkdir -p `dirname $LOGFILE`
temp_file=$(mktemp)
 if [ ! -z check ] && ! check; then
     echo "FAIL"
 else
     echo "PASS"
 fi
rm "$temp_file"

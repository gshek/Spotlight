#!/usr/bin/env bash

echo -n "`basename $0`: "

BINARY="./compression-reader"
TEST_DATA_XZ="test-compression/xz/test-data-10mb.bin.xz"

TEST_DATA="test-compression/test-data-10mb.bin"

function check {
    start_offset=123456
    length=1048575

    "${BINARY}" "${TEST_DATA_XZ}" $start_offset $length > "$temp_file" 2> "$LOGFILE"

    cmp -s -n $length "$temp_file" "$TEST_DATA" 0 $start_offset
}

export LOGFILE="logs/xz/`basename $0 | cut -d\- -f1`-`date +%y%m%d-%H:%M.%S`"
mkdir -p `dirname $LOGFILE`
temp_file=$(mktemp)
 if [ ! -z check ] && ! check; then
     echo "FAIL"
 else
     echo "PASS"
 fi
rm "$temp_file"

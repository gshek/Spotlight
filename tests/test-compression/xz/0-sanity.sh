#!/usr/bin/env bash

echo -n "`basename $0`: "

function check {
    [ -s ./compression-reader ]
}

 if [ ! -z check ] && ! check; then
     echo "FAIL"
 else
     echo "PASS"
 fi

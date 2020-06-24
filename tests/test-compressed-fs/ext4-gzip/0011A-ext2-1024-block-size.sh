#!/bin/bash
export TEST_MKE2FS_USE_EXT2=1
export MKE2FS_EXTRA_OPTIONS="-b 1024"

set -e
source `dirname $0`/lib.sh

e4test_declare_slow

source `dirname $0`/0011-file-integrity-large.sh

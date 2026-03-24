#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    echo "Usage: test_wrapper.sh [test]"
    exit 1
fi

"$1"
SIG=$?

if [ $SIG == 132 ]; then
    exit 0
else
    exit 1
fi

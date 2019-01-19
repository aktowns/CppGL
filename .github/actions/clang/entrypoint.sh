#!/usr/bin/env bash

set -o errexit 
set -o nounset
set -o pipefail 
set -e

if [[ "$1" == "test" ]]; then 
    echo "Building application"
    mkdir build
    cd build 
    cmake .. 
    make VERBOSE=1
fi

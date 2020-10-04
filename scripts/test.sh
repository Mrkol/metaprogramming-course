#!/usr/bin/env bash

set -e
cd "$(dirname "$(readlink -f "$0")")"
source util.sh

usage() {
    echo "Usage: $0 [-i <id>] [-n] <repo url> <task id>" 1>&2;
    exit 1;
}

ID=""
NOCOMPILE=""
while getopts ":i:n" opt; do
    case "$opt" in
        i)
            ID="$OPTARG"
        ;;
        n)
            NOCOMPILE="YES"
        ;;
        *) usage ;;
    esac
done
shift $(( OPTIND - 1 ))
if (( $# != 2)); then usage; fi
REPO="$1"
TASK="$2"

mkdir -p solutions

if [[ -z "$ID" ]]; then
    ID=`gen_id solutions`
fi

WORKDIR=$(realpath "solutions/$ID")
BUILDDIR="$WORKDIR/build"
TESTDIR=$(realpath "../tests")
BRANCH=task"$TASK"
CMAKE_EXTRA=""

if [[ -n "$NOCOMPILE" ]]; then
    CMAKE_EXTRA="-DNOCOMPILE=ON"
fi

git clone --single-branch --branch="$BRANCH" --depth=1 "$REPO" "$WORKDIR"
mkdir "$BUILDDIR"
cd "$BUILDDIR"

cmake -DTASK="$TASK" -DREPOSITORY_PATH="$WORKDIR" $CMAKE_EXTRA "$TESTDIR"
if [[ -n "$NOCOMPILE" ]]; then 
    set +e
    STATUS=0
    cmake --build . || STATUS=$?
    set -e

    if (( $STATUS == 0)); then
        echo "It compiles but should not" 1>&2
        exit 1
    fi
else
    cmake --build .
    ctest -V
fi

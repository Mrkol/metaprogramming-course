#!/usr/bin/env bash

set -e

REPO="$1"
TASK="$2"
ID="$3"
cd "$(dirname "$(readlink -f "$0")")"
source util.sh

mkdir -p solutions

if [[ -z "$ID" ]]; then
    ID=`gen_id solutions`
fi

WORKDIR=$(realpath "solutions/$ID")
BUILDDIR="$WORKDIR/build"
TESTDIR=$(realpath "../tests")
BRANCH=task"$TASK"

git clone --single-branch --branch="$BRANCH" --depth=1 "$REPO" "$WORKDIR"
mkdir "$BUILDDIR"
cd "$BUILDDIR"

cmake -DTASK="$TASK" -DREPOSITORY_PATH="$WORKDIR" "$TESTDIR"
cmake --build .
ctest -V

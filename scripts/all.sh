#!/usr/bin/env bash

set -e
cd "$(dirname "$(readlink -f "$0")")"
source util.sh

TASK=$1
shift 1

BRANCH=task"$TASK"

mkdir -p solutions

cat students.txt | while read repo; do
    if [[ -z "$repo" ]]; then
        continue
    fi
    echo "Looking for solution in $repo"

    git ls-remote "$repo" 'refs/heads/*' | awk -F" " '{print $2}' | awk -F"/" '{ print $3 }' | while read repo_branch; do
        if [[ "$repo_branch" == "$BRANCH" ]]; then
            ID=`gen_id solutions`
            MOUT="solutions/${ID}.out"
            MERR="solutions/${ID}.err"

            echo ">>> Testing repository $repo"

            set +e
            ./test.sh -i$ID $@ "$repo" "$TASK" > $MOUT 2> $MERR
            STATUS=$?
            set -e

            if (( $STATUS == 0 )); then
                echo $ID OK
            else
                echo $ID FAILED
            fi

            break
        fi
    done
done

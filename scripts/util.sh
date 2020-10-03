#!/usr/bin/env bash

gen_id() {
    local ROOT=$1

    local ID=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 3 | head -n 1)
    while [[ -e "$ROOT/$ID" ]]; do
        ID=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 3 | head -n 1)
    done

    echo -n $ID
}

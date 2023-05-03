#!/bin/bash

# TODO rewrite in Aquila once file access is implemented

command -v aq >/dev/null 2>&1 || { echo >&2 "Install Aquila first!"; exit 1; }

root='tests/'
temp=$(mktemp /tmp/aq.XXXXXX)

for f in "$root"*.aq; do
    echo "$f"
    g="$root"$(basename $f .aq).txt
    "$f" > "$temp"
    diff "$temp" "$g"
done

rm "$temp"

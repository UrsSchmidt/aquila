#!/bin/bash

# TODO rewrite in Aquila once file access is implemented

# run `ant package` first!

root='tests/'
temp="$root/~temp.txt"

for f in "$root"*.aq; do
    echo "$f"
    g="$root"$(basename $f .aq).txt
    "$f" > "$temp"
    diff "$temp" "$g"
done

rm "$temp"

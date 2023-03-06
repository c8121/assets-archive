#!/bin/bash

in="$1"
if [[ ! -f "$in" ]]; then
  echo "Not found: $in"
  exit
fi

out="$2"
if [[ -f "$out" ]]; then
  echo "File already exists: $out"
  exit
fi

args="$3"
if [[ "$args" != "" ]]; then
  echo "Arguments: $args"
else
  args="-background white -flatten -resize 250x250"
fi

convert "$in" $args "$out"

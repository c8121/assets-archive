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

cp "$in" "$out"

#!/bin/bash

#
# Recursively add files to archive
#

BASE=$(realpath "$(dirname "$0")")
ARCHIVE_COMMAND="$BASE/bin/archive"

source="$1"
if [ "$source" == "" ]; then
  echo "Usage: $0 <file or directory>"
  exit
fi

if [ -d "$source" ]; then
  old_ifs=$IFS
  IFS=$'\n'
  for f in $(find "$source" -type f); do
    $ARCHIVE_COMMAND "$f"
  done
  IFS=$old_ifs

elif [ -f "$source" ]; then
  $ARCHIVE_COMMAND "$source"
fi

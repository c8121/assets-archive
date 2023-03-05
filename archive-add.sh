#!/bin/bash

#
# Recursively add files to archive
#

BASE=$(realpath "$(dirname "$0")")
ARCHIVE_COMMAND="$BASE/bin/archive"
METADATA_COMMAND="$BASE/bin/archive-metadata"

source="$1"
if [ "$source" == "" ]; then
  echo "Usage: $0 <file or directory>"
  exit
fi

### Functions ###

function add_to_archive() {
  file="$1"
  if [ ! -f "$file" ]; then
    echo "File not found: $file"
    return
  fi

  hash=$($ARCHIVE_COMMAND "$file" -s)
  if [ "$hash" != "FAIL" ]; then
    $METADATA_COMMAND "$hash" "add-origin" "$file"
  else
    echo "Failed to add file ($hash)"
  fi
}

### Main ###

if [ -d "$source" ]; then

  old_ifs=$IFS
  IFS=$'\n'
  for f in $(find "$source" -type f); do
    add_to_archive "$f"
  done
  IFS=$old_ifs

else

  add_to_archive "$source"

fi

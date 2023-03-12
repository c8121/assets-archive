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
  file=$(realpath "$file")

  hash=$($ARCHIVE_COMMAND "$file" -s)
  if [ "$hash" != "FAIL" ]; then

    owner=$(stat -c '%U' "$file")
    groups=$(grep "^$(stat -c '%G' "$file"):" /etc/group | awk -F':' '{gsub(","," "); print $4}')
    filetime=$(date -r "$file" "+%Y-%m-%d %H:%M:%S")
    dirname=$(dirname "$file")
    filename=$(basename "$file")

    $METADATA_COMMAND "add-origin" "$hash" "$file" \
      -owner "$owner" -participants $groups \
      -category "$dirname" -subject "$filename" \
      -created "$filetime" -changed "$filetime"
  else
    echo "Failed to add file ($hash)"
  fi
}

### Main ###

if [ -d "$source" ]; then

  find "$source" -type f -print0 | while IFS= read -r -d '' file; do
    add_to_archive "$file"
  done

else

  add_to_archive "$source"

fi

#!/bin/bash

#
# Update meta data database
#

BASE=$(realpath "$(dirname "$0")")
METADATA_FILE_SUFFIX=".json"
METADATA_FILE_PATTERN='*'$METADATA_FILE_SUFFIX
METADATA_DB_COMMAND="$BASE/bin/archive-metadata-db"

archive_base="$1"
if [ "$archive_base" == "" ]; then
  echo "Usage: $0 <archive base directory>"
  exit
fi

### Main ###

if [ -d "$archive_base" ]; then

  find "$archive_base" -type f -name "$METADATA_FILE_PATTERN" -print0 | while IFS= read -r -d '' file; do

    echo "Read metadata from $file"

    hash=$(basename "$(dirname "$file")")$(basename "$file")
    hash="${hash%.*}"
    echo "HASH: $hash"

    $METADATA_DB_COMMAND "$hash"

  done

else

  echo "Archive base directory not found"

fi

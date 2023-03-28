#!/bin/bash

#
# Update fulltext index database
#

BASE=$(realpath "$(dirname "$0")")
ARCHIVE_FILE_SUFFIX=".archive"
ARCHIVE_FILE_PATTERN='*'$ARCHIVE_FILE_SUFFIX
EXPORT_COMMAND="$BASE/bin/archive-export"
CONVERT_COMMAND="$BASE/filter/convert-to-text.sh"
TOKENIZE_COMMAND="$BASE/bin/tokenizer"
INDEX_COMMAND="$BASE/bin/indexer"

archive_base="$1"
if [ "$archive_base" == "" ]; then
  echo "Usage: $0 <archive base directory>"
  exit
fi

### Main ###

if [ -d "$archive_base" ]; then

  find "$archive_base" -type f -name "$ARCHIVE_FILE_PATTERN" -print0 | while IFS= read -r -d '' file; do

    echo "Read metadata from $file"

    hash=$(basename "$(dirname "$file")")$(basename "$file")
    hash="${hash%.*}"
    echo "HASH: $hash"

    temp_file=$(mktemp)
    rm "$temp_file"

    $EXPORT_COMMAND "$hash" "$temp_file"
    $CONVERT_COMMAND "$temp_file" |
      $TOKENIZE_COMMAND -d $' .,;:()[]{}\\/-=<>"\'+*?!%_|$&@#\r\n\t\v\f' -n 2 -lcase -match "^[A-Za-z0-9]*$" |
      $INDEX_COMMAND "$hash"

    rm "$temp_file"

  done

else

  echo "Archive base directory not found"

fi

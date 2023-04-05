#!/bin/bash

#
# Update fulltext index database
#

BASE=$(realpath "$(dirname "$0")")
LIST_HASHES_COMMAND="$BASE/bin/archive"
EXPORT_COMMAND="$BASE/bin/archive-export"
CONVERT_COMMAND="$BASE/filter/convert-to-text.sh"
TOKENIZE_COMMAND="$BASE/bin/tokenizer"
INDEX_COMMAND="$BASE/bin/indexer"
MKTEMP_COMMAND="$BASE/bin/archive-tempname"

### Main ###

for hash in $($LIST_HASHES_COMMAND list -s); do

  echo "HASH: $hash"

  temp_file=$($MKTEMP_COMMAND)
  temp_dir=$($MKTEMP_COMMAND)
  mkdir "$temp_dir"

  $EXPORT_COMMAND "$hash" "$temp_file"
  $CONVERT_COMMAND "$temp_file" "$temp_dir" |
    $TOKENIZE_COMMAND -d $' .,;:()[]{}\\/-=<>"\'+*?!%_|$&@#\r\n\t\v\f' -n 2 -lcase -match "^[A-Za-z0-9]*$" |
    $INDEX_COMMAND "$hash"

  rm "$temp_file"

done

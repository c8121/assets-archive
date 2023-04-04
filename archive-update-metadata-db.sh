#!/bin/bash

#
# Update meta data database
#

BASE=$(realpath "$(dirname "$0")")
LIST_HASHES_COMMAND="$BASE/bin/archive"
METADATA_DB_COMMAND="$BASE/bin/archive-metadata-db"

### Main ###

for hash in $($LIST_HASHES_COMMAND list -s); do

  $METADATA_DB_COMMAND "$hash"

done

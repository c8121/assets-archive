#!/bin/bash

BASE=$(realpath "$(dirname "$0")")

sourceDir=$BASE/src
binDir=$BASE/bin
dependenciesDir=$BASE/dep

if [[ ! -d "$sourceDir" ]] ; then
	echo "Source directory not found: $sourceDir"
	exit
fi

if [[ ! -d "$dependenciesDir" ]]; then
  echo "Dependencies directory not found, please pull dependencies first: ./pull-dependencies.sh"
  exit
fi

if [[ ! -d "$binDir" ]] ; then
	echo "Create $binDir"
	mkdir -p "$binDir"
fi

cd "$BASE"
gcc -Wall -o "$binDir/archive" "$sourceDir/archive.c" -lbsd
gcc -Wall -o "$binDir/archive-metadata" "$sourceDir/archive-metadata.c" -lbsd
gcc -Wall -o "$binDir/archive-metadata-db" "$sourceDir/archive-metadata-db.c" -lbsd -lmysqlclient
gcc -Wall -o "$binDir/archive-export" "$sourceDir/archive-export.c" -lbsd
gcc -Wall -o "$binDir/archive-cat" "$sourceDir/archive-cat.c" -lbsd
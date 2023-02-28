#!/bin/bash

BASE=$(realpath "$(dirname "$0")")

sourceDir=$BASE/src
binDir=$BASE/bin

if [[ ! -d "$sourceDir" ]] ; then
	echo "Source directory not found: $sourceDir"
	exit
fi

if [[ ! -d "$binDir" ]] ; then
	echo "Create $binDir"
	mkdir -p "$binDir"
fi

cd "$BASE"
gcc -Wall -o "$binDir/archive" "$sourceDir/archive.c"
gcc -Wall -o "$binDir/archive-metadata" "$sourceDir/archive-metadata.c"
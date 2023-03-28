#!/bin/bash

BASE=$(realpath "$(dirname "$0")")

dependenciesDir=$BASE/dep

if [[ ! -d "$dependenciesDir" ]]; then
  echo "Create $dependenciesDir"
  mkdir -p "$dependenciesDir"
fi

dependencies="
	https://github.com/c8121/cutils
	https://github.com/c8121/ctextutils
	https://github.com/DaveGamble/cJSON
"

for dep in $dependencies; do

  name=$(basename "$dep")
  dir="$dependenciesDir/$name"
  echo "Dependency: $dep into $dir"

  if [ ! -d "$dir" ]; then
    cd "$dependenciesDir" || exit
    git clone "$dep"
  else
    cd "$dir" || exit
    git pull
  fi

done

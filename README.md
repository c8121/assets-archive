# assets-archive

Store files into an archive directory

- Deduplicated (same file only once, determined by content-hash)
- Organized in subdirectories, one per time period. Old directories will no be touched again to enable incremental backups.
- Files can be compressed/encrypted.

## Install dependencies

    apt install gcc
    apt install libbsd-dev

## Download & build

    git clone https://github.com/c8121/assets-archive.git
    cd assets-archive
    ./pull-dependencies.sh
    ./build.sh

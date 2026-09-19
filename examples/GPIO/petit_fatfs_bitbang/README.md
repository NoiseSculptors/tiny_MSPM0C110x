# Petit FatFs SD-card Bitbang Example

This example implements an SD card interface using custom SPI bit-banging
combined with the https://elm-chan.org/ Petit FatFs library.

## Functionality

The code performs the following actions:
1. Mounts a FAT32 filesystem on the SD card.
2. Reads up to 512 bytes from the file `README.TXT`.
3. Writes 512 generated ASCII characters to the file `WRITEME.TXT`.

## Important Limitations

Petit FatFs does not support file creation, growing existing files, cluster
allocation, or updating file sizes.  **Therefore, before running the example,
`WRITEME.TXT` must already exist on the card and must be pre-allocated to a
minimum size of 512 bytes.**

## Warning 

Currently, using the `-Os` optimization level within the main `CMakeLists.txt`
appears to cause issues when mounting an SD card. **Therefore, please use
`-O1`, `-O2`, or `-O3` instead.** This is currently required as a functional
workaround to ensure successful compilation.

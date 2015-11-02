# WebMPlayer
Homemade WebM player for container debugging and inspection purpouses.

This repository contains the source code and Visual Studio 2013 solution for a command line WebM player that implements an EBML parser and uses SDL and libvpx-1.4.0 to play back the content from the WebM file. The project may and most likely does contain bugs as I developed it to such state where I could debug and inspect my own files. The functionality is quite similar to mkvinfo however this tool also acts as a player and tries to decompress and render the frames.

The project contains prebuilt 32bit binaries of libvpx-1.4.0 (built with Visual Studio 2013) and SDL 2.0.3 (downloaded from the SDL homepage). If anyone wants to make a Linux makefile and add Linux binaries for libvpx and SDL so that this works on Linux too I'd be more than happy to merge in the changes.

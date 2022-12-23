# NABU RetroNET chat client

This repository contains a client for DJsures' RetroNET chat for the
NABU PC.  It supports 80 characters per line if the NABU is equipped
with an F18A VDP, and has a slightly improved line editor.

It is written in C and uses polling for I/O, so some character loss is
to be expected.

To compile it, use the Homebrew toolchain distributed by DJsures.

To use it, put the 000001.nabu file in some folder on your local hard
drive and enable homebrew mode (File -> Settings -> Source) in the
network adapter software, pointing it to that folder.

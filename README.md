# NABU RetroNET chat client

This repository contains a client for DJsures' RetroNET chat for the
NABU PC.  It supports 80 characters per line if the NABU is equipped
with an F18A VDP, and has a slightly improved line editor.

It is written in C and uses polling for I/O, so some character loss is
to be expected.

To compile it, use the Homebrew toolchain distributed by DJsures.

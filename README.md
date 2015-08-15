# Soyogi
Floating-point Ogg/Vorbis decoder for embedded platforms

## Overview
Soyogi is designed to run on some variants of Renesas RX231 MCU,
with at least 64KB of RAM and 128KB of Code Flash.

Its salient features include:
* 50% space savings by calculating MDCT via DCT-IV
* Totally static memory use, in a sense that neither heap area nor dynamic stack is required
* Variable-length CAR (Compressed Array Representation) for huffman tree, which saves around 25% of codebook RAM
* Most of the Ogg/Vorbis file with blocksizes 256 and 2048 could be decoded with only 24KiB of codec RAM on average

## Inverse MDCT via DCT-IV
Instead of extracting 2N time-domain samples from the vector,
Soyogi performs DCT-IV on the frequency-domain audio,
and saves the N-sized output until "Overlap-Add" process.
TDAC is executed at the same time as two vectors are overlapped,
which could be regarded as additional butterfly.

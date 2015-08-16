# Soyogi
Floating-point Ogg/Vorbis decoder for embedded platforms

## Overview
Soyogi(そよぎ, "tremor" in Japanese) is designed to run on some variants of Renesas RX231 MCU,
with at least 64KB of RAM and 128KB of Code Flash.

Its salient features include:
* 50% space savings by calculating MDCT via DCT-IV (which is also available in Tremor)
* Totally static memory use, in a sense that neither heap area nor dynamic stack is required
* Variable-length CAR (Compressed Array Representation) for huffman tree, which saves around 25% of codebook RAM
* Most of the stereo Ogg/Vorbis files with blocksizes 256 and 2048 could be decoded with only 24KiB of codec RAM on average

## Inverse MDCT via DCT-IV
Instead of extracting 2N time-domain samples from the vector,
Soyogi performs DCT-IV on the frequency-domain audio,
and saves the N-sized output until "Overlap-Add" process.
TDAC is executed at the same time as two vectors are overlapped,
which could be regarded as additional butterfly.

## Fully static in terms of memory use
All of the setup components will be allocated on a dedicated stack,
which could be of any configurable capacity up to 64KiB.
36KiB would be enough to decode an stereo Ogg/Vorbis file with blocksizes N and 4096.
Note that output PCM buffer to store the decoder output frame by frame is still required,
and when the output is 100% buffered, 48KiB or so should be the average RAM consumption.

## Development status
As of August 2015, the implementation is almost complete, if not fully.
Several Vorbis files were successfully decoded to PCM samples,
but conspicuous noise (maybe due to bug around overlapping) is still to be addressed.
Software SRC(Sampling rate converter, from 44.1kHz to 48kHz) also needs to be written,
because the target MCU is not capable of.
There is currently no plan to support Floor 0 and Submaps.

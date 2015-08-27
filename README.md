# Soyogi
Floating-point/Fixed-point Ogg/Vorbis decoder for embedded platforms

## Overview
Soyogi(そよぎ, "tremor" in Japanese) is designed to run on some variants of Renesas RX231 MCU,
with at least 64KB of RAM and 128KB of Code Flash.

Its salient features include:
* 50% space savings by calculating MDCT via DCT-IV (which is also available in Tremor)
* Totally static memory use, in a sense that neither heap area nor dynamic stack is required
* Variable-length CAR (Compressed Array Representation) for Huffman tree, which saves around 25% of codebook RAM
* Most of the stereo Ogg/Vorbis files with blocksizes 256 and 2048 could be decoded with only 24KiB of codec RAM on average
* Fixed point implementation for environments without FPU is also available as a separate branch

## Inverse MDCT via DCT-IV
Instead of extracting 2N time-domain samples from the vector,
Soyogi performs DCT-IV on the frequency-domain audio,
and saves the N-sized output until "Overlap-Add" process.
TDAC is executed at the same time as two vectors are overlapped,
which could be regarded as additional butterfly.

## Fully static in terms of memory use
All of the setup components will be allocated on a dedicated stack,
which could be of any configurable capacity up to 64KiB.
36KiB would be enough to decode a stereo Ogg/Vorbis file with blocksizes N and 4096.
Note that a PCM buffer to store the decoder output frame by frame is still required,
and when the output is 100% buffered, 48KiB or so should be the average RAM consumption.

## Hardware failsafe assumed
Soyogi basically does not check for errors during packet decode.
Instead, the MCU hardware is expected to protect from illegal access
with MPU (Memory Protection Unit) functionality.
Single dedicated stack for codec setup makes it easier to confine
the location of memory errors.
Errors are accumulated and detected once at the end of packet.

## Fixed-point version
Fixed-point version of Soyogi would be preferable if your CPU supports 32x32=64 multiplication.

## Limitations
* The maximum number of (sparse) codebook entries is 32767.
  * Practical bound is at around 7000.
* The maximum number of codebooks is 255.
  * Practical bound is at 50.
* The maximum size of setup stack is 65536 bytes.
  * Practical bound for blocksizes N/4096 is at 49152.
* Blocksizes N/8192 is not supported.
* Floor 0 is not supported.
* Submaps are not supported.

## Software SRC capability
FIR-based sampling rate converter, from 44.1kHz to 48kHz,
is implemented to play both 11.025/22.05/44.1kHz and 12/24/48kHz audio
with only one I2S master clock source.

## Development status
As of August 2015, the implementation is almost complete, if not fully.
Several Vorbis files were successfully decoded to clear PCM samples.
There is currently no plan to support Floor 0 and Submaps.

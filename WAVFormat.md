# General #

Structs for the WAV format can be found in riff\_struct.hpp


# Details #

WAV files contain a series of "chunks." The standard chunks are "RIFF" "fmt " and "data".

## RIFF Chunk ##
| **Size** | **Name** | **Description** |
|:---------|:---------|:----------------|
| 4 | ChunkID | "RIFF" in ascii |
| 4 | ChunkSize | 4 + (8 + fmt.SubchunkSize) + (8 + data.SubchunkSize) or file size - 8 |
| 4 | Format | "WAVE" in ascii |


---


## FMT Chunk ##
| **Size** | **Name** | **Description** |
|:---------|:---------|:----------------|
| 4 | SubchunkID | "fmt " in ascii |
| 4 | SubchunkSize | Size of rest of chunk after this (16 for PCM) |
| 2 | AudioFormat | Type of Compression (1 for PCM, linear quantization) |
| 2 | NumChannels| Mono = 1, Stereo = 2, ... |
| 2 | SampleRate | 8000, 44100, ...|
| 4 | ByteRate | SampleRate x NumChannels x ( BitsPerSample / 8 ) |
| 4 | BlockAlign | NumChannels x ( BitsPerSample / 8 ) |
| 2 | BitsPerSample | 8, 16, 32, ... |

### (Optional) ###
| **Size** | **Name** | **Description** |
|:---------|:---------|:----------------|
| 2 | ExtraFormatBytes |  |
| 2 | ValidBitsPerSample |  |
| 4 | ChannelMask |  |
| 16 | SubFormat |  |


---


## DATA Chunk ##
| **Size** | **Name** | **Description** |
|:---------|:---------|:----------------|
| 4 | SubchunkID | "data" in ascii |
| 4 | SubchunkSize | NumSamples x NumChannels x ( BitsPerSample / 8 ) |
|  | DATA |  |


---


## FACT Chunk (Optional) ##
| **Size** | **Name** | **Description** |
|:---------|:---------|:----------------|
| 4 | SubchunkID | "fact" in ascii |
| 4 | SubchunkSize | 4 |
| 4 | SampleLength | Per channel |


---


## PPEAK Chunk (Optional) ##
| **Size** | **Name** | **Description** |
|:---------|:---------|:----------------|
| 4 | Value| Signed value of peak |
| 4 | Position| Sample frame for peak |


## PEAK Chunk (Optional) ##
| **Size** | **Name** | **Description** |
|:---------|:---------|:----------------|
| 4 | SubchunkID | "PEAK" in ascii |
| 4 | SubchunkSize | Size of rest of chunk after this |
| 4 | Version | Peak Chunk Version |
| 4 | timestamp | Unix timestamp of creation |
| Sizeof(Pointer) | peak | Pointer to the PPEAK structs (one for each channel) |
| Sizeof(Pointer) | bit\_align | Space for the 64-bit alignment variable |
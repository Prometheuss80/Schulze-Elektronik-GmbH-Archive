# Exact HEX changes — all four hardware-tested versions

All addresses below are **zero-based ROM offsets**, shown in hexadecimal. Original and generated patched ROMs are **65,536 bytes**. The listing includes the original and replacement **35-byte calibration region**, plus the five text fields changed elsewhere. No complete ROM images are included.

The table's byte-change count is the number of bytes **actually differing**, rather than the total size of the replaced region.

## 8.10 DE → fw819de.bin

Original SHA-256: `d593901c113f3454459d136f2f9ec0e3815f0ba41f3cebe64d5608503361fbcf`  
Output SHA-256: `e78253f9210a3cfc6ca4a653c62a509cd2098e8e91e7b5344b6cb43793d03004`  
Changed byte count: **41** (28 calibration + 1 LCD + 10 date + 2 time).

| Offset | Field | Before (HEX) | After (HEX) |
|---|---|---|---|
| `B9B5h` | Calibration entry (35 B) | `30 22 38 20 51 35 78 68 E6 60 30 08 E6 60 2C 08 E6 60 28 30 50 25 D2 51 C2 37 78 5B 12 3B E1 00 00 00 00` | `30 14 38 D2 50 D2 51 78 68 76 01 08 76 01 08 76 01 08 30 50 26 D2 51 C2 37 78 5B 12 3B E1 00 00 00 00 00` |
| `3131h` | LCD version field (5 B) | `38 2E 31 30 00` | `38 2E 31 39 00` |
| `3057h` | Date #1 (8 B) | `31 35 2E 30 36 2E 30 34` | `32 32 2E 30 34 2E 32 36` |
| `346Fh` | Date #2 (8 B) | `31 35 2E 30 36 2E 30 34` | `32 32 2E 30 34 2E 32 36` |
| `3061h` | Time #1 (8 B) | `4D 69 20 31 32 3A 33 30` | `4D 69 20 31 38 3A 33 30` |
| `34A3h` | Time #2 (8 B) | `4D 69 20 31 32 3A 33 30` | `4D 69 20 31 38 3A 33 30` |


## 8.10 EN → fw819en.bin

Original SHA-256: `e22af897780acc77fcc9a33a3bdcff9091f5fa8c9985e5953013885acb63e6d5`  
Output SHA-256: `368f70845ada171afeb62373df4f4f802c2ec07b3c8d59b20075c52636b5f469`  
Changed byte count: **41** (28 calibration + 1 LCD + 10 date + 2 time).

| Offset | Field | Before (HEX) | After (HEX) |
|---|---|---|---|
| `B9A2h` | Calibration entry (35 B) | `30 22 38 20 51 35 78 68 E6 60 30 08 E6 60 2C 08 E6 60 28 30 50 25 D2 51 C2 37 78 5B 12 3B DE 00 00 00 00` | `30 14 38 D2 50 D2 51 78 68 76 01 08 76 01 08 76 01 08 30 50 26 D2 51 C2 37 78 5B 12 3B DE 00 00 00 00 00` |
| `3131h` | LCD version field (5 B) | `38 2E 31 30 00` | `38 2E 31 39 00` |
| `3057h` | Date #1 (8 B) | `31 35 2E 30 36 2E 30 34` | `32 32 2E 30 34 2E 32 36` |
| `346Ch` | Date #2 (8 B) | `31 35 2E 30 36 2E 30 34` | `32 32 2E 30 34 2E 32 36` |
| `3061h` | Time #1 (8 B) | `77 65 20 31 32 3A 33 30` | `77 65 20 31 38 3A 33 30` |
| `34A0h` | Time #2 (8 B) | `77 65 20 31 32 3A 33 30` | `77 65 20 31 38 3A 33 30` |


## 8.50 DE → fw859de.bin

Original SHA-256: `469b4f3621f412266676f3fcbcf209a752addaa56f6ce75edc62419a36d401bb`  
Output SHA-256: `edff3aa477482d72a3243a56aa2f76e33aade6da8a77c6336b4bebcabfa5040f`  
Changed byte count: **41** (28 calibration + 1 LCD + 10 date + 2 time).

| Offset | Field | Before (HEX) | After (HEX) |
|---|---|---|---|
| `BABAh` | Calibration entry (35 B) | `30 22 38 20 51 35 78 68 E6 60 30 08 E6 60 2C 08 E6 60 28 30 50 25 D2 51 C2 37 78 5B 12 3B D8 00 00 00 00` | `30 14 38 D2 50 D2 51 78 68 76 01 08 76 01 08 76 01 08 30 50 26 D2 51 C2 37 78 5B 12 3B D8 00 00 00 00 00` |
| `312Dh` | LCD version field (5 B) | `38 2E 35 30 00` | `38 2E 35 39 00` |
| `3053h` | Date #1 (8 B) | `31 35 2E 30 36 2E 30 34` | `32 32 2E 30 34 2E 32 36` |
| `346Bh` | Date #2 (8 B) | `31 35 2E 30 36 2E 30 34` | `32 32 2E 30 34 2E 32 36` |
| `305Dh` | Time #1 (8 B) | `4D 69 20 31 32 3A 33 30` | `4D 69 20 31 38 3A 33 30` |
| `349Fh` | Time #2 (8 B) | `4D 69 20 31 32 3A 33 30` | `4D 69 20 31 38 3A 33 30` |


## 8.50 EN → fw859en.bin

Original SHA-256: `f9d461cb2541e4584cf9811e5b68850b6dd13b36725339e09f433699b96aa5ed`  
Output SHA-256: `2c7aff211fe2729707bdd1fb5bb768fad59dab864801c90bc3079895851cf034`  
Changed byte count: **41** (28 calibration + 1 LCD + 10 date + 2 time).

| Offset | Field | Before (HEX) | After (HEX) |
|---|---|---|---|
| `BAA7h` | Calibration entry (35 B) | `30 22 38 20 51 35 78 68 E6 60 30 08 E6 60 2C 08 E6 60 28 30 50 25 D2 51 C2 37 78 5B 12 3B D5 00 00 00 00` | `30 14 38 D2 50 D2 51 78 68 76 01 08 76 01 08 76 01 08 30 50 26 D2 51 C2 37 78 5B 12 3B D5 00 00 00 00 00` |
| `312Dh` | LCD version field (5 B) | `38 2E 35 30 00` | `38 2E 35 39 00` |
| `3053h` | Date #1 (8 B) | `31 35 2E 30 36 2E 30 34` | `32 32 2E 30 34 2E 32 36` |
| `3468h` | Date #2 (8 B) | `31 35 2E 30 36 2E 30 34` | `32 32 2E 30 34 2E 32 36` |
| `305Dh` | Time #1 (8 B) | `77 65 20 31 32 3A 33 30` | `77 65 20 31 38 3A 33 30` |
| `349Ch` | Time #2 (8 B) | `77 65 20 31 32 3A 33 30` | `77 65 20 31 38 3A 33 30` |


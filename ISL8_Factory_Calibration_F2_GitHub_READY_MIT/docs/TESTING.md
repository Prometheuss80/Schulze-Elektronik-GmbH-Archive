# Testing and verification

## Physical hardware tests

**PASS** — the project maintainer reports successful testing of the complete V1.2 patch (factory calibration access, updated LCD version, date and time) on **each** of the four supported firmware versions on a real Schulze ISL8 charger. These are maintainer-reported tests, not a laboratory safety certification.

| Original | F2 calibration and display | Date/time change | Status |
|---|---|---|---|
| 8.10 DE | PASS | PASS | HW TESTED |
| 8.10 EN | PASS | PASS | HW TESTED |
| 8.50 DE | PASS | PASS | HW TESTED |
| 8.50 EN | PASS | PASS | HW TESTED |

## Original reference ROM SHA-256

| Version | SHA-256 | Output filename | Output SHA-256 |
|---|---|---|---|
| 8.10 DE | `d593901c113f3454459d136f2f9ec0e3815f0ba41f3cebe64d5608503361fbcf` | `fw819de.bin` | `e78253f9210a3cfc6ca4a653c62a509cd2098e8e91e7b5344b6cb43793d03004` |
| 8.10 EN | `e22af897780acc77fcc9a33a3bdcff9091f5fa8c9985e5953013885acb63e6d5` | `fw819en.bin` | `368f70845ada171afeb62373df4f4f802c2ec07b3c8d59b20075c52636b5f469` |
| 8.50 DE | `469b4f3621f412266676f3fcbcf209a752addaa56f6ce75edc62419a36d401bb` | `fw859de.bin` | `edff3aa477482d72a3243a56aa2f76e33aade6da8a77c6336b4bebcabfa5040f` |
| 8.50 EN | `f9d461cb2541e4584cf9811e5b68850b6dd13b36725339e09f433699b96aa5ed` | `fw859en.bin` | `2c7aff211fe2729707bdd1fb5bb768fad59dab864801c90bc3079895851cf034` |

## Binary integrity checks

For each of the four reference images: exact hash recognition, unique 35-byte calibration pattern, expected original `LCALL` target, unique LCD version string, two exact date strings, two exact localized time strings, 65,536-byte output, and **41 changed bytes only** were verified. The remaining bytes are byte-for-byte unchanged. The patch script refuses unknown/modified originals.

To run only redistributable unit tests without proprietary binaries:

```sh
python -m unittest discover -v
```

The original ROMs were used only in local verification. **Neither original nor generated firmware images are included** in the release ZIP or the GitHub-ready directory.

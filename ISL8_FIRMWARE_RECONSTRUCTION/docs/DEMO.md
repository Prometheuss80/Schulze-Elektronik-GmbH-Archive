# Six-address synthetic demonstration

`fw850en.bin` SHA-256: `f6117f0593a9bbceb87baa3caf9cac5fff7d577e16c08d637998e7bb6fbedb05`

```text
0000: 02 FE 80  -> LJMP 0FE80h
FE80: 02 00 00  -> LJMP 0000h
```

All remaining ROM bytes are zero. The matching `LOG_ISL8_ALL.txt` is:

```text
0000*0@
0001*1@
0002*2@
FE80*3@
FE81*4@
FE82*5@
```

### Verified on this demo

- `002_TEST.py`: six valid input lines, zero malformed lines.
- `005_RESULT.py`: six observed ROM bytes (0.0092% coverage).
- `006_DATA.py`: no isolated addresses.
- `007_OUTPUT.py`: six input bytes extracted.
- `050_BLOCK_1B_PC.py`: two separate blocks.
- `051_TIME_TRACE.py`: six CODE addresses, zero DATA addresses.
- `052_TRACE.py`: generated trace.
- `101_RECONSTRUCTION_FINAL.py` through `108_PROCESS_FINAL.py`: generated and post-processed `Reconstruction.a51`; no code/data jump conflicts reported.

The generated sample is in `examples/Reconstruction.a51` and contains no original Schulze firmware. The Keil A51 build and `109_DIFF_FINAL.py` round-trip were not performed here. Always compare the resulting firmware bytes after assembling in Keil before considering reconstruction verified.

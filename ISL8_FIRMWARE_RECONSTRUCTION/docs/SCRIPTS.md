# Original tool inventory

The original script filenames and numbering are preserved as supplied. The workflow groups below follow the author’s notes; consult individual scripts for exact inputs and outputs.

| Group | Purpose |
| --- | --- |
| `001` | Address acquisition and fast binary capture / conversion |
| `002` | Validate log sequence and syntax |
| `003–004` | Split and join large logs (public release fixes index width) |
| `005–007` | Address map and initial output |
| `008–010` | Optional deduplication, checks and ASCII search |
| `050–052` | Timing trace, blocks and CODE/DATA recognition |
| `101–108` | Main assembly reconstruction and validation |
| `109–110` | Keil HEX comparison and visual coverage report |
| `111+` | Specialized auxiliary scripts |

## Included Python tools

- `001_LOGGER.py`
- `001_V2_CONVERT_BIN_TXT.py`
- `001_V2_LOGGER_FAST_BIN.py`
- `002_TEST.py`
- `003_SPLIT.py`
- `004_JOIN.py`
- `005_RESULT.py`
- `006_DATA.py`
- `007_OUTPUT.py`
- `008_DEDUP.py`
- `009_CHECK.py`
- `010_SEARCH.py`
- `050_BLOCK_1B_PC.py`
- `051_TIME_TRACE.py`
- `052_TRACE.py`
- `101_RECONSTRUCTION_FINAL.py`
- `102_FILL_FINAL.py`
- `103_RENAME_FINAL.py`
- `104_LABEL.py`
- `105_JUMPS.py`
- `106_STRIPPED.py`
- `107_CHECK_FINAL.py`
- `108_PROCESS_FINAL.py`
- `109_DIFF_FINAL.py`
- `110_SERVER_FINAL.py`
- `111_FILL_DATA.py`
- `120_DPTR_SEARCH.py`
- `130_DB_DATA.py`
- `135_DB_TABLE.py`
- `140_ASCII_DECODE.py`
- `145_ROM_TABLE_SEARCH.py`
- `150_CLEAN_ASM.py`
- `160_COVERAGE_ASM.py`
- `170_DIFF_DATA.py`
- `171_DIFF_CODE.py`
- `172_REMOVE_DIFF_CODE_ADDRS.py`
- `200_RUN_ORG_CALIB.py`
- `201_RUN_ORG_CALIB.py`
- `RS232_CALIB.py`

The Arduino sketch `ISL8_EMUL_RPI5_FAST.ino` is included for hardware-side research. It has not been built in this environment.

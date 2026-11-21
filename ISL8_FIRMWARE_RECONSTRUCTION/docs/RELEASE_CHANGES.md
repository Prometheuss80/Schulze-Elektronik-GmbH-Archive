# Minimal compatibility fixes in the GitHub release

Three original scripts are archived unchanged under `docs/original_scripts/`:

- `003_SPLIT.py`: part filename formatting changed from `:02d` to `:03d` to match `004_JOIN.py`.
- `005_RESULT.py`: event counter regex changed from `\d@` to `\d+@` to accept multi-digit counters.
- `006_DATA.py`: event counter regex changed from `\d@` to `\d+@` for the same reason.

These changes do not modify firmware bytes and preserve the original script filenames. All other source tools were copied byte-for-byte from the user-provided RAR. The synthetic six-event demo passes before and after these changes.

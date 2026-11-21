# Copyright (c) 2026 Martin Stránský (Prometheuss80)
# SPDX-License-Identifier: MIT
"""Unit tests that work without any proprietary Schulze firmware ROM."""
import unittest
from patch_factory_calibration import (KNOWN_FIRMWARE, ORIGINAL_PREFIX,
    ORIGINAL_SUFFIX, PATCH_PREFIX, PATCH_SUFFIX, PATCH_SIZE, ROM_SIZE,
    patch_rom_with_report)

class TestPatcherStructure(unittest.TestCase):
    def test_four_original_profiles(self):
        self.assertEqual(len(KNOWN_FIRMWARE), 4)
        self.assertEqual(sorted(f.output_filename for f in KNOWN_FIRMWARE.values()),
                         ['fw819de.bin','fw819en.bin','fw859de.bin','fw859en.bin'])

    def test_replacement_length(self):
        self.assertEqual(len(ORIGINAL_PREFIX)+2+len(ORIGINAL_SUFFIX), PATCH_SIZE)
        self.assertEqual(len(PATCH_PREFIX)+2+len(PATCH_SUFFIX), PATCH_SIZE)
        self.assertEqual(PATCH_SIZE, 35)

    def test_unknown_sha_rejected(self):
        with self.assertRaisesRegex(ValueError, 'unknown SHA-256'):
            patch_rom_with_report(bytes(ROM_SIZE))

    def test_bad_rom_length_rejected(self):
        with self.assertRaises(ValueError):
            patch_rom_with_report(bytes(ROM_SIZE-1))

    def test_version_and_date_profiles(self):
        for fw in KNOWN_FIRMWARE.values():
            self.assertEqual(len(fw.date_offsets), 2)
            self.assertEqual(len(fw.time_offsets), 2)
            self.assertEqual(len(fw.display_before),len(fw.display_after))
            self.assertTrue(fw.output_filename.endswith(fw.language.lower()+'.bin'))

if __name__ == '__main__':
    unittest.main()

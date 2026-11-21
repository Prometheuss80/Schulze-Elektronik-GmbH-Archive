#!/usr/bin/env python3
import re

# Vstupní i výstupní soubor jsou stejné:
INPUT_A51 = "Reconstruction.a51"

REG_MAP_BYTE = "name_reg_data.txt"
REG_MAP_BIT  = "name_reg_bit.txt"

# -------------------------------------------------
# 1) Načtení map registrů
# -------------------------------------------------
def load_reg_map(path):
    m = {}
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            parts = line.split()
            if len(parts) < 2:
                continue

            addr_str, name = parts[0], parts[1]

            if addr_str.lower().startswith("0x"):
                addr = int(addr_str, 16)
            else:
                addr = int(addr_str, 16)

            m[addr] = name

    return m


reg_map_byte = load_reg_map(REG_MAP_BYTE)
reg_map_bit  = load_reg_map(REG_MAP_BIT)

# -------------------------------------------------
# 2) Klasifikace instrukcí
# -------------------------------------------------
BIT_JUMP   = {"JB", "JNB", "JBC"}
BIT_SINGLE = {"SETB", "CLR", "CPL"}   # CLR A/C je výjimka
BIT_ACC    = {"ANL", "ORL"}           # ANL C,bit / ORL C,bit
BIT_MOV    = {"MOV"}                  # MOV C,bit / MOV bit,C

# Syntaxe typu:
# A_1234:     MOV  0xD5,#0x00
line_re = re.compile(r'^(\s*\w+:\s+)(\w+)(\s+)(.*)$')

# Zachytí "0xXX" které nejsou #0xXX nebo součást čísla
addr_re = re.compile(r'(?<!#)(?<![0-9A-Fa-f])0x([0-9A-Fa-f]{2})(?=[^\w]|$)')


def is_bit_context(mnemo: str, operands: str) -> bool:
    mn = mnemo.upper()

    if mn in BIT_JUMP:
        return True

    if mn in BIT_SINGLE:
        op = operands.strip().split(",")[0].strip().upper()
        if op in ("A", "C"):
            return False
        return True

    if mn in BIT_ACC:
        parts = [x.strip().upper() for x in operands.split(",")]
        if len(parts) >= 2 and parts[0] == "C":
            return True

    if mn in BIT_MOV:
        parts = [x.strip().upper() for x in operands.split(",")]
        if len(parts) == 2 and ("C" in parts):
            return True

    return False


# -------------------------------------------------
# 3) Přejmenování v jednom řádku
# -------------------------------------------------
def replace_in_line(line: str) -> str:
    m = line_re.match(line)
    if not m:
        return line

    prefix, mnemo, ws, rest = m.groups()
    mn = mnemo.upper()

    # --- DB se nikdy nepřejmenovává ---
    if mn == "DB":
        return line

    # Odstraníme komentář z operandů pro analýzu
    code_part = rest.split("//", 1)[0]
    ops_clean = [x.strip().upper() for x in code_part.split(",") if x.strip()]

    # ==========================================================
    # SPECIÁLNÍ OPRAVA:
    # MOV ?,C (opcode 0x92 = MOV bit,C) → NESAHAT NA TENTO ŘÁDEK
    # necháme přesně tak, jak je (číslo 0x??, žádné přejmenování)
    # ==========================================================
    if mn == "MOV" and len(ops_clean) == 2 and ops_clean[1] == "C":
        return line

    # Ostatní instrukce: běžná detekce bit/byte kontextu
    bit_ctx = is_bit_context(mnemo, code_part)

    def repl_addr(ma):
        addr_hex = ma.group(1)
        addr_int = int(addr_hex, 16)

        if bit_ctx:
            sym = reg_map_bit.get(addr_int)
        else:
            sym = reg_map_byte.get(addr_int)

        return sym if sym else ma.group(0)

    new_rest = addr_re.sub(repl_addr, rest)

    end = ""
    if line.endswith("\n"):
        end = "\n"

    return prefix + mnemo + ws + new_rest + end


# -------------------------------------------------
# 4) Přepsání originálního souboru
# -------------------------------------------------
total_lines = 0
changed = 0

with open(INPUT_A51, "r", encoding="utf-8", errors="ignore") as fin:
    lines = fin.readlines()

new_lines = []
for line in lines:
    total_lines += 1
    new_line = replace_in_line(line)
    if new_line != line:
        changed += 1
    new_lines.append(new_line)

# Přepsání originálu
with open(INPUT_A51, "w", encoding="utf-8") as fout:
    fout.writelines(new_lines)

print(f"Hotovo. Zpracováno řádků: {total_lines}, upraveno: {changed}")
print(f"Výstup uložen zpět do: {INPUT_A51}")

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Reconstruction v8.4
- USER_CODE override (force decode as instruction)
- fixes false DB / placeholder at code addresses
- logic identical to v8.3 otherwise
"""

from pathlib import Path
import re
import json
from typing import Dict, Tuple, List, Optional, Set

# -------- configuration --------
TABLE_FILE            = Path("C500.txt")
FIRMWARE_FILE         = Path("fw850en.bin")
TRACE_FILE            = Path("TRACE_ISL8.txt")
HEADER_FILE           = Path("header.txt")
OUTPUT_FILE           = Path("Reconstruction.a51")

TIME_CODE_FILE        = Path("TIME_CODE_ADDRS.txt")   # 100% CODE z time-trace
TIME_DATA_FILE        = Path("TIME_DATA_ADDRS.txt")   # 100% DATA z time-trace

FINAL_USER_CODE_FILE  = Path("FINAL_USER_CODE.txt")   # ručně potvrzený CODE
FINAL_USER_DATA_FILE  = Path("FINAL_USER_DATA.txt")   # ručně potvrzená DATA


SUSPECT_FILE          = Path("INFO_suspect_addrs_FINAL.txt")
DATA_TRACE_FILE       = Path("INFO_data_trace_FINAL.txt")
DEBUG_JSON            = Path("INFO_reconstruction_indexed_FINAL.json")

FIRMWARE_SIZE         = 0x10000

AUTO_DATA_BLOCK_LEN   = 32
MAX_ENTRIES_TO_PROCESS = None  # None = entire trace

# formatting columns
LABEL_COL = 0
INSTR_COL = 15
COMMENT_COL = 48

# regex
RANGE_RE  = re.compile(r'0x([0-9A-Fa-f]{1,4})\s*-\s*0x([0-9A-Fa-f]{1,4})')
SINGLE_RE = re.compile(r'0x([0-9A-Fa-f]{1,4})\b')


# -------------------------------------------------------------
# Helper functions
# -------------------------------------------------------------
def load_table(path: Path):
    t = {}
    for ln in path.read_text(encoding="utf-8").splitlines():
        s = ln.strip()
        if not s or "*" not in s or "/" not in s:
            continue
        try:
            code, rest = s.split("*",1)
            instr, length = rest.split("/",1)
            opcode = int(code.strip().replace("H",""), 16)
            t[opcode] = (instr.strip(), int(length.strip()))
        except:
            continue
    return t


def load_firmware(path: Path) -> bytearray:
    b = path.read_bytes()
    if len(b) < FIRMWARE_SIZE:
        b += bytes([0]) * (FIRMWARE_SIZE - len(b))
    return bytearray(b[:FIRMWARE_SIZE])


def parse_trace_line(s: str):
    s = s.strip()
    if not s:
        return None
    if s.lower() == "trace":
        return "trace"
    m = re.match(r'^([0-9A-Fa-f]{4})', s)
    if m:
        return int(m.group(1), 16)
    try:
        return int(s,16)
    except:
        return None


def load_trace(path: Path, max_entries=None):
    out=[]
    for ln in path.read_text(encoding="utf-8").splitlines():
        v = parse_trace_line(ln)
        if v is None:
            continue
        out.append(v)
        if max_entries and len(out) >= max_entries:
            break
    return out


def load_clean_file(path: Path) -> Set[int]:
    out=set()
    if not path.exists():
        return out
    for ln in path.read_text(encoding="utf-8").splitlines():
        s=ln.strip()
        if not s or s.startswith(";"):
            continue
        m=RANGE_RE.search(s)
        if m:
            a1=int(m.group(1),16); a2=int(m.group(2),16)
            for a in range(min(a1,a2), max(a1,a2)+1):
                out.add(a)
            continue
        m2=SINGLE_RE.search(s)
        if m2:
            out.add(int(m2.group(1),16))
    return out

def load_simple_addr_file(path: Path) -> Set[int]:
    """
    Načte soubor s adresami ve formátu:
    0001
    00AF
    1F2C
    """
    out = set()
    if not path.exists():
        return out

    for ln in path.read_text(encoding="utf-8").splitlines():
        s = ln.strip()
        if not s or s.startswith(";"):
            continue
        try:
            out.add(int(s[:4], 16))
        except ValueError:
            pass

    return out


def ascii_for_bytes(bb: bytes, length: int) -> str:
    if length <= 3:
        arr = list(bb)[:length]
    else:
        arr = list(bb)[:8]
    return '"' + ''.join(chr(b) if 32<=b<127 else '.' for b in arr) + '"'


def ascii_preview(raw: bytes, length: int):
    return ascii_for_bytes(raw, length)


def label_for(a:int)->str:
    return f"A_{a:04X}"


# -------------------------------------------------------------
# MAIN
# -------------------------------------------------------------
def main():

    table = load_table(TABLE_FILE)
    fw = load_firmware(FIRMWARE_FILE)
    trace = load_trace(TRACE_FILE, MAX_ENTRIES_TO_PROCESS)
    header = HEADER_FILE.read_text(encoding="utf-8").splitlines() if HEADER_FILE.exists() else ["; header missing"]

    # nové zdroje
    time_code = load_simple_addr_file(Path("TIME_CODE_ADDRS.txt"))
    time_data = load_simple_addr_file(Path("TIME_DATA_ADDRS.txt"))

    final_user_code = load_clean_file(Path("FINAL_USER_CODE.txt"))
    final_user_data = load_clean_file(Path("FINAL_USER_DATA.txt"))

    # přesně stejné významy jako dřív
    confirmed_code = set(time_code) | set(final_user_code)
    confirmed_data = set(time_data) | set(final_user_data)

    print(
        f"[i] TIME code: {len(time_code)} | "
        f"TIME data: {len(time_data)} | "
        f"USER code: {len(final_user_code)} | "
        f"USER data: {len(final_user_data)}"
    )


    addr_map: Dict[int, dict] = {}
    emitted_ranges: List[Tuple[int,int]] = []
    discovered_auto_ranges = []
    known_targets=set()

    last_dptr: Optional[int] = None

    def in_emitted_ranges(a:int)->bool:
        for s,e in emitted_ranges:
            if s <= a <= e:
                return True
        return False

    # ============================================================
    # PASS: parse trace
    # ============================================================
    trace_addrs = [a for a in trace if isinstance(a, int)]

    all_entry_addrs = sorted(
        set(trace_addrs)
        | confirmed_code
        | confirmed_data
    )

#   print(f"[i] Total addresses for reconstruction: {len(all_entry_addrs)}")

    for idx, addr in enumerate(all_entry_addrs):

        
            # === AUTORITATIVNÍ FILTR ADRES ===
        if addr not in confirmed_code and addr not in confirmed_data:
            continue
        
        if in_emitted_ranges(addr):
            continue
        if addr in addr_map:
            continue

        b0 = fw[addr]

        # --- USER FORCED CODE ---
        if addr in confirmed_code:
            # zachovej instrukční dekódování
            pass

        # --- USER DATA (only if NOT forced code) ---
        if addr in final_user_data and addr not in confirmed_code:
            raw = bytes([b0])
            addr_map[addr] = {
                "text": f"DB 0x{b0:02X}\t; USER DATA",
                "raw": raw,
                "length": 1,
                "kind":"user_data",
                "first_idx": idx
            }
            emitted_ranges.append((addr,addr))
            continue


        # --- AUTO EXISTING DATA ---
        if addr in confirmed_data and addr not in confirmed_code:

            raw = bytes([b0])
            addr_map[addr] = {
                "text": f"DB 0x{b0:02X}\t; AUTO DATA (existing)",
                "raw": raw,
                "length": 1,
                "kind":"auto_data",
                "first_idx": idx
            }
            emitted_ranges.append((addr,addr))
            continue
        

        opcode = b0

        # --------------------------------------------------------
        # CJNE @R0, CJNE @R1, CJNE Rn fixes
        # --------------------------------------------------------
        if opcode == 0xB6:  # CJNE @R0
            if addr+2 < len(fw):
                imm = fw[addr+1]
                rel8 = fw[addr+2]
                rel = rel8 if rel8<0x80 else rel8-0x100
                tgt = (addr+3+rel)&0xFFFF
                text=f"CJNE @R0,#0x{imm:02X},A_{tgt:04X}"
                raw=bytes(fw[addr:addr+3])
                addr_map[addr]={
                    "text":text,"raw":raw,"length":3,"kind":"instr","first_idx":idx
                }
                emitted_ranges.append((addr,addr+2))
                known_targets.add(tgt)
                continue
            else:
                addr_map[addr]={
                    "text":f"DB 0x{opcode:02X}\t; TRUNCATED CJNE","raw":bytes([opcode]),
                    "length":1,"kind":"db","first_idx":idx
                }
                emitted_ranges.append((addr,addr))
                continue

        if opcode == 0xB7:  # CJNE @R1
            if addr+2 < len(fw):
                imm = fw[addr+1]
                rel8 = fw[addr+2]
                rel = rel8 if rel8<0x80 else rel8-0x100
                tgt=(addr+3+rel)&0xFFFF
                text=f"CJNE @R1,#0x{imm:02X},A_{tgt:04X}"
                raw=bytes(fw[addr:addr+3])
                addr_map[addr]={
                    "text":text,"raw":raw,"length":3,"kind":"instr","first_idx":idx
                }
                emitted_ranges.append((addr,addr+2))
                known_targets.add(tgt)
                continue
            else:
                addr_map[addr]={
                    "text":f"DB 0x{opcode:02X}\t; TRUNCATED CJNE","raw":bytes([opcode]),
                    "length":1,"kind":"db","first_idx":idx
                }
                emitted_ranges.append((addr,addr))
                continue

        if 0xB8 <= opcode <= 0xBF:  # CJNE Rn
            if addr+2<len(fw):
                imm=fw[addr+1]
                rel8=fw[addr+2]
                rel=rel8 if rel8<0x80 else rel8-0x100
                tgt=(addr+3+rel)&0xFFFF
                rn=opcode-0xB8
                text=f"CJNE R{rn},#0x{imm:02X},A_{tgt:04X}"
                raw=bytes(fw[addr:addr+3])
                addr_map[addr]={
                    "text":text,"raw":raw,"length":3,"kind":"instr","first_idx":idx
                }
                emitted_ranges.append((addr,addr+2))
                known_targets.add(tgt)
                continue
            else:
                addr_map[addr]={
                    "text":f"DB 0x{opcode:02X}\t; TRUNCATED CJNE","raw":bytes([opcode]),
                    "length":1,"kind":"db","first_idx":idx
                }
                emitted_ranges.append((addr,addr))
                continue

        # --------------------------------------------------------
        # MOV direct,direct fix (opcode 0x85)
        # --------------------------------------------------------
        if opcode==0x85:
            if addr+2 < len(fw):
                dd = fw[addr+2]
                ss = fw[addr+1]
                text = f"MOV 0x{dd:02X},0x{ss:02X}"
                raw = bytes(fw[addr:addr+3])
                addr_map[addr]={
                    "text":text,"raw":raw,"length":3,"kind":"instr","first_idx":idx
                }
                emitted_ranges.append((addr,addr+2))
                continue
            else:
                addr_map[addr]={
                    "text":f"DB 0x{opcode:02X}\t; TRUNCATED MOV","raw":bytes([opcode]),
                    "length":1,"kind":"db","first_idx":idx
                }
                emitted_ranges.append((addr,addr))
                continue

        # --------------------------------------------------------
        # Default table handling
        # --------------------------------------------------------
        if opcode not in table:
            addr_map[addr]={
                "text":f"DB 0x{opcode:02X}\t; UNKNOWN OPCODE",
                "raw":bytes([opcode]),
                "length":1,
                "kind":"db",
                "first_idx":idx
            }
            emitted_ranges.append((addr,addr))
            continue

        instr_template, length = table[opcode]
        raw = bytes(fw[addr:addr+length])
        ops = list(raw[1:]) if len(raw)>1 else []
        text = instr_template

        # CJNE A
        if opcode==0xB4 and len(ops)>=2:
            imm=ops[0]; rel8=ops[1]
            rel=rel8 if rel8<0x80 else rel8-0x100
            tgt=(addr+length+rel)&0xFFFF
            text=f"CJNE A,#0x{imm:02X},A_{tgt:04X}"

        # MOV DPTR,#imm16
        if opcode==0x90 and addr+2<len(fw):
            hi=fw[addr+1]; lo=fw[addr+2]
            last_dptr=(hi<<8)|lo
            text=f"MOV DPTR,#0x{hi:02X}{lo:02X}"

        elif opcode==0x75 and len(ops)>=2:
            text=f"MOV 0x{ops[0]:02X},#0x{ops[1]:02X}"

        elif "#data16" in text and len(ops)>=2:
            val=(ops[1]<<8)|ops[0]
            text=text.replace("#data16", f"#0x{val:04X}")

        elif "#data" in text and ops:
            text=text.replace("#data", f"#0x{ops[-1]:02X}")

        # address replacements
        if "direct" in text and ops:
            text=text.replace("direct", f"0x{ops[0]:02X}")

        if "addr11" in text and ops:
            addr11=((addr+2)&0xF800)|((opcode&0xE0)<<3)|ops[0]
            text=text.replace("addr11", f"A_{addr11:04X}")

        if "addr16" in text and len(ops)>=2:
            a16=(ops[0]<<8)|ops[1]
            text=text.replace("addr16", f"A_{a16:04X}")

        if "rel" in text and ops:
            rel8=ops[-1]
            rel=rel8 if rel8<0x80 else rel8-0x100
            tgt=(addr+length+rel)&0xFFFF
            text=re.sub(r"bit[., ]*rel", f"bit, A_{tgt:04X}", text)
            if not re.search(r"bit[, ]*A_", text):
                text=re.sub(r"\brel\b", f"A_{tgt:04X}", text)

        # --------------------------------------------------------
        # *** FIX: ANL/ORL C,/bit ***
        # --------------------------------------------------------
        if opcode == 0xA0 and ops:   # ORL C,/bit
            text = f"ORL C,/0x{ops[0]:02X}"

        elif opcode == 0xB0 and ops: # ANL C,/bit
            text = f"ANL C,/0x{ops[0]:02X}"

        # Normal bit replace (only if not /bit)
        elif "bit" in text and ops:
            text=text.replace("bit", f"0x{ops[0]:02X}")

        # MOVC detection (auto data)
        if text.upper().startswith("MOVC"):
            if last_dptr is not None:
                s=last_dptr
                e=min(s+AUTO_DATA_BLOCK_LEN-1, FIRMWARE_SIZE-1)
                discovered_auto_ranges.append((s,e))
                for a in range(s,e+1):
                    confirmed_data.add(a)
                   # auto_data_existing.add(a)

        # Record jump target labels
        for m in re.finditer(r"\bA_([0-9A-Fa-f]{4})\b", text):
            try:
                known_targets.add(int(m.group(1),16))
            except:
                pass

        addr_map[addr]={
            "text":text,"raw":raw,"length":length,"kind":"instr","first_idx":idx
        }
        emitted_ranges.append((addr,addr+length-1))

    # ============================================================
    # Build output .a51 file
    # ============================================================
    addrs_sorted = sorted(addr_map.keys())
    out_lines=[]
    defined_labels=set()

    i=0
    while i<len(addrs_sorted):
        start=addrs_sorted[i]
        block=[]
        j=i
        while j<len(addrs_sorted):
            a=addrs_sorted[j]
            if j==i:
                block.append(a); j+=1; continue
            prev=addrs_sorted[j-1]
            if a == prev + addr_map[prev]["length"]:
                block.append(a); j+=1; continue
            break

        out_lines.append(f"org  0x{start:04X}")
        for a in block:
            rec=addr_map[a]
            lbl = label_for(a)
            raw=rec["raw"]
            length=rec["length"]
            asc=ascii_preview(raw,length)
            if lbl not in defined_labels:
                left=f"{lbl}:"
                defined_labels.add(lbl)
            else:
                left=" "*(len(label_for(0))+1)
            line=left
            if len(line)<INSTR_COL:
                line=line+" "*(INSTR_COL-len(line))
            else:
                line += " "
            line += rec["text"]
            if len(line)<COMMENT_COL:
                line=line+" "*(COMMENT_COL-len(line))
            else:
                line+=" "
            line += f"// {asc}"
            out_lines.append(line)

        i=j

    # Missing referenced labels
    used=set()
    for ln in out_lines:
        for m in re.finditer(r"\bA_([0-9A-Fa-f]{4})\b", ln):
            used.add(m.group(0).upper())

    missing = sorted(
        lbl for lbl in used
        if lbl not in defined_labels
        and int(lbl.split("_")[1], 16) not in confirmed_code
    )

   
    if missing:
        out_lines.append("")
        out_lines.append("; ==== DOPLNĚNÉ PLACEHOLDERY ====")
        for lbl in missing:
            addr_ph=int(lbl.split("_")[1],16)
            orig_byte=fw[addr_ph]
            asc=ascii_preview(bytes([orig_byte]),1)
            out_lines.append(f"org  0x{addr_ph:04X}")
            out_lines.append(f"{lbl}:     DB 0x{orig_byte:02X}    // {asc} ; placeholder – nebyl v trace")
        out_lines.append("; ==================================")

    OUTPUT_FILE.write_text("\n".join(header+[""]+out_lines+["","end"]), encoding="utf-8")
    print(f"[+] WROTE {OUTPUT_FILE} ({len(out_lines)} lines)")

    # Suspects
    suspects = sorted(a for a,rec in addr_map.items() if rec["kind"] in ("db","unknown"))
    if suspects:
        with SUSPECT_FILE.open("w",encoding="utf-8") as f:
            for a in suspects:
                f.write(f"0x{a:04X}\n")
        print(f"[i] Suspects written: {SUSPECT_FILE}")

    # Data trace
    all_data = sorted(confirmed_data)
    if all_data:
        with DATA_TRACE_FILE.open("w",encoding="utf-8") as f:
            for a in all_data:
                f.write(f"0x{a:04X}: 0x{fw[a]:02X}   {ascii_preview(bytes([fw[a]]),1)}\n")
        print(f"[i] Data trace written: {DATA_TRACE_FILE}")

   
    # Debug JSON
    debug={
        f"{a:04X}": {
            "text":rec["text"],
            "raw":list(rec["raw"]),
            "length":rec["length"],
            "first_idx":rec["first_idx"]
        } for a,rec in addr_map.items()
    }
    DEBUG_JSON.write_text(json.dumps(debug,indent=2),encoding="utf-8")
    print(f"[i] Debug JSON saved.")

# -------------------------------------------------------------
if __name__=="__main__":
    main()



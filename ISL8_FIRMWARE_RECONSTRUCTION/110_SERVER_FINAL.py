#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import http.server
import socketserver
import json
import re
from pathlib import Path

# --------------------------------------------------------
# INPUT FILES
# --------------------------------------------------------

FW_FILE = Path("fw850en.bin")

TIME_CODE_FILE = Path("TIME_CODE_ADDRS.txt")
TIME_DATA_FILE = Path("TIME_DATA_ADDRS.txt")

USER_DATA_FILE = Path("FINAL_USER_DATA.txt")
USER_CODE_FILE = Path("FINAL_USER_CODE.txt")

STATIC_DIR = Path("static")
MAP_FILE   = STATIC_DIR / "map.json"

ROM_SIZE = 0x10000  # 64 kB

# --------------------------------------------------------
# LOAD HELPERS
# --------------------------------------------------------

def load_fw():
    if not FW_FILE.exists():
        print("[ERR] fw850en.bin not found!")
        return [0] * ROM_SIZE

    data = FW_FILE.read_bytes()
    if len(data) < ROM_SIZE:
        data += bytes([0]) * (ROM_SIZE - len(data))
    return list(data[:ROM_SIZE])


def load_simple_addr_file(path: Path):
    """Načte soubor typu:
       0x000E
       0x000F
    """
    out = set()
    if not path.exists():
        return out

    for ln in path.read_text().splitlines():
        ln = ln.strip()
        if not ln:
            continue
        try:
            out.add(int(ln.replace("0x", "")[:4], 16))
        except:
            pass
    return out


RANGE_RE = re.compile(r'0x([0-9A-Fa-f]{4})-0x([0-9A-Fa-f]{4})')

def load_range_file(path: Path):
    """Načte rozsahy:
       0x08E6-0x08EA/5b
    a rozbalí je na jednotlivé adresy
    """
    out = set()
    if not path.exists():
        return out

    for ln in path.read_text().splitlines():
        ln = ln.strip()
        if not ln:
            continue

        m = RANGE_RE.search(ln)
        if not m:
            continue

        start = int(m.group(1), 16)
        end   = int(m.group(2), 16)

        for a in range(start, end + 1):
            out.add(a)

    return out

# --------------------------------------------------------
# BUILD MAP.JSON
# --------------------------------------------------------

def build_map_json():
    print("[i] Generating static/map.json ...")

    STATIC_DIR.mkdir(exist_ok=True)

    fw = load_fw()

    time_code = load_simple_addr_file(TIME_CODE_FILE)
    time_data = load_simple_addr_file(TIME_DATA_FILE)

#   user_code = load_simple_addr_file(USER_CODE_FILE)
    user_code = load_range_file(USER_CODE_FILE)
    user_data = load_simple_addr_file(USER_DATA_FILE)
#   user_data = load_range_file(USER_DATA_FILE)

    print(f"[i] TIME CODE: {len(time_code)}")
    print(f"[i] TIME DATA: {len(time_data)}")
    print(f"[i] USER CODE: {len(user_code)}")
    print(f"[i] USER DATA: {len(user_data)}")

    pixels = []

    for addr in range(ROM_SIZE):
        byte = fw[addr]

        # PRIORITA (shora dolů!)
        if addr in user_code:
            kind = "UC"   # User confirmed CODE
        elif addr in user_data:
            kind = "UD"   # User confirmed DATA
        elif addr in time_code:
            kind = "C"    # Time-trace CODE
        elif addr in time_data:
            kind = "D"    # Time-trace DATA
        else:
            kind = "?"

        pixels.append({
            "addr": addr,
            "byte": byte,
            "kind": kind
        })

    MAP_FILE.write_text(
        json.dumps({"pixels": pixels}),
        encoding="utf-8"
    )

    print("[OK] map.json generated.")

# --------------------------------------------------------
# HTTP SERVER (favicon FIX)
# --------------------------------------------------------

class FixedHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/favicon.ico":
            self.send_response(204)
            self.end_headers()
            return
        super().do_GET()


def run_server():
    PORT = 8000

    print("[i] Web server running:")
    print(f"    http://localhost:{PORT}/static/index.html")
    print("[i] Press Ctrl+C to stop.")

    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), FixedHandler) as httpd:
        httpd.serve_forever()

# --------------------------------------------------------
# MAIN
# --------------------------------------------------------

if __name__ == "__main__":
    build_map_json()
    run_server()

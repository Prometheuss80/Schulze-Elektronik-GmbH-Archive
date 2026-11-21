import re

input_file = "Reconstruction.a51"
output_file = "Reconstruction_clean.a51"

with open(input_file, "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

out = []

for line in lines:
    # odstranění komentářů //
    line = re.sub(r'//.*', '', line)

    # odstranění trailing whitespace
    line = line.rstrip()

    if not line:
        out.append("")
        continue

    out.append(line)

    # pokud je instrukce RET
    if re.search(r'\bRET\b', line):
        out.append("")  # přidá prázdný řádek

with open(output_file, "w", encoding="utf-8") as f:
    f.write("\n".join(out))

print("Done. Output:", output_file)
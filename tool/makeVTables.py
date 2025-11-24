#!/usr/bin/env python3

import json
import re
import sys
from pathlib import Path

VERSION_SUFFIXES = {
    "1_0": "src/symbols1_0.h",
    "1_1": "src/symbols1_1.h",
    "1_2": "src/symbols1_2.h",
}

VTABLE_JSON = "src/diskCode/vtabledef.json"
OUTPUT_HEADER = "src/diskCode/vtables.h"
OUTPUT_SOURCE = "src/filesystem/vtables.c"
OUTPUT_SC = "src/diskCode/static_vtables.c"

INCLUDES = [
    '#include "../include/n64dd.h"',
    '#include "../include/save.h"',
    '#include "../include/message.h"',
    '#include "../include/gfx.h"',
    '#include "../include/game.h"',
    '#include "../include/play_state.h"',
    '#include "../include/player.h"',
    '#include "../include/controller.h"',
    '#include "../include/sfx.h"',
    '#include "../include/fault.h"',
]

define_re = re.compile(r'^\s*#define\s+(\S+)\s+(0x[0-9A-Fa-f]+|\d+)\b')

def load_symbols(header_path):
    out = {}
    p = Path(header_path)
    if not p.exists():
        print(f"Error: header '{header_path}' missing", file=sys.stderr)
        return out
    for line in p.read_text().splitlines():
        m = define_re.match(line)
        if m:
            out[m.group(1)] = m.group(2)
    return out

version_symbols = {ver: load_symbols(path) for ver, path in VERSION_SUFFIXES.items()}
VERSION_LIST = list(VERSION_SUFFIXES.keys())

# ----------------------------
# LOAD JSON vtable definitions
# ----------------------------

print("Generating vtables from JSON...")

with open(VTABLE_JSON, "r", encoding="utf-8") as f:
    raw_entries = json.load(f)

entries = []
for e in raw_entries:
    decl = e["decl"]
    symbol = e["symbol"]
    is_static = e.get("static", False)   # <-- default if missing

    # Extract field name & pointer-ness exactly like original script
    m_fp = re.search(r'\(\s*\*\s*([A-Za-z_]\w*)\s*\)', decl)
    if m_fp:
        field_name = m_fp.group(1)
        is_pointer = True
    else:
        ids = re.findall(r'([A-Za-z_]\w*)', decl)
        if not ids:
            print(f"Cannot extract field name for decl: {decl}", file=sys.stderr)
            continue
        field_name = ids[-1]
        no_paren = re.sub(r'\([^)]*\)', '', decl)
        is_pointer = "*" in no_paren

    entries.append({
        "decl": decl.rstrip(";").strip(),
        "symbol": symbol,
        "field_name": field_name,
        "static": is_static,
        "is_pointer": is_pointer,
    })


# ----------------------------
# GENERATE HEADER 
# ----------------------------

h_lines = []
h_lines.append("#ifndef VTABLES_H")
h_lines.append("#define VTABLES_H")
h_lines.append("")

for inc in INCLUDES:
    h_lines.append(inc)
h_lines.append("")

h_lines.append("typedef struct")
h_lines.append("{")
for e in entries:
    if e["static"]:
        continue    
    h_lines.append(f"    {e['decl']};")
h_lines.append("} VersionVTable;")
h_lines.append("")
h_lines.append("#endif // VTABLES_H")

Path(OUTPUT_HEADER).parent.mkdir(parents=True, exist_ok=True)
Path(OUTPUT_HEADER).write_text("\n".join(h_lines), encoding="utf-8")

# ----------------------------
# GENERATE STATIC C ARRAYS
# ----------------------------

sc_lines = []
sc_lines.append('#include "vtables.h"')
sc_lines.append("")

for e in entries:
    if e["static"]:
        arr_name = f"{e['field_name']}s"
        sc_lines.append(f"void* {arr_name}[] =")
        sc_lines.append("{")

        for idx, ver in enumerate(VERSION_LIST):
            # maintain original behavior: index 2 = NULL
            if idx == 2:
                sc_lines.append("    NULL,")

            lookup = f"{e['symbol']}_{ver}"
            val = version_symbols[ver].get(lookup)

            if val is None:
                sc_lines.append("    NULL,  /* missing */")
            else:
                sc_lines.append(f"    (void*){val},")

        sc_lines.append("};\n")

Path(OUTPUT_SC).parent.mkdir(parents=True, exist_ok=True)
Path(OUTPUT_SC).write_text("\n".join(sc_lines), encoding="utf-8")

# ----------------------------
# GENERATE VTABLE STRUCT INSTANCES
# ----------------------------

c_lines = []
c_lines.append('#include "../diskCode/vtables.h"')
c_lines.append("")

for ver in VERSION_LIST:
    symbols = version_symbols.get(ver, {})
    c_lines.append(f"VersionVTable VTABLE_{ver} =")
    c_lines.append("{")

    for e in entries:
        if e["static"]:
            continue

        fname = e["field_name"]
        lookup = f"{e['symbol']}_{ver}"
        val = symbols.get(lookup)

        if val is None:
            c_lines.append(f"    .{fname} = NULL,  // {lookup} not found")
        else:
            if e["is_pointer"]:
                c_lines.append(f"    .{fname} = (void*){val},")
            else:
                c_lines.append(f"    .{fname} = {val},")

    c_lines.append("};\n")

Path(OUTPUT_SOURCE).parent.mkdir(parents=True, exist_ok=True)
Path(OUTPUT_SOURCE).write_text("\n".join(c_lines), encoding="utf-8")

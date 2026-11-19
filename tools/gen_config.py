#!/usr/bin/env python3
import yaml
import sys
from pathlib import Path

def main():
    if len(sys.argv) != 3:
        print("usage: gen_config.py <experiment.yaml> <out_config.h>")
        sys.exit(1)

    exp_file = Path(sys.argv[1])
    out_file = Path(sys.argv[2])

    with exp_file.open() as f:
        exp = yaml.safe_load(f)

    cfg = exp.get("config", {})
    timing = cfg.get("timing", {})
    io = cfg.get("io", {})
    types = cfg.get("types", {})

    out_file.parent.mkdir(parents=True, exist_ok=True)

    with out_file.open("w") as f:
        f.write("#pragma once\n\n")

        for k, v in timing.items():
            f.write(f"#define {k} ({v})\n")

        f.write("\n")

        if io.get("USE_SERIAL_READER", False):
            f.write("#define USE_SERIAL_READER 1\n")
        else:
            f.write("#define USE_SERIAL_READER 0\n")

        serial_baud = io.get("SERIAL_BAUD", 115200)
        f.write(f"#define SERIAL_BAUD ({serial_baud})\n")

        f.write("\n")

        for name, ctype in types.items():
            f.write(f"typedef {ctype} {name};\n")

if __name__ == "__main__":
    main()

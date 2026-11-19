#!/usr/bin/env python3
"""Receive serial output from STM32 and save to log.

Stops when --secs elapses OR --lines lines are received (whichever comes first).
If neither --secs nor --lines is given, runs until Ctrl+C.
If --idle-secs is set, it also stops after that many seconds without new bytes.
"""
import argparse
import sys
import time
from collections import Counter

import serial


def main() -> None:
    parser = argparse.ArgumentParser(description="STM32 serial monitor")
    parser.add_argument("--port", required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument(
        "--secs", type=int, default=0,
        help="Stop after N seconds (0 = no time limit)",
    )
    parser.add_argument(
        "--lines", type=int, default=0,
        help="Stop after N newlines received (0 = no line limit)",
    )
    parser.add_argument(
        "--idle-secs", type=float, default=0,
        help="Stop after N seconds without receiving new bytes (0 = disabled)",
    )
    parser.add_argument("--log", required=True)
    args = parser.parse_args()

    print(f"[stm32-monitor] Opening {args.port} @ {args.baud}", flush=True)
    if args.secs:
        print(f"[stm32-monitor] Time limit : {args.secs} s", flush=True)
    if args.lines:
        print(f"[stm32-monitor] Line limit : {args.lines}", flush=True)
    if args.idle_secs:
        print(f"[stm32-monitor] Idle limit : {args.idle_secs:.1f} s", flush=True)
    if not args.secs and not args.lines:
        print("[stm32-monitor] No limit set — running until Ctrl+C", flush=True)

    ser = serial.Serial(args.port, args.baud, timeout=0.2)
    start = time.time()
    last_rx_at = start
    rx = 0
    lines_rx = 0
    byte_hist = Counter()
    try:
        with open(args.log, "wb") as out:
            while True:
                if args.secs and (time.time() - start >= args.secs):
                    break
                if args.lines and lines_rx >= args.lines:
                    break
                if args.idle_secs and rx > 0 and (time.time() - last_rx_at >= args.idle_secs):
                    break
                data = ser.read(1024)
                if data:
                    out.write(data)
                    out.flush()
                    byte_hist.update(data)
                    text = data.decode("utf-8", errors="replace")
                    sys.stdout.write(text)
                    sys.stdout.flush()
                    rx += len(data)
                    lines_rx += text.count("\n")
                    last_rx_at = time.time()
    except KeyboardInterrupt:
        print("\n[stm32-monitor] Interrupted by user", flush=True)

    ser.close()
    elapsed = time.time() - start
    print(f"\n[stm32-monitor] elapsed={elapsed:.1f}s  bytes_rx={rx}  lines_rx={lines_rx}")
    print(f"[stm32-monitor] log={args.log}")
    if rx > 0:
        top = byte_hist.most_common(5)
        top_fmt = ", ".join(f"0x{b:02X}:{n}" for b, n in top)
        print(f"[stm32-monitor] top_bytes={top_fmt}")
        if top and top[0][0] == 0x09 and (top[0][1] / rx) > 0.8:
            print(
                "[stm32-monitor] WARN: stream dominated by TAB (0x09); likely UART baud/clock mismatch or wrong serial source",
                flush=True,
            )


if __name__ == "__main__":
    main()

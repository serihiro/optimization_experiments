#!/usr/bin/env python3
import json
import re
import statistics
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PAT = re.compile(r"elapsed_sec=(\S+) gflops=(\S+) checksum=(\S+)")
SIZES = [256, 384, 512]
REPEATS = 3


def run_cmd(cmd):
    out = subprocess.check_output(cmd, cwd=ROOT, text=True)
    m = PAT.search(out)
    if not m:
        raise RuntimeError(f"unexpected output: {out}")
    return {
        "elapsed_sec": float(m.group(1)),
        "gflops": float(m.group(2)),
        "checksum": float(m.group(3)),
    }


def benchmark(binary: str, n: int):
    runs = [run_cmd([f"./{binary}", str(n)]) for _ in range(REPEATS)]
    return {
        "elapsed_sec_avg": statistics.mean(r["elapsed_sec"] for r in runs),
        "gflops_avg": statistics.mean(r["gflops"] for r in runs),
        "checksum": runs[0]["checksum"],
    }


def main():
    subprocess.check_call(["make", "clean"], cwd=ROOT)
    subprocess.check_call(["make"], cwd=ROOT)

    results = []
    print("| N | naive GFLOPS | optimized GFLOPS | speedup |")
    print("|---:|---:|---:|---:|")

    for n in SIZES:
        naive = benchmark("naive", n)
        optimized = benchmark("optimized", n)

        checksum_diff = abs(naive["checksum"] - optimized["checksum"])
        if checksum_diff > 1e-6:
            raise RuntimeError(f"checksum mismatch at N={n}: diff={checksum_diff}")

        speedup = optimized["gflops_avg"] / naive["gflops_avg"]
        row = {
            "N": n,
            "naive": naive,
            "optimized": optimized,
            "speedup": speedup,
        }
        results.append(row)
        print(f"| {n} | {naive['gflops_avg']:.3f} | {optimized['gflops_avg']:.3f} | {speedup:.2f}x |")

    out_path = ROOT / "results.json"
    out_path.write_text(json.dumps(results, indent=2), encoding="utf-8")
    print(f"\\nWrote detailed results to {out_path}")


if __name__ == "__main__":
    main()

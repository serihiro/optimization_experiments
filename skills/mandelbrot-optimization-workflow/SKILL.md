---
name: mandelbrot-optimization-workflow
description: Run an end-to-end optimization experiment workflow for Mandelbrot or similar numeric kernels: implement naive and multiple optimized versions (including SIMD candidates), benchmark them under fixed conditions, compare speed/accuracy, and write a markdown report with analysis and next-step proposals. Use when a user asks for iterative optimization + benchmark + report generation.
---

# Mandelbrot Optimization Workflow

Follow this checklist to execute one full experiment cycle.

1. Define scope
- Fix input size, max iteration/count limits, and correctness criteria.
- Decide implementation stages (`naive`, intermediate optimizations, SIMD, optional unroll).

2. Implement variants
- Keep `naive` readable and trusted as baseline.
- Add 2–4 optimized variants, each with one clear optimization idea.
- Keep variant naming explicit (`scalar_optimized`, `simd_avx2`, etc.).

3. Add correctness guardrails
- Use identical input coordinates/data across all variants.
- Verify with checksum or error metric against baseline.
- Fail fast if correctness check breaks.

4. Benchmark consistently
- Build with explicit compiler flags.
- Run each variant multiple times and record `best` + `avg`.
- Report speedup relative to `naive`.

5. Write report (`report.md`)
- Include goal, setup, benchmark table, and correctness status.
- Explain performance changes using divergence/branch/memory/SIMD utilization.
- Add concrete next optimization ideas.

6. Persist artifacts
- Keep per-theme directory with `benchmark.c`, `Makefile`, `report.md`.
- Update repository index (`README.md`) if needed.

## Output template

```markdown
# <Theme> Optimization Report

## Goal
...

## Setup
...

## Results
| impl | best | avg | speedup | correctness |
|---|---:|---:|---:|---|

## Analysis
...

## Next steps
- ...
```

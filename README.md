# About this repository
- C code snippets of optimization experiments

# How to try

```sh
$ git clone git@github.com:serihiro/optimization_experiments.git
$ cd optimization_experiments/dgemm
$ make execute_all
```

# Contents
- [x] dgemm
- [x] dgemv
- [x] gemm_cache_optimization
- [x] mandelbrot_simd
- [x] image_processing_simd

- [x] distance_computation_simd
- [x] kdtree_nearest_neighbor


# Benchmark build optimization flags
- `dgemm` と `dgemv` はデフォルトで `-O3` を使用します。
- 学習目的で最適化を無効化する場合は `OPT_LEVEL=-O0` を明示してください。

```sh
$ make -C dgemm clean execute_all OPT_LEVEL=-O0
$ make -C dgemv clean execute_all OPT_LEVEL=-O0
```

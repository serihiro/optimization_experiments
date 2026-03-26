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
- すべての実験ディレクトリの `Makefile` で、GCC最適化オプションはデフォルトで `-O3` です。
- 環境変数 `OPT_LEVEL` で上書きできます（例: `-O0`, `-O2`, `-Ofast`）。

```sh
$ make -C dgemm clean execute_all OPT_LEVEL=-O0
$ make -C mandelbrot_simd clean run OPT_LEVEL=-O2
$ make -C kdtree_nearest_neighbor clean run OPT_LEVEL=-Ofast
```

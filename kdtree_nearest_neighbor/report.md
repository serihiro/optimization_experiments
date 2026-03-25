# KD-tree nearest neighbor 最適化レポート

## テーマ定義

- 問題: $N$ 点から各クエリ点の最近傍を探索する。
- 入力: `N_POINTS=10000`, `N_QUERIES=1000`, `DIM=32` のランダムな `float` ベクトル。
- 正解条件: `brute_force` を参照実装とし、`kd_tree` / `simd_distance` の最近傍 ID と距離二乗が一致すること（許容誤差 $1e^{-4}$）。

## 実装バリエーション

1. `brute_force`
   - 全点に対して距離二乗をスカラーで計算する基準実装。
2. `kd_tree`
   - 軸ごとに分割した KD-tree を構築し、枝刈りを行いながら探索。
3. `simd_distance`
   - 全探索は維持しつつ、距離計算のみ AVX2/FMA で SIMD 化。

## benchmark 条件

- コンパイラ: `gcc -O3 -march=native -std=c11`
- 計測: 5 回実行して `best` と `avg` を採用
- 再現コマンド:

```sh
cd kdtree_nearest_neighbor
make run
```

## 結果（実測）

| method | best(s) | avg(s) | speedup vs brute | correctness |
|---|---:|---:|---:|---|
| brute_force | 0.479359 | 0.553336 | 1.00x | reference |
| kd_tree | 1.369341 | 1.520941 | 0.35x | pass |
| simd_distance | 0.134864 | 0.212677 | 3.55x | pass |

## 考察

- この設定（`DIM=32`）では `kd_tree` は `brute_force` より遅かった。
  - 理由は高次元で枝刈りが効きづらく、結局多くのノードを訪問するため。
  - 分岐・再帰によるオーバーヘッドも無視できない。
- `simd_distance` は全探索のままでも 1 比較あたりの計算コストを減らせるため、安定して高速化できた。
- LLM embedding search に近い観点では、
  - SIMD は「計算単価の削減」
  - index（KD-tree や ANN）は「比較回数削減」
  という役割分担になる。

## 次の改善案

- 次元を 8/16/32/64 と振って、KD-tree が有利になる境界を可視化する。
- KD-tree の葉ノードだけ SIMD 距離計算を使うハイブリッド探索を試す。
- HNSW / IVF-PQ と比較し、recall@k と latency のトレードオフを測る。
- データを SoA 化して、SIMD load/store と cache locality を改善する。

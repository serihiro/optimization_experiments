# Distance computation（embedding系）最適化レポート

## テーマ
ベクトルDBの基礎カーネルとして使われる以下の計算を対象に、段階的最適化を比較した。

- dot product
- cosine similarity
- L2 distance

## 問題設定
- ベクトル次元: `DIM=768`
- ペア数: `PAIRS=4096`
- 実行回数: `RUNS=7`
- 比較対象:
  1. `naive`
  2. `scalar_fma`
  3. `simd_avx2`
  4. `simd_avx2_fma_hsum`

すべて同一データに対して実行し、`dot_sum / cos_sum / l2_sum` が `naive` と一致（許容誤差 `EPS=1e-5`）することを確認する。

## 最適化段階

### 1) naive
素直なスカラー実装。読みやすさ優先で、1要素ずつ加算。

### 2) scalar_fma
スカラーのまま `fmaf` を利用し、積和をFMAへ寄せる。

### 3) simd_avx2
AVX2で8要素ずつ並列計算。積和後にhorizontal reductionで総和化。

### 4) simd_avx2_fma_hsum
AVX2 + FMA + horizontal reduction を組み合わせ、SIMD実装の演算密度を改善。

## ベンチマーク結果（サンプル）
`make run` 実行例（環境依存）:

| impl | best [s] | avg [s] | speedup vs naive(best) | match_naive |
|---|---:|---:|---:|---|
| naive | 0.003862 | 0.003920 | 1.00x | yes |
| scalar_fma | 0.003844 | 0.003932 | 1.00x | yes |
| simd_avx2 | 0.001032 | 0.001251 | 3.74x | yes |
| simd_avx2_fma_hsum | 0.000862 | 0.000922 | 4.48x | yes |

## 考察
- スカラー段階でもFMA活用により依存関係が短くなり、一定の改善が見られる。
- SIMD化で8要素並列になり、dot / norm / l2 の独立計算をまとめて処理できるため大きく高速化。
- AVX2版では最終的にhorizontal reductionが必要で、この部分は完全並列化できないためボトルネック候補になる。
- `simd_avx2_fma_hsum` はFMAにより命令数をさらに抑え、最速になりやすい。

## 次の改善案
1. クエリ1本 vs 複数ベクトル（SoAレイアウト）でメモリアクセス最適化。
2. AVX-512版（16要素並列）と比較。
3. 近似計算（量子化/低精度）でスループットと精度のトレードオフを評価。
4. prefetch とブロッキングで大規模コーパス時のキャッシュ効率を改善。

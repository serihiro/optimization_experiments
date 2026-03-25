# image_processing_simd report

## テーマ
- Image processing（SIMD教材）
- 対象フィルタ: `gaussian blur`, `sobel edge detection (x方向)`, `convolution(sharpen)`
- 実装比較: `scalar`, `SSE`, `AVX2`

## 入出力仕様
- 入力: `1920x1080` の 8-bit grayscale 画像
- 出力: 同サイズ 8-bit grayscale 画像（境界1pxは0で固定）
- 畳み込み: 3x3 kernel
- 正しさ: scalar 実装との checksum 一致で検証

## 実装バリエーション
1. `scalar`
   - 素直な3重ループ
2. `sse`
   - sliding window（`x` 方向16pxブロック）
   - packed pixels（`uint8 -> int16` 展開）
   - integer SIMD で積和 (`_mm_mullo_epi16`, `_mm_add_epi16`)
3. `avx2`
   - sliding window（`x` 方向16pxブロック）
   - packed pixels（`_mm256_cvtepu8_epi16`）
   - integer SIMD で積和 (`_mm256_mullo_epi16`, `_mm256_add_epi16`)

## benchmark 条件
- CPU/OS: 実行環境依存
- コンパイル: `gcc -O3 -std=c11 -Wall -Wextra -march=native -mavx2`
- 実行回数: `RUNS=5`
- 指標: `best` / `avg` / checksum一致

## 結果
### gaussian_blur
| impl | best [s] | avg [s] | speedup vs scalar(best) | correctness |
|---|---:|---:|---:|---|
| scalar | 0.010590 | 0.011574 | 1.00x | baseline |
| sse | 0.007806 | 0.008572 | 1.36x | yes |
| avx2 | 0.008095 | 0.009132 | 1.31x | yes |

### sobel_x
| impl | best [s] | avg [s] | speedup vs scalar(best) | correctness |
|---|---:|---:|---:|---|
| scalar | 0.009345 | 0.010535 | 1.00x | baseline |
| sse | 0.001258 | 0.001331 | 7.43x | yes |
| avx2 | 0.000931 | 0.001346 | 10.04x | yes |

### sharpen
| impl | best [s] | avg [s] | speedup vs scalar(best) | correctness |
|---|---:|---:|---:|---|
| scalar | 0.009070 | 0.010326 | 1.00x | baseline |
| sse | 0.001171 | 0.001651 | 7.75x | yes |
| avx2 | 0.001201 | 0.001569 | 7.55x | yes |

## 考察
- gaussian は kernel係数が正のみで除算処理（`/16`）が必要なため、SIMD化しても除算のオーバーヘッドが効いて伸びが限定的。
- sobel/sharpen は係数処理が軽く、integer SIMD の積和効率が効いて大きく高速化。
- AVX2 は sobel で最速だったが、gaussian/sharpen では SSE と拮抗。メモリ帯域・除算の影響が大きい。

## 次の改善案
1. gaussian 向けに `/16` をシフト (`>>4`) ベースの丸めに置き換える。
2. sobel を `Gx/Gy` 同時計算にしてロード再利用率を上げる。
3. AVX2 で 32px タイル化し、ロード/展開の再利用を増やす。
4. 入力の行バッファ化で cache locality を改善する。

# Mandelbrot SIMD 最適化レポート

## 目的
Mandelbrot集合の反復計算を題材に、以下の4実装を比較した。

1. `naive`: 素朴なスカラ実装
2. `scalar_optimized`: スカラのままループ内演算を整理
3. `simd_avx2`: AVX2 (4 lane, double) で4画素同時処理
4. `simd_avx2_unroll2`: AVX2 + 反復ループ2回アンロール

## 計測条件
- 解像度: `1920 x 1080`
- 最大反復回数: `MAX_ITER=1000`
- コンパイル: `gcc -O3 -march=native -mavx2 -mfma`
- 各実装を `5` 回計測し、`best` と `avg` を記録
- 各実装で同一の座標配列を利用し、checksum一致で正しさ確認

## ベンチマーク結果

実行コマンド:

```sh
cd mandelbrot_simd
make run
```

計測結果:

| impl | best [s] | avg [s] | speedup vs naive (best) | checksum match |
|---|---:|---:|---:|---|
| naive | 2.920916 | 2.944462 | 1.00x | yes |
| scalar_optimized | 2.919429 | 2.943262 | 1.00x | yes |
| simd_avx2 | 1.183748 | 1.204597 | 2.47x | yes |
| simd_avx2_unroll2 | 1.160712 | 1.200952 | 2.52x | yes |

## 考察
- `scalar_optimized` は `naive` とほぼ同等だった。Mandelbrot計算は分岐と反復回数のばらつきが支配的で、単純なスカラ微調整だけでは伸びにくい。
- `simd_avx2` は 4 lane 同時計算により約 `2.47x` 改善。理想値 `4x` に届かない主因は以下。
  - laneごとの発散タイミング差により、早く終わるlaneが待たされる（ダイバージェンス）
  - active mask判定やblend更新のオーバーヘッド
  - メモリ帯域より演算・分岐律速に近い特性
- `simd_avx2_unroll2` は `simd_avx2` 比でわずかに改善（約2%）。
  - ループ制御オーバーヘッドは減るが、分岐とmask制御が残るため伸び幅は限定的。

## 次の改善案
- **タイル化 + 画素再配置**: 発散特性の近い画素をまとめることでSIMDダイバージェンスを減らす。
- **精度切替**: 画質要件が許す領域で `float` 実装を導入し lane 幅を増やす。
- **OpenMP併用**: SIMD + スレッド並列で CPU コアを使い切る。
- **ROI最適化**: 高反復が集中する領域のみ高精度/高反復で処理。

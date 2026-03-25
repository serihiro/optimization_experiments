#define _POSIX_C_SOURCE 200809L
#include <immintrin.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH 1920
#define HEIGHT 1080
#define MAX_ITER 1000
#define RUNS 5

typedef void (*mandelbrot_fn)(uint16_t *out);

typedef struct {
  const char *name;
  mandelbrot_fn fn;
} impl_t;

static double x_coords[WIDTH];
static double y_coords[HEIGHT];

static void init_coords(void) {
  for (int x = 0; x < WIDTH; ++x) {
    x_coords[x] = -2.0 + 3.0 * (double)x / (double)WIDTH;
  }
  for (int y = 0; y < HEIGHT; ++y) {
    y_coords[y] = -1.0 + 2.0 * (double)y / (double)HEIGHT;
  }
}

static inline double now_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static inline uint64_t checksum(const uint16_t *buf) {
  uint64_t s = 0;
  for (int i = 0; i < WIDTH * HEIGHT; ++i) {
    s = (s * 1315423911u) ^ buf[i];
  }
  return s;
}

void mandelbrot_naive(uint16_t *out) {
  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      double cr = x_coords[x];
      double ci = y_coords[y];
      double zr = 0.0;
      double zi = 0.0;
      uint16_t iter = 0;
      while ((zr * zr + zi * zi <= 4.0) && (iter < MAX_ITER)) {
        double zr_next = zr * zr - zi * zi + cr;
        double zi_next = 2.0 * zr * zi + ci;
        zr = zr_next;
        zi = zi_next;
        iter++;
      }
      out[y * WIDTH + x] = iter;
    }
  }
}

void mandelbrot_scalar_optimized(uint16_t *out) {
  for (int y = 0; y < HEIGHT; ++y) {
    const double ci = y_coords[y];
    for (int x = 0; x < WIDTH; ++x) {
      const double cr = x_coords[x];
      double zr = 0.0;
      double zi = 0.0;
      double zr2 = 0.0;
      double zi2 = 0.0;
      uint16_t iter = 0;
      while ((zr2 + zi2 <= 4.0) && (iter < MAX_ITER)) {
        double zrzi = zr * zi;
        zr = zr2 - zi2 + cr;
        zi = 2.0 * zrzi + ci;
        zr2 = zr * zr;
        zi2 = zi * zi;
        iter++;
      }
      out[y * WIDTH + x] = iter;
    }
  }
}

void mandelbrot_avx2(uint16_t *out) {
  const __m256d two = _mm256_set1_pd(2.0);
  const __m256d four = _mm256_set1_pd(4.0);

  for (int y = 0; y < HEIGHT; ++y) {
    const __m256d civ = _mm256_set1_pd(y_coords[y]);
    int x = 0;
    for (; x + 3 < WIDTH; x += 4) {
      __m256d crv = _mm256_setr_pd(x_coords[x + 0], x_coords[x + 1], x_coords[x + 2], x_coords[x + 3]);
      __m256d zrv = _mm256_setzero_pd();
      __m256d ziv = _mm256_setzero_pd();
      __m256i itv = _mm256_setzero_si256();

      for (int i = 0; i < MAX_ITER; ++i) {
        __m256d zr2 = _mm256_mul_pd(zrv, zrv);
        __m256d zi2 = _mm256_mul_pd(ziv, ziv);
        __m256d mag2 = _mm256_add_pd(zr2, zi2);
        __m256d active = _mm256_cmp_pd(mag2, four, _CMP_LE_OQ);
        int mask = _mm256_movemask_pd(active);
        if (mask == 0) {
          break;
        }

        itv = _mm256_sub_epi64(itv, _mm256_castpd_si256(active));
        __m256d zrzi = _mm256_mul_pd(zrv, ziv);
        __m256d next_zr = _mm256_add_pd(_mm256_sub_pd(zr2, zi2), crv);
        __m256d next_zi = _mm256_add_pd(_mm256_mul_pd(two, zrzi), civ);
        zrv = _mm256_blendv_pd(zrv, next_zr, active);
        ziv = _mm256_blendv_pd(ziv, next_zi, active);
      }

      uint64_t tmp[4];
      _mm256_storeu_si256((__m256i *)tmp, itv);
      out[y * WIDTH + x + 0] = (uint16_t)tmp[0];
      out[y * WIDTH + x + 1] = (uint16_t)tmp[1];
      out[y * WIDTH + x + 2] = (uint16_t)tmp[2];
      out[y * WIDTH + x + 3] = (uint16_t)tmp[3];
    }

    for (; x < WIDTH; ++x) {
      double cr = x_coords[x];
      double zr = 0.0, zi = 0.0, zr2 = 0.0, zi2 = 0.0;
      uint16_t iter = 0;
      while ((zr2 + zi2 <= 4.0) && (iter < MAX_ITER)) {
        double zrzi = zr * zi;
        zr = zr2 - zi2 + cr;
        zi = 2.0 * zrzi + y_coords[y];
        zr2 = zr * zr;
        zi2 = zi * zi;
        iter++;
      }
      out[y * WIDTH + x] = iter;
    }
  }
}

void mandelbrot_avx2_unroll2(uint16_t *out) {
  const __m256d two = _mm256_set1_pd(2.0);
  const __m256d four = _mm256_set1_pd(4.0);

  for (int y = 0; y < HEIGHT; ++y) {
    const __m256d civ = _mm256_set1_pd(y_coords[y]);
    int x = 0;
    for (; x + 3 < WIDTH; x += 4) {
      __m256d crv = _mm256_setr_pd(x_coords[x + 0], x_coords[x + 1], x_coords[x + 2], x_coords[x + 3]);
      __m256d zrv = _mm256_setzero_pd();
      __m256d ziv = _mm256_setzero_pd();
      __m256i itv = _mm256_setzero_si256();

      for (int i = 0; i < MAX_ITER; i += 2) {
        for (int u = 0; u < 2; ++u) {
          __m256d zr2 = _mm256_mul_pd(zrv, zrv);
          __m256d zi2 = _mm256_mul_pd(ziv, ziv);
          __m256d mag2 = _mm256_add_pd(zr2, zi2);
          __m256d active = _mm256_cmp_pd(mag2, four, _CMP_LE_OQ);
          int mask = _mm256_movemask_pd(active);
          if (mask == 0) {
            goto store_result;
          }

          itv = _mm256_sub_epi64(itv, _mm256_castpd_si256(active));
          __m256d zrzi = _mm256_mul_pd(zrv, ziv);
          __m256d next_zr = _mm256_add_pd(_mm256_sub_pd(zr2, zi2), crv);
          __m256d next_zi = _mm256_add_pd(_mm256_mul_pd(two, zrzi), civ);
          zrv = _mm256_blendv_pd(zrv, next_zr, active);
          ziv = _mm256_blendv_pd(ziv, next_zi, active);
        }
      }

    store_result:
      uint64_t tmp[4];
      _mm256_storeu_si256((__m256i *)tmp, itv);
      out[y * WIDTH + x + 0] = (uint16_t)tmp[0];
      out[y * WIDTH + x + 1] = (uint16_t)tmp[1];
      out[y * WIDTH + x + 2] = (uint16_t)tmp[2];
      out[y * WIDTH + x + 3] = (uint16_t)tmp[3];
    }

    for (; x < WIDTH; ++x) {
      double cr = x_coords[x];
      double ci = y_coords[y];
      double zr = 0.0, zi = 0.0, zr2 = 0.0, zi2 = 0.0;
      uint16_t iter = 0;
      while ((zr2 + zi2 <= 4.0) && (iter < MAX_ITER)) {
        double zrzi = zr * zi;
        zr = zr2 - zi2 + cr;
        zi = 2.0 * zrzi + ci;
        zr2 = zr * zr;
        zi2 = zi * zi;
        iter++;
      }
      out[y * WIDTH + x] = iter;
    }
  }
}

static void benchmark_impl(const impl_t impl, uint64_t *out_checksum, double *out_best,
                           double *out_avg) {
  uint16_t *buf = (uint16_t *)aligned_alloc(32, WIDTH * HEIGHT * sizeof(uint16_t));
  if (!buf) {
    fprintf(stderr, "allocation failed\n");
    exit(1);
  }
  double best = 1e30;
  double sum = 0.0;
  uint64_t chk = 0;

  for (int r = 0; r < RUNS; ++r) {
    memset(buf, 0, WIDTH * HEIGHT * sizeof(uint16_t));
    double t0 = now_sec();
    impl.fn(buf);
    double t1 = now_sec();
    double elapsed = t1 - t0;
    if (elapsed < best) {
      best = elapsed;
    }
    sum += elapsed;
    chk = checksum(buf);
  }

  free(buf);
  *out_checksum = chk;
  *out_best = best;
  *out_avg = sum / RUNS;
}

int main(void) {
  init_coords();
  const impl_t impls[] = {
      {"naive", mandelbrot_naive},
      {"scalar_optimized", mandelbrot_scalar_optimized},
      {"simd_avx2", mandelbrot_avx2},
      {"simd_avx2_unroll2", mandelbrot_avx2_unroll2},
  };

  uint64_t baseline = 0;
  for (size_t i = 0; i < sizeof(impls) / sizeof(impls[0]); ++i) {
    uint64_t chk = 0;
    double best = 0.0, avg = 0.0;
    benchmark_impl(impls[i], &chk, &best, &avg);
    if (i == 0) {
      baseline = chk;
    }
    printf("impl=%s best=%.6f avg=%.6f checksum=%llu match_naive=%s\n", impls[i].name,
           best, avg, (unsigned long long)chk, (chk == baseline) ? "yes" : "no");
  }

  return 0;
}

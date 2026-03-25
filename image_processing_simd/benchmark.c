#define _POSIX_C_SOURCE 200809L
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH 1920
#define HEIGHT 1080
#define RUNS 5

typedef struct {
  const char *name;
  int8_t k[3][3];
  int divisor;
  int use_abs;
} filter_t;

typedef void (*conv_fn)(const uint8_t *src, uint8_t *dst, const filter_t *filter);

typedef struct {
  const char *name;
  conv_fn fn;
} impl_t;

static uint8_t image[WIDTH * HEIGHT];

static inline double now_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static inline uint64_t checksum(const uint8_t *buf) {
  uint64_t s = 1469598103934665603ULL;
  for (int i = 0; i < WIDTH * HEIGHT; ++i) {
    s ^= buf[i];
    s *= 1099511628211ULL;
  }
  return s;
}

static void init_image(void) {
  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      uint32_t v = (uint32_t)(x * 13 + y * 17 + ((x ^ y) * 7));
      image[y * WIDTH + x] = (uint8_t)(v & 0xff);
    }
  }
}

static inline uint8_t clamp_u8(int v) {
  if (v < 0) {
    return 0;
  }
  if (v > 255) {
    return 255;
  }
  return (uint8_t)v;
}

void convolution_scalar(const uint8_t *src, uint8_t *dst, const filter_t *filter) {
  memset(dst, 0, WIDTH * HEIGHT);
  for (int y = 1; y < HEIGHT - 1; ++y) {
    for (int x = 1; x < WIDTH - 1; ++x) {
      int acc = 0;
      for (int ky = -1; ky <= 1; ++ky) {
        for (int kx = -1; kx <= 1; ++kx) {
          acc += (int)src[(y + ky) * WIDTH + (x + kx)] * (int)filter->k[ky + 1][kx + 1];
        }
      }
      if (filter->use_abs && acc < 0) {
        acc = -acc;
      }
      if (filter->divisor > 1) {
        acc = (acc + filter->divisor / 2) / filter->divisor;
      }
      dst[y * WIDTH + x] = clamp_u8(acc);
    }
  }
}

static inline __m128i abs_epi16_sse2(__m128i v) {
  __m128i neg = _mm_sub_epi16(_mm_setzero_si128(), v);
  return _mm_max_epi16(v, neg);
}

void convolution_sse(const uint8_t *src, uint8_t *dst, const filter_t *filter) {
  memset(dst, 0, WIDTH * HEIGHT);
  const __m128i zero = _mm_setzero_si128();

  for (int y = 1; y < HEIGHT - 1; ++y) {
    int x = 1;
    for (; x + 15 < WIDTH - 1; x += 16) {
      __m128i acc_lo = _mm_setzero_si128();
      __m128i acc_hi = _mm_setzero_si128();

      for (int ky = -1; ky <= 1; ++ky) {
        const uint8_t *row = src + (y + ky) * WIDTH + x;
        for (int kx = -1; kx <= 1; ++kx) {
          int8_t kw = filter->k[ky + 1][kx + 1];
          if (kw == 0) {
            continue;
          }
          __m128i pix = _mm_loadu_si128((const __m128i *)(row + kx));
          __m128i lo = _mm_unpacklo_epi8(pix, zero);
          __m128i hi = _mm_unpackhi_epi8(pix, zero);
          __m128i w = _mm_set1_epi16(kw);
          acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(lo, w));
          acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(hi, w));
        }
      }

      if (filter->use_abs) {
        acc_lo = abs_epi16_sse2(acc_lo);
        acc_hi = abs_epi16_sse2(acc_hi);
      }

      if (filter->divisor > 1) {
        __m128i half = _mm_set1_epi16((short)(filter->divisor / 2));
        acc_lo = _mm_add_epi16(acc_lo, half);
        acc_hi = _mm_add_epi16(acc_hi, half);
        int16_t buf_lo[8];
        int16_t buf_hi[8];
        _mm_storeu_si128((__m128i *)buf_lo, acc_lo);
        _mm_storeu_si128((__m128i *)buf_hi, acc_hi);
        for (int i = 0; i < 8; ++i) {
          buf_lo[i] = (int16_t)((uint16_t)buf_lo[i] / (uint16_t)filter->divisor);
          buf_hi[i] = (int16_t)((uint16_t)buf_hi[i] / (uint16_t)filter->divisor);
        }
        acc_lo = _mm_loadu_si128((const __m128i *)buf_lo);
        acc_hi = _mm_loadu_si128((const __m128i *)buf_hi);
      }

      __m128i packed = _mm_packus_epi16(acc_lo, acc_hi);
      _mm_storeu_si128((__m128i *)(dst + y * WIDTH + x), packed);
    }

    for (; x < WIDTH - 1; ++x) {
      int acc = 0;
      for (int ky = -1; ky <= 1; ++ky) {
        for (int kx = -1; kx <= 1; ++kx) {
          acc += (int)src[(y + ky) * WIDTH + (x + kx)] * (int)filter->k[ky + 1][kx + 1];
        }
      }
      if (filter->use_abs && acc < 0) {
        acc = -acc;
      }
      if (filter->divisor > 1) {
        acc = (acc + filter->divisor / 2) / filter->divisor;
      }
      dst[y * WIDTH + x] = clamp_u8(acc);
    }
  }
}

void convolution_avx2(const uint8_t *src, uint8_t *dst, const filter_t *filter) {
  memset(dst, 0, WIDTH * HEIGHT);

  for (int y = 1; y < HEIGHT - 1; ++y) {
    int x = 1;
    for (; x + 15 < WIDTH - 1; x += 16) {
      __m256i acc = _mm256_setzero_si256();

      for (int ky = -1; ky <= 1; ++ky) {
        const uint8_t *row = src + (y + ky) * WIDTH + x;
        for (int kx = -1; kx <= 1; ++kx) {
          int8_t kw = filter->k[ky + 1][kx + 1];
          if (kw == 0) {
            continue;
          }
          __m128i pix8 = _mm_loadu_si128((const __m128i *)(row + kx));
          __m256i pix16 = _mm256_cvtepu8_epi16(pix8);
          __m256i w = _mm256_set1_epi16(kw);
          acc = _mm256_add_epi16(acc, _mm256_mullo_epi16(pix16, w));
        }
      }

      if (filter->use_abs) {
        acc = _mm256_abs_epi16(acc);
      }

      if (filter->divisor > 1) {
        __m256i half = _mm256_set1_epi16((short)(filter->divisor / 2));
        acc = _mm256_add_epi16(acc, half);
        int16_t buf[16];
        _mm256_storeu_si256((__m256i *)buf, acc);
        for (int i = 0; i < 16; ++i) {
          buf[i] = (int16_t)((uint16_t)buf[i] / (uint16_t)filter->divisor);
        }
        acc = _mm256_loadu_si256((const __m256i *)buf);
      }

      __m128i lo = _mm256_castsi256_si128(acc);
      __m128i hi = _mm256_extracti128_si256(acc, 1);
      __m128i packed = _mm_packus_epi16(lo, hi);
      _mm_storeu_si128((__m128i *)(dst + y * WIDTH + x), packed);
    }

    for (; x < WIDTH - 1; ++x) {
      int acc = 0;
      for (int ky = -1; ky <= 1; ++ky) {
        for (int kx = -1; kx <= 1; ++kx) {
          acc += (int)src[(y + ky) * WIDTH + (x + kx)] * (int)filter->k[ky + 1][kx + 1];
        }
      }
      if (filter->use_abs && acc < 0) {
        acc = -acc;
      }
      if (filter->divisor > 1) {
        acc = (acc + filter->divisor / 2) / filter->divisor;
      }
      dst[y * WIDTH + x] = clamp_u8(acc);
    }
  }
}

static void benchmark_one(const impl_t *impl, const filter_t *filter, uint64_t *out_chk,
                          double *out_best, double *out_avg) {
  uint8_t *dst = aligned_alloc(32, WIDTH * HEIGHT);
  if (!dst) {
    fprintf(stderr, "allocation failed\n");
    exit(1);
  }

  double best = 1e30;
  double sum = 0.0;
  uint64_t chk = 0;

  for (int r = 0; r < RUNS; ++r) {
    memset(dst, 0, WIDTH * HEIGHT);
    double t0 = now_sec();
    impl->fn(image, dst, filter);
    double t1 = now_sec();
    double dt = t1 - t0;
    if (dt < best) {
      best = dt;
    }
    sum += dt;
    chk = checksum(dst);
  }

  free(dst);
  *out_chk = chk;
  *out_best = best;
  *out_avg = sum / RUNS;
}

int main(void) {
  init_image();

  const filter_t filters[] = {
      {"gaussian_blur", {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}}, 16, 0},
      {"sobel_x", {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}, 1, 1},
      {"sharpen", {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}}, 1, 0},
  };

  const impl_t impls[] = {
      {"scalar", convolution_scalar},
      {"sse", convolution_sse},
      {"avx2", convolution_avx2},
  };

  for (size_t f = 0; f < sizeof(filters) / sizeof(filters[0]); ++f) {
    printf("filter=%s\n", filters[f].name);
    uint64_t baseline = 0;

    for (size_t i = 0; i < sizeof(impls) / sizeof(impls[0]); ++i) {
      uint64_t chk = 0;
      double best = 0.0;
      double avg = 0.0;
      benchmark_one(&impls[i], &filters[f], &chk, &best, &avg);
      if (i == 0) {
        baseline = chk;
      }
      printf("  impl=%s best=%.6f avg=%.6f checksum=%llu match_scalar=%s\n", impls[i].name,
             best, avg, (unsigned long long)chk, (chk == baseline) ? "yes" : "no");
    }
  }

  return 0;
}

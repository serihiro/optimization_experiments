#define _POSIX_C_SOURCE 200809L
#include <immintrin.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DIM 768
#define PAIRS 4096
#define RUNS 7
#define EPS 1e-5f

typedef struct {
  float dot_sum;
  float cos_sum;
  float l2_sum;
} metrics_t;

typedef void (*kernel_fn)(const float *a, const float *b, int dim, metrics_t *out);

typedef struct {
  const char *name;
  kernel_fn fn;
} impl_t;

static float *g_a;
static float *g_b;

static inline double now_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void init_data(void) {
  g_a = aligned_alloc(32, (size_t)PAIRS * DIM * sizeof(float));
  g_b = aligned_alloc(32, (size_t)PAIRS * DIM * sizeof(float));
  if (!g_a || !g_b) {
    fprintf(stderr, "allocation failed\n");
    exit(1);
  }

  for (int i = 0; i < PAIRS * DIM; ++i) {
    uint32_t x = (uint32_t)(i * 1664525u + 1013904223u);
    uint32_t y = (uint32_t)(i * 22695477u + 1u);
    g_a[i] = ((float)(x & 0xffffu) / 65535.0f) * 2.0f - 1.0f;
    g_b[i] = ((float)(y & 0xffffu) / 65535.0f) * 2.0f - 1.0f;
  }
}

static inline float hsum256_ps(__m256 v) {
  __m128 low = _mm256_castps256_ps128(v);
  __m128 high = _mm256_extractf128_ps(v, 1);
  __m128 sum = _mm_add_ps(low, high);
  sum = _mm_hadd_ps(sum, sum);
  sum = _mm_hadd_ps(sum, sum);
  return _mm_cvtss_f32(sum);
}

static void kernel_naive(const float *a, const float *b, int dim, metrics_t *out) {
  float dot = 0.0f;
  float na = 0.0f;
  float nb = 0.0f;
  float l2 = 0.0f;

  for (int i = 0; i < dim; ++i) {
    float av = a[i];
    float bv = b[i];
    float d = av - bv;
    dot += av * bv;
    na += av * av;
    nb += bv * bv;
    l2 += d * d;
  }

  out->dot_sum += dot;
  out->cos_sum += dot / (sqrtf(na) * sqrtf(nb) + 1e-12f);
  out->l2_sum += sqrtf(l2);
}

static void kernel_scalar_fma(const float *a, const float *b, int dim, metrics_t *out) {
  float dot = 0.0f;
  float na = 0.0f;
  float nb = 0.0f;
  float l2 = 0.0f;

  for (int i = 0; i < dim; ++i) {
    float av = a[i];
    float bv = b[i];
    float d = av - bv;
    dot = fmaf(av, bv, dot);
    na = fmaf(av, av, na);
    nb = fmaf(bv, bv, nb);
    l2 = fmaf(d, d, l2);
  }

  out->dot_sum += dot;
  out->cos_sum += dot / (sqrtf(na) * sqrtf(nb) + 1e-12f);
  out->l2_sum += sqrtf(l2);
}

static void kernel_avx2(const float *a, const float *b, int dim, metrics_t *out) {
  __m256 dotv = _mm256_setzero_ps();
  __m256 nav = _mm256_setzero_ps();
  __m256 nbv = _mm256_setzero_ps();
  __m256 l2v = _mm256_setzero_ps();

  int i = 0;
  for (; i + 7 < dim; i += 8) {
    __m256 av = _mm256_load_ps(a + i);
    __m256 bv = _mm256_load_ps(b + i);
    __m256 dv = _mm256_sub_ps(av, bv);
    dotv = _mm256_add_ps(dotv, _mm256_mul_ps(av, bv));
    nav = _mm256_add_ps(nav, _mm256_mul_ps(av, av));
    nbv = _mm256_add_ps(nbv, _mm256_mul_ps(bv, bv));
    l2v = _mm256_add_ps(l2v, _mm256_mul_ps(dv, dv));
  }

  float dot = hsum256_ps(dotv);
  float na = hsum256_ps(nav);
  float nb = hsum256_ps(nbv);
  float l2 = hsum256_ps(l2v);

  for (; i < dim; ++i) {
    float av = a[i];
    float bv = b[i];
    float d = av - bv;
    dot += av * bv;
    na += av * av;
    nb += bv * bv;
    l2 += d * d;
  }

  out->dot_sum += dot;
  out->cos_sum += dot / (sqrtf(na) * sqrtf(nb) + 1e-12f);
  out->l2_sum += sqrtf(l2);
}

static void kernel_avx2_fma_hsum(const float *a, const float *b, int dim, metrics_t *out) {
  __m256 dotv = _mm256_setzero_ps();
  __m256 nav = _mm256_setzero_ps();
  __m256 nbv = _mm256_setzero_ps();
  __m256 l2v = _mm256_setzero_ps();

  int i = 0;
  for (; i + 7 < dim; i += 8) {
    __m256 av = _mm256_load_ps(a + i);
    __m256 bv = _mm256_load_ps(b + i);
    __m256 dv = _mm256_sub_ps(av, bv);
    dotv = _mm256_fmadd_ps(av, bv, dotv);
    nav = _mm256_fmadd_ps(av, av, nav);
    nbv = _mm256_fmadd_ps(bv, bv, nbv);
    l2v = _mm256_fmadd_ps(dv, dv, l2v);
  }

  float dot = hsum256_ps(dotv);
  float na = hsum256_ps(nav);
  float nb = hsum256_ps(nbv);
  float l2 = hsum256_ps(l2v);

  for (; i < dim; ++i) {
    float av = a[i];
    float bv = b[i];
    float d = av - bv;
    dot = fmaf(av, bv, dot);
    na = fmaf(av, av, na);
    nb = fmaf(bv, bv, nb);
    l2 = fmaf(d, d, l2);
  }

  out->dot_sum += dot;
  out->cos_sum += dot / (sqrtf(na) * sqrtf(nb) + 1e-12f);
  out->l2_sum += sqrtf(l2);
}

static void run_impl(const impl_t *impl, metrics_t *metrics, double *best, double *avg) {
  double best_time = 1e30;
  double total = 0.0;
  metrics_t last = {0};

  for (int r = 0; r < RUNS; ++r) {
    metrics_t acc = {0};
    double t0 = now_sec();
    for (int p = 0; p < PAIRS; ++p) {
      impl->fn(g_a + (size_t)p * DIM, g_b + (size_t)p * DIM, DIM, &acc);
    }
    double t1 = now_sec();
    double dt = t1 - t0;
    if (dt < best_time) {
      best_time = dt;
    }
    total += dt;
    last = acc;
  }

  *metrics = last;
  *best = best_time;
  *avg = total / RUNS;
}

static int almost_equal(float a, float b) {
  float diff = fabsf(a - b);
  float scale = fmaxf(1.0f, fmaxf(fabsf(a), fabsf(b)));
  return diff <= EPS * scale;
}

int main(void) {
  init_data();

  const impl_t impls[] = {
      {"naive", kernel_naive},
      {"scalar_fma", kernel_scalar_fma},
      {"simd_avx2", kernel_avx2},
      {"simd_avx2_fma_hsum", kernel_avx2_fma_hsum},
  };

  metrics_t baseline = {0};
  for (size_t i = 0; i < sizeof(impls) / sizeof(impls[0]); ++i) {
    metrics_t m;
    double best, avg;
    run_impl(&impls[i], &m, &best, &avg);

    if (i == 0) {
      baseline = m;
    }

    int ok = almost_equal(m.dot_sum, baseline.dot_sum) && almost_equal(m.cos_sum, baseline.cos_sum) &&
             almost_equal(m.l2_sum, baseline.l2_sum);

    printf("impl=%s best=%.6f avg=%.6f dot_sum=%.6f cos_sum=%.6f l2_sum=%.6f match_naive=%s\n",
           impls[i].name, best, avg, m.dot_sum, m.cos_sum, m.l2_sum, ok ? "yes" : "no");
  }

  free(g_a);
  free(g_b);
  return 0;
}

#define _POSIX_C_SOURCE 200809L
#include <immintrin.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef N_POINTS
#define N_POINTS 10000
#endif

#ifndef N_QUERIES
#define N_QUERIES 1000
#endif

#ifndef DIM
#define DIM 32
#endif

#ifndef RUNS
#define RUNS 5
#endif

typedef struct {
    float v[DIM];
    int id;
} Point;

typedef struct KDNode {
    int point_index;
    int axis;
    struct KDNode* left;
    struct KDNode* right;
} KDNode;

static int cmp_axis = 0;
static Point* cmp_points = NULL;

static int cmp_point_index(const void* a, const void* b) {
    const int ia = *(const int*)a;
    const int ib = *(const int*)b;
    float da = cmp_points[ia].v[cmp_axis];
    float db = cmp_points[ib].v[cmp_axis];
    if (da < db) return -1;
    if (da > db) return 1;
    return ia - ib;
}

static inline double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static inline float l2_dist_sq_scalar(const float* a, const float* b) {
    float sum = 0.0f;
    for (int d = 0; d < DIM; d++) {
        float diff = a[d] - b[d];
        sum += diff * diff;
    }
    return sum;
}

static inline float l2_dist_sq_simd(const float* a, const float* b) {
#if defined(__AVX2__)
    __m256 acc = _mm256_setzero_ps();
    int d = 0;
    for (; d + 8 <= DIM; d += 8) {
        __m256 va = _mm256_loadu_ps(a + d);
        __m256 vb = _mm256_loadu_ps(b + d);
        __m256 diff = _mm256_sub_ps(va, vb);
        acc = _mm256_fmadd_ps(diff, diff, acc);
    }
    float tmp[8];
    _mm256_storeu_ps(tmp, acc);
    float sum = tmp[0] + tmp[1] + tmp[2] + tmp[3] + tmp[4] + tmp[5] + tmp[6] + tmp[7];
    for (; d < DIM; d++) {
        float diff = a[d] - b[d];
        sum += diff * diff;
    }
    return sum;
#else
    return l2_dist_sq_scalar(a, b);
#endif
}

static int brute_force_scalar(const Point* points, int n_points, const float* query, float* out_best_dist) {
    int best_id = -1;
    float best_dist = INFINITY;
    for (int i = 0; i < n_points; i++) {
        float dist = l2_dist_sq_scalar(points[i].v, query);
        if (dist < best_dist) {
            best_dist = dist;
            best_id = points[i].id;
        }
    }
    *out_best_dist = best_dist;
    return best_id;
}

static int brute_force_simd(const Point* points, int n_points, const float* query, float* out_best_dist) {
    int best_id = -1;
    float best_dist = INFINITY;
    for (int i = 0; i < n_points; i++) {
        float dist = l2_dist_sq_simd(points[i].v, query);
        if (dist < best_dist) {
            best_dist = dist;
            best_id = points[i].id;
        }
    }
    *out_best_dist = best_dist;
    return best_id;
}

static KDNode* build_kdtree(Point* points, int* idx, int n, int depth, KDNode* pool, int* pool_pos) {
    if (n <= 0) {
        return NULL;
    }
    int axis = depth % DIM;
    cmp_axis = axis;
    cmp_points = points;
    qsort(idx, (size_t)n, sizeof(int), cmp_point_index);

    int mid = n / 2;
    KDNode* node = &pool[(*pool_pos)++];
    node->point_index = idx[mid];
    node->axis = axis;
    node->left = build_kdtree(points, idx, mid, depth + 1, pool, pool_pos);
    node->right = build_kdtree(points, idx + mid + 1, n - mid - 1, depth + 1, pool, pool_pos);
    return node;
}

typedef struct {
    int best_id;
    float best_dist;
} NNResult;

static void kd_nearest(const Point* points, const KDNode* node, const float* query, NNResult* result) {
    if (!node) {
        return;
    }

    const Point* p = &points[node->point_index];
    float dist = l2_dist_sq_scalar(p->v, query);
    if (dist < result->best_dist) {
        result->best_dist = dist;
        result->best_id = p->id;
    }

    int axis = node->axis;
    float diff = query[axis] - p->v[axis];

    const KDNode* near_branch = diff <= 0 ? node->left : node->right;
    const KDNode* far_branch = diff <= 0 ? node->right : node->left;

    kd_nearest(points, near_branch, query, result);

    if (diff * diff < result->best_dist) {
        kd_nearest(points, far_branch, query, result);
    }
}

static int kd_tree_search(const Point* points, const KDNode* root, const float* query, float* out_best_dist) {
    NNResult result = {.best_id = -1, .best_dist = INFINITY};
    kd_nearest(points, root, query, &result);
    *out_best_dist = result.best_dist;
    return result.best_id;
}

static uint64_t run_method(
    const char* name,
    int (*method)(const Point*, int, const float*, float*),
    const Point* points,
    int n_points,
    const float* queries,
    int n_queries,
    const int* ref_ids,
    const float* ref_dists,
    int verify,
    double* out_best,
    double* out_avg
) {
    double best = INFINITY;
    double total = 0.0;
    uint64_t checksum = 0;

    for (int r = 0; r < RUNS; r++) {
        double t0 = now_sec();
        uint64_t run_checksum = 0;
        int mismatch = 0;

        for (int q = 0; q < n_queries; q++) {
            float dist;
            int id = method(points, n_points, queries + (size_t)q * DIM, &dist);
            run_checksum += (uint64_t)(id + 1) * (uint64_t)(q + 1);
            if (verify && (id != ref_ids[q] || fabsf(dist - ref_dists[q]) > 1e-4f)) {
                mismatch++;
            }
        }

        double t1 = now_sec();
        double elapsed = t1 - t0;
        if (elapsed < best) best = elapsed;
        total += elapsed;
        checksum = run_checksum;

        if (verify && mismatch > 0) {
            fprintf(stderr, "[ERROR] %s mismatch count: %d\n", name, mismatch);
            exit(1);
        }
    }

    *out_best = best;
    *out_avg = total / RUNS;
    return checksum;
}

static int kd_method_adapter(const Point* points, int n_points, const float* query, float* out_best_dist);
static const KDNode* g_kd_root = NULL;

static int kd_method_adapter(const Point* points, int n_points, const float* query, float* out_best_dist) {
    (void)n_points;
    return kd_tree_search(points, g_kd_root, query, out_best_dist);
}

int main(void) {
    Point* points = (Point*)aligned_alloc(32, sizeof(Point) * N_POINTS);
    float* queries = (float*)aligned_alloc(32, sizeof(float) * (size_t)N_QUERIES * DIM);
    int* idx = (int*)malloc(sizeof(int) * N_POINTS);
    KDNode* pool = (KDNode*)malloc(sizeof(KDNode) * N_POINTS);
    int* ref_ids = (int*)malloc(sizeof(int) * N_QUERIES);
    float* ref_dists = (float*)malloc(sizeof(float) * N_QUERIES);

    if (!points || !queries || !idx || !pool || !ref_ids || !ref_dists) {
        fprintf(stderr, "allocation failed\n");
        return 1;
    }

    srand(42);
    for (int i = 0; i < N_POINTS; i++) {
        points[i].id = i;
        idx[i] = i;
        for (int d = 0; d < DIM; d++) {
            points[i].v[d] = (float)rand() / (float)RAND_MAX;
        }
    }
    for (int q = 0; q < N_QUERIES; q++) {
        for (int d = 0; d < DIM; d++) {
            queries[(size_t)q * DIM + d] = (float)rand() / (float)RAND_MAX;
        }
    }

    int pool_pos = 0;
    KDNode* root = build_kdtree(points, idx, N_POINTS, 0, pool, &pool_pos);
    g_kd_root = root;

    for (int q = 0; q < N_QUERIES; q++) {
        ref_ids[q] = brute_force_scalar(points, N_POINTS, queries + (size_t)q * DIM, &ref_dists[q]);
    }

    double best_brute, avg_brute;
    double best_kd, avg_kd;
    double best_simd, avg_simd;

    uint64_t sum_brute = run_method(
        "brute_force_scalar",
        brute_force_scalar,
        points,
        N_POINTS,
        queries,
        N_QUERIES,
        ref_ids,
        ref_dists,
        0,
        &best_brute,
        &avg_brute
    );

    uint64_t sum_kd = run_method(
        "kd_tree",
        kd_method_adapter,
        points,
        N_POINTS,
        queries,
        N_QUERIES,
        ref_ids,
        ref_dists,
        1,
        &best_kd,
        &avg_kd
    );

    uint64_t sum_simd = run_method(
        "brute_force_simd",
        brute_force_simd,
        points,
        N_POINTS,
        queries,
        N_QUERIES,
        ref_ids,
        ref_dists,
        1,
        &best_simd,
        &avg_simd
    );

    printf("KD-tree nearest neighbor benchmark\n");
    printf("N_POINTS=%d N_QUERIES=%d DIM=%d RUNS=%d\n\n", N_POINTS, N_QUERIES, DIM, RUNS);

    printf("%-20s %-12s %-12s %-12s %-16s\n", "method", "best(s)", "avg(s)", "speedup", "checksum");
    printf("%-20s %-12.6f %-12.6f %-.2fx      %-16llu\n", "brute_force", best_brute, avg_brute, best_brute / best_brute,
           (unsigned long long)sum_brute);
    printf("%-20s %-12.6f %-12.6f %-.2fx      %-16llu\n", "kd_tree", best_kd, avg_kd, best_brute / best_kd,
           (unsigned long long)sum_kd);
    printf("%-20s %-12.6f %-12.6f %-.2fx      %-16llu\n", "simd_distance", best_simd, avg_simd, best_brute / best_simd,
           (unsigned long long)sum_simd);

    free(points);
    free(queries);
    free(idx);
    free(pool);
    free(ref_ids);
    free(ref_dists);
    return 0;
}

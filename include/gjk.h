#ifndef GJK_H
#define GJK_H

#define GJK_MAX_ITERATIONS 20

struct gjk_support {
    int aid, bid;
    float a[3];
    float b[3];
};
struct gjk_simplex {
    int max_iter, iter;
    int hit, cnt;
    struct gjk_vertex {
        float a[3];
        float b[3];
        float p[3];
        int aid, bid;
    } v[4];
    float bc[4], D;
};
struct gjk_result {
    int hit;
    float p0[3];
    float p1[3];
    float distance_squared;
    int iterations;
};
extern int gjk(struct gjk_simplex *s, const struct gjk_support *sup, float *dv);
extern void gjk_analyze(struct gjk_result *res, const struct gjk_simplex *s);
extern void gjk_quad(struct gjk_result *res, float a_radius, float b_radius);

#endif
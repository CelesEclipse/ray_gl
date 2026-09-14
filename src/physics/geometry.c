#include <stdlib.h>
#include <float.h>
#include <string.h>
#include "geometry.h"
#include "raylib.h"
#include "raymath.h"

/* A simple struct describing a capsule collider 3D
- base: center of the bottom hemisphere
- tip: center of the top hemisphere
- radius: thickness, radius of one circle slice of the main cylinder
*/
struct CapsuleCollider3D
{
    Vector3 base;
    Vector3 tip;
    float radius;
};

CapsuleCollider3D_t * geometry_capsule_alloc()
{
    CapsuleCollider3D_t * ret = malloc(sizeof(CapsuleCollider3D_t));
    if (!ret) {
        TraceLog(LOG_FATAL, "Failed to allocate capsule object");
        return NULL;
    } else {
        memset(ret, 0, sizeof(CapsuleCollider3D_t));
    }

    return ret;
}

void geometry_capsule_destroy(CapsuleCollider3D_t * capsule)
{
    if (capsule != NULL) {
        free(capsule);
    }
    capsule = NULL;
}

float geometry_capsule_calculate_height(const CapsuleCollider3D_t *capsule)
{
    if (!capsule) return 0.0f;

    float total_height = Vector3Distance(capsule->base, capsule->tip) + (2.0f * capsule->radius);
    return total_height;
}

Vector3 geometry_capsule_get_coord(const CapsuleCollider3D_t * capsule, int type)
{
    if (!capsule) return Vector3Zero();

    Vector3 ret = Vector3Zero();
    if (type == 0) {
        ret = capsule->base;
    } else if (type == 1) {
        ret = capsule->tip;
    }

    return ret;
}

float geometry_capsule_get_radius(const CapsuleCollider3D_t * capsule)
{
    if (!capsule) return 0.0f;
    return capsule->radius;
}

/* A simple struct defining a sphere
    I don't even understand why they didn't provide a builtin type
*/
struct SphereCollider
{
    Vector3 center;
    float radius;  
};

SphereCollider_t * geometry_sphere_alloc()
{
    SphereCollider_t * ret = malloc(sizeof(SphereCollider_t));
    if (!ret) {
        TraceLog(LOG_FATAL, "Failed to allocate sphere object");
        return NULL;
    } else {
        memset(ret, 0, sizeof(SphereCollider_t));
    }

    return ret;
}

void geometry_sphere_destroy(SphereCollider_t * sphere)
{
    if (sphere != NULL) {
        free(sphere);
    }
    sphere = NULL;
}

Vector3 geometry_sphere_get_center(const SphereCollider_t * sphere)
{
    if (!sphere) return Vector3Zero();
    return sphere->center;
}

float geometry_sphere_get_radius(const SphereCollider_t * sphere)
{
    if (!sphere) return 0.0f;
    return sphere->radius;
}

void geometry_capsule_set_coord(CapsuleCollider3D_t * capsule, int type, Vector3 coord)
{
    if (!capsule) return;
    if (type == 0)  capsule->base = coord;
    if (type == 1)  capsule->tip = coord;
}

void geometry_capsule_set_radius(CapsuleCollider3D_t * capsule, float rad)
{
    if (!capsule) return;
    capsule->radius = rad;
}

/*
Function to calculate the projection of a point to a line segment
*/
Vector3 geometry_point_to_segment_projection(Vector3 point, Vector3 tip1, Vector3 tip2)
{
    // call A(x0, y0, z0) and B(x1, y1, z1)
    // direction vector: AB-> = B - A = (x1 - x0, y1 - y0, z1 - z0);
    // Need to check division by zero here
    // Damn it, should I change this file to C++ to use ... only overload operator ??!!
    Vector3 dir_vec = Vector3Subtract(tip2, tip1);
    if (FloatEquals(dir_vec.x, 0.0f) && FloatEquals(dir_vec.y, 0.0f) && FloatEquals(dir_vec.z, 0.0f)) {
        // handle exception or something else here
    }

    // calc A to P: AP-> = P - A
    Vector3 point_to_tip = Vector3Subtract(point, tip1);

    // calc parameter "t" of the straight line that contains the segment
    // projection C(t) = A + t.AB
    Vector3 ret = Vector3Zero();
    float t = Vector3DotProduct(point_to_tip, dir_vec) / Vector3DotProduct(dir_vec, dir_vec);
    t = fmaxf(0.0f, fminf(1.0f, t));
    ret = Vector3Add(tip1, Vector3Scale(dir_vec, t));

    return ret;
}

/*
Shortest Distance between two Line Segments algorithm
- Segment S1 from P0 to P1 -> L1(s) = P0 + s . u (with 0 <= s <= 1, u = P1 - P0)
- Segment S2 from Q0 to Q1 -> L2(s) = Q0 + t . v (with 0 <= t <= 1, v = Q1 - Q0)
Need to find vector W(s, t) = L1(s) - L2(t) with s, t in [0, 1] 
*/
Vector3 geometry_closest_distance_seg2seg(Vector3 Abase, Vector3 Atip, Vector3 Bbase, Vector3 Btip)
{
    /* 1. system of linear equations
    // W(s, t) . u = 0
    // W(s, t) . v = 0

    It becomes:
    // a . s - b . t = -d
    // b . s - c . t = -e
    */
    Vector3 ret = Vector3Zero();
    Vector3 w0 = Vector3Subtract(Bbase, Abase);
    Vector3 u  = Vector3Subtract(Atip, Abase);
    Vector3 v  = Vector3Subtract(Btip, Bbase);
    float a = Vector3DotProduct(u, u);
    float b = Vector3DotProduct(u, v);
    float c = Vector3DotProduct(v, v);
    float d = Vector3DotProduct(u, w0);
    float e = Vector3DotProduct(v, w0);
    
    float D = a * c - b * b;
    float s, t, s_nom, t_nom, t_denom;

    /* 2. Find the nearest point on an infinite line */
    if (D <= FLT_EPSILON) {
        // parallel
        s_nom = 0.0f;
        D = 1.0f;
        t_nom = e;
        t_denom = c;
    } else {
        s_nom = b * e - c * d;
        t_nom = a * e - b * d;
        t_denom = D;
    }

    /* 3. Clamping, hmm I will see, whether there's another way to shrink these messes */
    if (s_nom < 0.0f) {
        s = 0.0f;
        t_nom = e;
        t_denom = c;
    } else if (s_nom > D) {
        s = 1.0f;
        t_nom = e + b;
        t_denom = c;
    } else {
        s = s_nom / D;
    }

    // Fixed s, calculate t
    if (t_nom < 0.0f) {
        t = 0.0f;
        s = fmaxf(0.0f, fminf(1.0f, - d / a));
    } else if (t_nom > t_denom) {
        t = 1.0f;
        s = fmaxf(0.0f, fminf(1.0f, (b - d) / a));
    } else {
        t = t_nom / t_denom;
    }

    Vector3 Pmin = Vector3Add(Abase, Vector3Scale(u, s));
    Vector3 Qmin = Vector3Add(Bbase, Vector3Scale(v, t));
    ret = Vector3Subtract(Pmin, Qmin);
    return ret;
}

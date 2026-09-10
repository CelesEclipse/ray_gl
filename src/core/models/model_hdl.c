#include "model_hdl.h"
#include "raylib.h"
#include "raymath.h"

BoundingBox model_get_scaled_bbox(Model model, Vector3 position, float scale, float rotation_y)
{
    BoundingBox raw = GetModelBoundingBox(model);

    raw.min = Vector3Scale(raw.min, scale);
    raw.max = Vector3Scale(raw.max, scale);

    // rotation will break simple AABB, consider later
    Vector3 size = Vector3Subtract(raw.max, raw.min);
    Vector3 center = Vector3Add(position, Vector3Scale(Vector3Add(raw.min, raw.max), rotation_y));

    return (BoundingBox){
        .min = Vector3Subtract(center, Vector3Scale(size, rotation_y)),
        .max = Vector3Add(center, Vector3Scale(size, rotation_y))
    };
}

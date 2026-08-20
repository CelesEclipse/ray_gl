#pragma once

#include <raylib.h>

Vector3 collision_resolve_aabb(BoundingBox b1, BoundingBox b2, Vector3 movement);

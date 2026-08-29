#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "utils.h"

#define MAX_STATE_SIZE      20

const char * state_to_string(int value)
{
    static char res[MAX_STATE_SIZE];
    size_t dest_size = sizeof(res);

    switch (value) {
    case 0:
        strncpy(res, "IDLE", dest_size - 1);
        res[dest_size - 1] = '\0';
        break;
    case 1:
        strncpy(res, "MOVING", dest_size - 1);
        res[dest_size - 1] = '\0';
        break;
    case 2:
        strncpy(res, "ATTACKING", dest_size - 1);
        res[dest_size - 1] = '\0';
        break;
    case 3:
        strncpy(res, "DEAD", dest_size - 1);
        res[dest_size - 1] = '\0';
        break;
    default:
        break;
    }
    return res;
}

float random_float(float min, float max)
{
    float scale = (float)rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

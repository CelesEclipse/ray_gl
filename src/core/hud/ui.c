#include <raylib.h>
#include "ui.h"

void display_hp_bar(int x, int y, int width, int height, float hp, float max_hp)
{
    if (max_hp == 0) return;

    Color color;
    float ratio = hp / max_hp;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;
    
    if (ratio >= 0.6) {
        color = GREEN;
    } else if (ratio > 0.3 && ratio < 0.6) {
        color = GOLD;
    } else {
        color = RED;
    } 

    DrawRectangle(x, y, width, height, DARKGRAY);
    DrawRectangle(x, y, (int)(width * ratio), height, color);
    DrawRectangleLines(x, y, width, height, BLACK);
}

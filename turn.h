#ifndef TURN_H
#define TURN_H

#include "global.h"
#include "motor.h"

void Spin45(bool wait = true)
{
    moveRel(M_TURN, 70, -45 * ratio[M_TURN], wait);
}

// 转盘转 n 个 90 度。overshoot > 0 时先多转 overshoot 度再回退 (见 global.h 中 TURN_OVERSHOOT)
void Spin(int n, int overshoot = TURN_OVERSHOOT, bool wait = true)
{
    int16_t degree = -n * (90 * ratio[M_TURN]);
    if (n == 0 || overshoot <= 0)
    {
        moveRel(M_TURN, 100, degree, wait);
        return;
    }

    int16_t extra = degree > 0 ? overshoot : -overshoot;
    moveRel(M_TURN, 100, degree + extra);
    moveRel(M_TURN, 100, -extra, wait);
}
void Show()
{
    moveRel(M_TURN, 50, 4 * (90 * ratio[M_TURN]));
}
#endif
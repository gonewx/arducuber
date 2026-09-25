#ifndef MOTOR_H
#define MOTOR_H

#include <util/atomic.h>

#include "global.h"

BricktronicsMotor motors[] = {
    BricktronicsMotor(BricktronicsMegashield::MOTOR_1),
    BricktronicsMotor(BricktronicsMegashield::MOTOR_2),
    BricktronicsMotor(BricktronicsMegashield::MOTOR_3)};

bool isOverwrite(int32_t a, int32_t b)
{
    if (b > 0)
    {
        if (a > INT32_MAX - b)
        {
            return true;
        }
    }
    if (b < 0)
    {
        if (a < INT32_MIN - b)
        {
            return true;
        }
    }
    return false;
}

// 设置目标位置。positions[] 为 32 位, 在 8 位 AVR 上写入不是原子的,
// 而定时器中断会读取它并调用 update(), 所以写入期间必须屏蔽中断
void setTarget(int m, int32_t pos)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        positions[m] = pos;
        motors[m].goToPosition(pos);
    }
}

uint16_t motorTimeouts = 0;

// 等待电机连续 10ms 停在目标附近; 超时返回 false, 避免魔方卡住时程序挂死
bool waitForArrival(int m, unsigned long timeoutMs = MOTOR_WAIT_TIMEOUT_MS)
{
    unsigned long startMs = millis();
    int factor = 0;
    while (factor < 10)
    {
        if (abs(positions[m] - motors[m].getPosition()) < 5)
        {
            factor++;
        }
        else
        {
            factor = 0;
        }
        if (millis() - startMs > timeoutMs)
        {
            motorTimeouts++;
            Serial.print(F("waitForArrival timeout m:"));
            Serial.print(m);
            Serial.print(F(" target:"));
            Serial.print(positions[m]);
            Serial.print(F(" real:"));
            Serial.println(motors[m].getPosition());
            return false;
        }
        delay(1);
    }
    return true;
}

bool busy(int m)
{
    return !motors[m].settledAtPosition(positions[m]);
}

// 绝对位移将忽略 power 参数的符号
void moveAbs(int m, int power, int16_t degree, bool wait = true)
{
#ifdef DEBUG
    Serial.print(F("moveAbs m:"));
    Serial.print(m);
    Serial.print(F(" power:"));
    Serial.print(power);
    Serial.print(F(" degree:"));
    Serial.print(degree);
#endif
    // BricktronicsMotor 旋转一圈为 720度， 转化为 BricktronicsMotor 的;
    // BricktronicsMotor 旋转旋转方向与EV3相反, 需要取反

    int32_t pos = int32_t(abs(degree)) * 2;
    //正向
    if ((power > 0 && degree > 0) || (power < 0 && degree < 0))
    {
        pos = -pos;
    }

    motors[m]._pid.SetOutputLimits(-abs(power) * MAX_M_POWER / 100, abs(power) * MAX_M_POWER / 100);
#ifdef DEBUG

    Serial.print(F(" before:"));
    Serial.print(positions[m]);
    Serial.print(F(" after:"));
    Serial.print(pos);
#endif

    setTarget(m, pos);
    if (wait)
    {
        waitForArrival(m);

    }
#ifdef DEBUG

    Serial.print(F(" real:"));
    Serial.println(motors[m].getPosition());
#endif
}

void moveRel(int m, int power, int16_t degree, bool wait = true)
{
#ifdef DEBUG
    Serial.print(F("moveRel m:"));
    Serial.print(m);
    Serial.print(F(" power:"));
    Serial.print(power);
    Serial.print(F(" degree:"));
    Serial.print(degree);
#endif

    // BricktronicsMotor 旋转一圈为 720度， 转化为 BricktronicsMotor 的;
    // BricktronicsMotor 旋转旋转方向与EV3相反, 需要取反

    int32_t pos = abs(degree) * 2;
    //正向
    if ((power > 0 && degree > 0) || (power < 0 && degree < 0))
    {
        pos = -pos;
    }

    motors[m]._pid.SetOutputLimits(-abs(power) * MAX_M_POWER / 100, abs(power) * MAX_M_POWER / 100);

#ifdef DEBUG
    Serial.print(F(" before:"));
    Serial.print(positions[m]);
    Serial.print(F(" :"));
    Serial.print(motors[m].getPosition());
#endif

    bool overflow = isOverwrite(positions[m], pos);
    if (!overflow)
    {
        pos += positions[m];
    }
#ifdef DEBUG
    Serial.print(F(" after:"));
    Serial.print(pos);
#endif
    if (overflow)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            setTarget(m, pos);
            // 编码器写入内部会重新开中断, 必须放在最后
            motors[m].setPosition(0);
        }
    }
    else
    {
        setTarget(m, pos);
    }
    if (wait)
    {
        waitForArrival(m);
    }
#ifdef DEBUG
    Serial.print(F(" :"));
    Serial.println(motors[m].getPosition());
#endif
}

void move(int m, int power)
{
    power = -power;

    motors[m].setFixedDrive(power * MAX_M_POWER / 100);
}

void hold(int m)
{
    motors[m].hold();
}
void brake(int m)
{
    motors[m].brake();
}
void endstop(int m, int power, int wait = 200)
{
    // BricktronicsMotor 旋转旋转方向与EV3相反, 需要取反
    power = -power;

    int32_t p0, p1;
    motors[m].setFixedDrive(power * MAX_M_POWER / 100);

    p1 = motors[m].getPosition();

    while (true)
    {
        delay(wait);
        p0 = p1;
        p1 = motors[m].getPosition();

        if (power < 0)
        {
            if (p0 <= p1)
            {
                break;
            }
        }
        else
        {
            if (p0 >= p1)
            {
                break;
            }
        }
    };
    int32_t stopped = motors[m].getPosition();
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        positions[m] = stopped;
    }

    Serial.println(F(" "));
}

void reset(int m)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        positions[m] = 0;
        // 编码器写入内部会重新开中断, 必须放在最后
        motors[m].setPosition(0);
    }
}

int32_t getPosition(int m)
{
    // BricktronicsMotor 旋转一圈为 720度， EV3 一圏为 360 度;
    // BricktronicsMotor 旋转旋转方向与EV3相反, 需要取反
    return -motors[m].getPosition() / 2;
}

int32_t getTargetPosition(int m)
{
    // BricktronicsMotor 旋转一圈为 720度， EV3 一圏为 360 度;
    // BricktronicsMotor 旋转旋转方向与EV3相反, 需要取反
    return -positions[m] / 2;
}

#endif
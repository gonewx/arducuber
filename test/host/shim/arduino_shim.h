// 在 PC 上编译固件中与硬件无关的部分 (颜色识别、校验、求解) 所需的最小 Arduino 兼容层
#ifndef ARDUINO_SHIM_H
#define ARDUINO_SHIM_H

#include <stdint.h>
#include <stdlib.h>

typedef uint8_t byte;

#define F(x) (x)

struct HostSerial
{
    template <class T>
    void print(T) {}
    template <class T>
    void println(T) {}
    void println() {}
};
static HostSerial Serial;

#endif

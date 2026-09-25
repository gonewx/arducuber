#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>

// #define DEBUG

//-----------------------------------------------------------------------------
// Background task and routines to use color sensor as flashing light
//-----------------------------------------------------------------------------

#define S_CLR IN_3

#define CLR_R 0
#define CLR_B 1
#define CLR_O 2
#define CLR_G 3
#define CLR_W 4
#define CLR_Y 5

#define L_DEL 1000

// tcs34725 two 
#define T_SCNT -675
#define T_SEDG -575
#define T_SCNR -545

#define M_TURN 0
#define M_TILT 1
#define M_SCAN 2
#define MAX_M_POWER 255

#define P_LOW 160
#define P_HIGH 180

#define M_WAIT_MUL 4

// 等待电机到位的超时时间 (ms)。超时后不再无限等待, 避免魔方卡住时程序挂死
#define MOTOR_WAIT_TIMEOUT_MS 5000

// 扫描臂到位后, 读取颜色前的稳定等待 (ms)
#define SCAN_SETTLE_MS 100

// 每个色块采样次数: 3 = 读 3 次各通道取中值以抑制噪声; 1 = 单次读取 (与旧版本相同)
#define SCAN_SAMPLES 3

// 拧层时的过冲角度 (电机角度, 已含齿轮比)。先多转该角度再回退, 用于克服魔方阻力、
// 让层转到位。MindCuber 原版为 T_OVR = 57。0 = 关闭 (与旧版本行为相同), 需在实机上调试
#define TURN_OVERSHOOT 0

#define NFACE 6 // number of faces in cube
#define POS(FF, OO) (((FF)*8) + (OO))

#define makeLong(hi, low) (((long)hi) << 16 | (low))
#define highWord(w) ((w) >> 16)
#define lowWord(w) ((w)&0xFFFF)

const int EEPROM_ID_ADDR = 0;
const int EEPROM_DATA_ADDR = 1;
const uint8_t EEPROM_ID = 0x99;

uint8_t white_rgb[3] = {255, 255, 255};

// 齿轮比
const int ratio[] = {3, 1, 3};

// 电机目标位置, 定时器中断中会读取, 主循环写入时须使用 setTarget() 保证原子性
volatile int32_t positions[] = {0, 0, 0};

bool scanOK = true;

unsigned long start = 0;

#endif

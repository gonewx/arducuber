#ifndef DEVICE_H
#define DEVICE_H

#include <BricktronicsMegashield.h>
#include <BricktronicsMotor.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_TCS34725.h>

#include "btn.h"
#include "color.h"
#include "dist.h"
#include "solver.h"
#include "validator.h"

Adafruit_TCS34725 colorSensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_60X);

CubeColors cubeColors = CubeColors();
Validator validator = Validator();
CubeSolver cubeSolver = CubeSolver();

SensorDist sensorDist = SensorDist();
Button btn = Button(BricktronicsMegashield::SENSOR_2);

// const int rs = 49, en = 48, d4 = 47, d5 = 46, d6 = 45, d7 = 44;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#endif
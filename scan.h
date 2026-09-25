#ifndef SCAN_H
#define SCAN_H

#include "global.h"
#include "motor.h"
#include "turn.h"
#include "tilt.h"
#include "color.h"

uint8_t rgb[3];

void ScanAway()
{
    moveAbs(M_SCAN, 100, -340);
}

void ScanCal()
{
    //根据实际测试，比 mindcuber的值略大
    endstop(M_SCAN, 70);

    moveRel(M_SCAN, 100, -100);
    endstop(M_SCAN, 50, 500);
    reset(M_SCAN);
    ScanAway();
}

void calibrateRGB()
{

    for (int i = 0; i < 3; i++)
    {
        if (rgb[i] > white_rgb[i])
        {
            rgb[i] = white_rgb[i];
        }
        rgb[i] = map(rgb[i], 0, white_rgb[i], 0, 255);
    }
}

static uint8_t median3(uint8_t a, uint8_t b, uint8_t c)
{
    if (a > b)
    {
        uint8_t t = a;
        a = b;
        b = t;
    }
    if (b > c)
        b = c;
    return a > b ? a : b;
}

// SCAN_SAMPLES 为 3 时连续读 3 次, 各通道取中值; 为 1 时单次读取 (与旧版本相同)
void readRGB(uint8_t *out)
{
    uint8_t samples[3][3];
    const uint8_t n = SCAN_SAMPLES >= 3 ? 3 : 1;
    for (uint8_t i = 0; i < n; i++)
    {
        float red, green, blue;
        colorSensor.getRGB(&red, &green, &blue);
        samples[0][i] = uint8_t(red);
        samples[1][i] = uint8_t(green);
        samples[2][i] = uint8_t(blue);
    }
    for (uint8_t ch = 0; ch < 3; ch++)
    {
        out[ch] = n == 3 ? median3(samples[ch][0], samples[ch][1], samples[ch][2]) : samples[ch][0];
    }
}

void ScanRGB(int face, int piece, uint8_t *rgb)
{
#ifdef DEBUG
    unsigned long start = millis();
    Serial.println();
    Serial.print(F(" scan pos before:"));
    Serial.print(getPosition(M_SCAN));
    Serial.print(F(" _pidOutput :"));
    Serial.print(motors[M_SCAN]._pidOutput);
    Serial.print(F(" turn pos:"));
    Serial.print(getPosition(M_TURN));
    Serial.println();
#endif

    readRGB(rgb);

    calibrateRGB();
#ifdef DEBUG

    Serial.print(F(" after :"));
    Serial.print(getPosition(M_SCAN));
    Serial.print(F(" _pidOutput :"));
    Serial.print(motors[M_SCAN]._pidOutput);
    Serial.print(F(" positions:"));
    Serial.print(positions[M_SCAN]);
    Serial.print(F(" turn pos:"));
    Serial.print(getPosition(M_TURN));
    Serial.print(F(" time:"));
    Serial.print(millis() - start);

    Serial.println();
#endif

    cubeColors.setRGB(face, piece, rgb);
}

void ScanPiece(int face, int piece)
{
#ifdef DEBUG
    Serial.print(F("ScanPiece; face:"));
    Serial.print(face);
    Serial.print(F(" piece:"));
    Serial.print(piece);

#endif
    if (scanOK)
    {
        ScanRGB(face, piece, rgb);
    }

#ifdef DEBUG

    cubeColors.print(face);

    Serial.println();
#endif
}
void ScanMiddle(int face)
{
#ifdef DEBUG
    Serial.print(F("ScanMiddle; face:"));
    Serial.println(face);
    Serial.println();

    start = millis();
#endif
    moveAbs(M_SCAN, 100, T_SCNT, false);
    waitForArrival(M_SCAN);
    delay(SCAN_SETTLE_MS);
    ScanPiece(face, 8);
}

void ScanCorner(int face, int piece)
{
    start = millis();
#ifdef DEBUG
    Serial.print(F("ScanCorner; face:"));
    Serial.print(face);
    Serial.print(F(" piece:"));
    Serial.print(piece);
    Serial.println();
#endif
    if (scanOK)
    {

        start = millis();
        moveAbs(M_SCAN, 100, T_SCNR, false);

        Spin45(false);
#ifdef DEBUG
        Serial.print(F("ScanCorner; wait scan"));
        Serial.println();
#endif
        waitForArrival(M_SCAN);
        waitForArrival(M_TURN);
        delay(SCAN_SETTLE_MS);
        ScanPiece(face, piece);
    }
    else
    {
        Spin45();
    }
}

void ScanEdge(int face, int piece)
{
#ifdef DEBUG
    start = millis();
    Serial.print(F("ScanEdge; face:"));
    Serial.print(face);
    Serial.print(F(" piece:"));
    Serial.println(piece);
    Serial.println();
#endif

    if (scanOK)
    {
        start = millis();
        moveAbs(M_SCAN, 100, T_SEDG, false);

        Spin45(false);
#ifdef DEBUG
        Serial.print(F("ScanEdge; wait scan"));
        Serial.println();
#endif

        waitForArrival(M_SCAN);
        waitForArrival(M_TURN);

        delay(SCAN_SETTLE_MS);
        ScanPiece(face, piece);
    }
    else
    {
        Spin45();
    }
}
void ScanFace(int face, int offset)
{
#ifdef DEBUG
    Serial.print(F("ScanFace; face:"));
    Serial.print(face);
    Serial.print(F(" offset:"));
    Serial.println(offset);
    Serial.println();
#endif

    TiltAway();

    do
    {
        scanOK = true;
        ScanMiddle(face);

        brake(M_TILT);

        start = millis();

        for (int i = 0; i < 4; i++)
        {
            int p = offset;
            ScanCorner(face, p);
            p++;
            ScanEdge(face, p);
            offset = (offset + 2) & 7;
        }
       

    } while (!scanOK);
#ifdef DEBUG

    Serial.println();
    Serial.println();
#endif
}

// NXT
void ScanCube()
{
    ScanFace(3, 2);
    cubeColors.print(3);
    ScanAway();
    Tilt(1);
    ScanFace(2, 2);
    cubeColors.print(2);

    ScanAway();
    Tilt(1);
    ScanFace(1, 2);
    cubeColors.print(1);

    ScanAway();
    Tilt(1);
    ScanFace(0, 2);
    cubeColors.print(0);

    ScanAway();
    Spin(1, 0);
    delay(50);
    Tilt(1);
    ScanFace(4, 6);
    cubeColors.print(4);

    ScanAway();
    Tilt(2);
    ScanFace(5, 2);
    cubeColors.print(5);

    ScanAway();

}

#endif
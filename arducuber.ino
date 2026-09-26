#include <EEPROM.h>
#include <avr/pgmspace.h>

#include "global.h"
#include "device.h"
#include "banner.h"
#include "tilt.h"
#include "scan.h"
#include "color.h"
#include "mcmoves.h"

void setup()
{
  // Be sure to set your serial console to 115200 baud
  Serial.begin(115200);

  lcd.init(); // initialize the lcd

  if (colorSensor.begin())
  {
    Serial.println(F("Found sensor"));
  }
  else
  {
    Serial.println(F("No Color Sensor found ... check your connections"));
    while (1)
      ; // halt!
  }

  btn.begin();

  // Initialize the motor connections
  motors[M_TURN].begin();
  motors[M_TILT].begin();
  motors[M_SCAN].begin();

  motors[M_SCAN].pidSetTunings(2.64, 23.6, 0.1207317073);

  initialize();

  OCR0A = 0x7F;
  TIMSK0 |= _BV(OCIE0A);
}

// Timer0 比较中断, 约每 1ms 触发一次; 每 50ms 为各电机执行一次 PID update()。
// 电机连续 5 个周期停在目标附近后暂停 update(), 以减少到位后的抖动。
ISR(TIMER0_COMPA_vect)
{
  static unsigned char count_ms = 0;
  static uint8_t stableFactor[3] = {0, 0, 0};

  if (++count_ms == 50)
  {
    for (uint8_t m = 0; m < 3; m++)
    {
      if (abs(positions[m] - motors[m].getPosition()) < 5)
      {
        if (stableFactor[m] < 5)
          stableFactor[m]++;
      }
      else
      {
        stableFactor[m] = 0;
      }

      if (stableFactor[m] < 5)
      {
        motors[m].update();
      }
    }

    count_ms = 0;
  }
}

void initialize()
{
  motors[M_TURN].coast();
  motors[M_TILT].coast();
  motors[M_SCAN].coast();

  reset(M_TURN);
  reset(M_TILT);
  reset(M_SCAN);

  motors[M_TURN]._pid.SetOutputLimits(-MAX_M_POWER, MAX_M_POWER);
  motors[M_TILT]._pid.SetOutputLimits(-MAX_M_POWER, MAX_M_POWER);
  motors[M_SCAN]._pid.SetOutputLimits(-MAX_M_POWER, MAX_M_POWER);

  scanOK = true;
  motorTimeouts = 0;

  readWhiteRGB();
}

void init_cube(byte *cube)
{
  int o = 0;
  for (byte f = 0; f < NFACE; f++)
    for (int i = 0; i < 8; i++)
      cube[o++] = f;
}

void readWhiteRGB()
{
  // Read the white calibration value from EEPROM
  byte id = EEPROM.read(EEPROM_ID_ADDR);
  if (id == EEPROM_ID)
  {
    for (int i = 0; i < 3; i++)
    {
      white_rgb[i] = EEPROM.read(EEPROM_DATA_ADDR + i + 1);
    }
  }
}

void writeWhiteRGB()
{
  // Read the white calibration value from EEPROM
  EEPROM.write(EEPROM_ID_ADDR, EEPROM_ID);

  for (int i = 0; i < 3; i++)
  {
    EEPROM.write(EEPROM_DATA_ADDR + i + 1, white_rgb[i]);
  }
}

bool CubeSense()
{
  float cm = sensorDist.get();

  return cm > 0 && cm < 16;
}

void CubeInsert()
{
  int count = 0;
  while (count < 150)
  {
    count++;
    if (!CubeSense())
    {
      count = 0;
    }
    if (btn.isPressed(BTN_LEFT))
    {
      moveRel(M_TURN, 75, 2 * ratio[M_TURN], true);
    }
    if (btn.isPressed(BTN_RIGHT))
    {
      moveRel(M_TURN, 75, -2 * ratio[M_TURN], true);
    }
    if (btn.isPressed(BTN_UP))
    {
      calibrate_white();
      count = 0;
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Insert cube...");
    }
    delay(10);
  }
}

void CubeRemove()
{
  int count = 0;
  while (count < 150)
  {
    count++;
    if (CubeSense())
      count = 0;
    delay(10);
  }
}

bool Solve(byte *cube)
{
  bool solved = false;
  int pieces_valid = 0;
  for (int tries = 0; !solved && tries < 3; tries++)
  {
    ScanCube();

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Processing...");
    Serial.println(F("Processing..."));

    // 依次尝试 red/orange 的各种区分方式, 共 COLOR_STRATEGIES 种
    // (旧版本循环 12 次, 但 sort_colors 使用 t % 6, 后 6 次与前 6 次完全重复)
    for (int i = 0; i < COLOR_STRATEGIES; i++)
    {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Determine Clr...");

      Serial.println(F("determine_colors..."));
      cubeColors.determine_colors(cube, i);

      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Check valid ...");

      Serial.println(F("valid_pieces..."));
      bool is_valid = validator.valid_pieces(cube);

      if (is_valid)
      {
        pieces_valid++;
        Serial.println(F("is_valid"));
        cubeColors.print();

        // display_cube(cube);
        if (cubeSolver.solve(cube))
        {
          solved = true;
          break;
        }
      }
      else
      {
        Serial.print(F(" not valid: "));
        Serial.println(i);
        cubeColors.print();
      }
    }

    if (!solved)
    {
      cubeColors.print();
    }
  }
  if (solved)
  {
    ScanAway();

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Solving ...");

    Serial.println(F(" Solving.."));

    cubeSolver.uc = CUBE_LEFT;
    cubeSolver.fc = CUBE_DOWN;

    for (int i = 0; i < cubeSolver.solve_n; i++)
    {
      int fi = cubeSolver.solve_fce[i];
      int ri = cubeSolver.solve_rot[i];
      int fo = opposite[fi];
      int rn = 0;
      for (int j = i + 1; rn == 0 && j < cubeSolver.solve_n; j++)
      {
        if (cubeSolver.solve_fce[j] != fo)
          rn = cubeSolver.solve_rot[j];
      }

      lcd.setCursor(0, 1);
      lcd.print("Move ");
      lcd.print(i + 1);
      lcd.print(" of ");
      lcd.print(cubeSolver.solve_n);

      cubeSolver.manipulate(cube, fi, ri, rn);
    }
  }
  else
  {

    Serial.print(F("Scan error..."));
    delay(500);

    cubeColors.print();
    if (pieces_valid > 1)
    {
      Serial.print(F("Cube "));
      Serial.println(pieces_valid);
      delay(1000);
    }
    else
    {
      Serial.println(pieces_valid);
    }
  }

  ScanAway();
  TiltAway();

  if (motorTimeouts > 0)
  {
    Serial.print(F("Motor timeouts: "));
    Serial.println(motorTimeouts);
  }

  return solved;
}

// 等待魔方连续被检测到约 1.5 秒; 超时返回 false
bool waitCubePresent(unsigned long timeoutMs)
{
  unsigned long startMs = millis();
  int count = 0;
  while (count < 100)
  {
    count = CubeSense() ? count + 1 : 0;
    if (millis() - startMs > timeoutMs)
      return false;
    delay(10);
  }
  return true;
}

// 白平衡校准: 在 "Insert cube..." 界面按 "上" 键进入。
// 放入白色中心朝上的魔方 -> 扫描臂移到中心读取 16 次取平均 -> 存入 EEPROM -> 取出魔方。
// (旧版本要求读数超过 250 才结束, 白色实际约 170~240, 所以会一直卡在 "Cal White...")
void calibrate_white()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cal White:");
  lcd.setCursor(0, 1);
  lcd.print("White center up");
  Serial.println(F("Calibrate white"));

  while (btn.isPressed(BTN_UP))
    delay(10);

  if (!waitCubePresent(20000))
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Cal canceled");
    delay(1500);
    return;
  }

  moveAbs(M_SCAN, 100, T_SCNT, false);
  waitForArrival(M_SCAN);
  delay(SCAN_SETTLE_MS);

  uint16_t sum[3] = {0, 0, 0};
  const uint8_t n = 16;
  for (uint8_t i = 0; i < n; i++)
  {
    uint8_t sample[3];
    readRGB(sample);
    for (uint8_t ch = 0; ch < 3; ch++)
      sum[ch] += sample[ch];
  }
  ScanAway();

  uint8_t w[3];
  for (uint8_t ch = 0; ch < 3; ch++)
    w[ch] = sum[ch] / n;

  Serial.print(F("White R: "));
  Serial.print(w[0]);
  Serial.print(F(" G: "));
  Serial.print(w[1]);
  Serial.print(F(" B: "));
  Serial.println(w[2]);

  lcd.clear();
  lcd.setCursor(0, 0);
  if (w[0] < 20 || w[1] < 20 || w[2] < 20)
  {
    // 读数过暗, 多半是传感器没有对准魔方, 保留原来的校准值
    lcd.print("Cal failed");
  }
  else
  {
    for (uint8_t ch = 0; ch < 3; ch++)
      white_rgb[ch] = w[ch];
    writeWhiteRGB();
    lcd.print("Cal White OK");
  }
  lcd.setCursor(0, 1);
  lcd.print(w[0]);
  lcd.print(' ');
  lcd.print(w[1]);
  lcd.print(' ');
  lcd.print(w[2]);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Remove cube...");
  CubeRemove();
}

void loop()
{
  while (!btn.isPressed(BTN_CONFIRM))
  {
    delay(10);
  }

  byte cube[NFACE * 8];
  initialize();

  banner();

  init_cube(cube);

  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 1);
  lcd.print("Reset scan...");

  ScanCal();
  delay(500);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Reset tilt...");

  TiltCal();
  delay(500);

  // moveAbs(M_SCAN, 100, T_SCNT);
  // moveAbs(M_SCAN, 100, T_SEDG);
  // moveAbs(M_SCAN, 100, T_SCNR);

  // while (true)
  // {
  //   Tilt();
  //   TiltAway();
  //   Spin(1);
  // }

  while (true)
  {
    ScanAway();
    TiltAway();
    delay(500);

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Insert cube...");

    Serial.println(F("CubeInsert:"));
    CubeInsert();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scanning...");

    Solve(cube);

    Show();

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Remove cube...");

    Serial.println(F("CubeRemove:"));
    CubeRemove();
  }
}

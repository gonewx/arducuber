// 马达诊断: 依次让 Megashield 的 Motor1/2/3 正转、反转各一小段,
// 读取编码器变化, 判断是马达没转 (供电/驱动/线缆) 还是编码器没读到。
// 串口监视器 115200。按 Mega 上的复位键可重新测试。
#include <BricktronicsMegashield.h>
#include <BricktronicsMotor.h>

BricktronicsMotor motors[] = {
    BricktronicsMotor(BricktronicsMegashield::MOTOR_1),
    BricktronicsMotor(BricktronicsMegashield::MOTOR_2),
    BricktronicsMotor(BricktronicsMegashield::MOTOR_3)};

const char *const names[] = {"Motor1 (turntable)", "Motor2 (tilt)", "Motor3 (scan)"};

const int16_t DRIVE = 130;  // 约 50% 功率, 时间很短, 扫描臂/翻转臂碰到限位也无妨
const int PULSE_MS = 250;

void testMotor(int m)
{
  BricktronicsMotor &motor = motors[m];
  Serial.print(F("== "));
  Serial.print(names[m]);
  Serial.println(F(" : forward then backward, watch the motor =="));

  motor.setPosition(0);
  motor.setFixedDrive(DRIVE);
  delay(PULSE_MS);
  motor.brake();
  delay(300);
  int32_t p1 = motor.getPosition();

  motor.setFixedDrive(-DRIVE);
  delay(PULSE_MS);
  motor.brake();
  delay(300);
  int32_t p2 = motor.getPosition();
  motor.coast();

  Serial.print(F("   encoder after forward: "));
  Serial.print(p1);
  Serial.print(F("   after backward: "));
  Serial.println(p2);

  int32_t moved1 = abs(p1);
  int32_t moved2 = abs(p2 - p1);
  if (moved1 > 20 && moved2 > 20)
    Serial.println(F("   OK: motor turns and encoder counts."));
  else if (moved1 <= 2 && moved2 <= 2)
    Serial.println(F("   NO ENCODER CHANGE: if the motor did NOT move -> check battery/DC jack power, cable, port; "
                     "if it DID move -> encoder signal not reaching the Mega (cable/port)."));
  else
    Serial.println(F("   WEAK/PARTIAL: motor barely moved -> low motor voltage or mechanism jammed."));
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println(F("Motor test start (motors are powered from the Megashield DC jack, NOT from USB)"));
  for (int m = 0; m < 3; m++)
  {
    motors[m].begin();
  }
  for (int m = 0; m < 3; m++)
  {
    testMotor(m);
    delay(500);
  }
  Serial.println(F("Done. Press the Mega reset button to run again."));
}

void loop()
{
}

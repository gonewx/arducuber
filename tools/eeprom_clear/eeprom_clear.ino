// 清空 EEPROM (把所有字节写成 0), 用于清除白平衡校准值。
// 运行完成后板载 LED (13 脚) 常亮, 然后重新上传主程序即可。
#include <EEPROM.h>

void setup()
{
  for (unsigned int i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
}

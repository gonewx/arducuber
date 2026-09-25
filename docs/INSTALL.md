# 安装与运行

本文基于 **Arduino IDE 2.3.10**（2026 年 6 月发布，内置 Arduino CLI 1.5.1）编写。命令行方式适用于 **arduino-cli 1.5.x**。

- [1. 准备工作](#1-准备工作)
- [2. 方式一：Arduino IDE](#2-方式一arduino-ide)
- [3. 方式二：arduino-cli 命令行](#3-方式二arduino-cli-命令行)
- [4. 首次运行](#4-首次运行)
- [5. 日常使用流程](#5-日常使用流程)
- [6. 调参与测试](#6-调参与测试)
- [7. 常见问题](#7-常见问题)

## 1. 准备工作

### 硬件

器件清单、乐高搭建和传感器改造见 [readme](../readme.md)。接线速查：

| 设备 | 接到 | 说明 |
| --- | --- | --- |
| 转盘马达（底盘） | Megashield Motor1 | EV3 大马达 |
| 翻转马达 | Megashield Motor2 | EV3 大马达 |
| 扫描马达 | Megashield Motor3 | EV3 中马达 |
| HC-SR04 超声波 | Megashield Sensor1 | 跳线接 4、5 脚；接法见 readme |
| ADKeyboard 按键板 | Megashield Sensor2 | 跳线接 3、4 脚；接法见 readme |
| TCS34725 颜色传感器 | I2C（Mega 的 SDA=20，SCL=21） | 与 LCD 共用 I2C 总线 |
| LCD1602 (I2C) | I2C | 默认地址 0x27 |
| 电池（2 节 18650） | Megashield 的 DC 电源口 | 马达只由这里供电 |

> USB 线只能给主控供电，带不动马达。上传程序可以只接 USB，**运行时必须接电池**，而且电量要充足。

### 软件

| 组件 | 版本 / 来源 |
| --- | --- |
| Arduino IDE | 2.3.10 或更新，<https://www.arduino.cc/en/software> |
| 开发板支持 | Arduino AVR Boards（`arduino:avr`），通过开发板管理器安装 |
| Adafruit TCS34725 | 库管理器（会自动安装依赖 Adafruit BusIO） |
| LiquidCrystal I2C | 库管理器，作者 Frank de Brabander |
| HCSR04 ultrasonic sensor | 库管理器，作者 gamegine（**注意有同名的其他库**，见常见问题） |
| BricktronicsMegashield | GitHub：<https://github.com/wayneandlayne/BricktronicsMegashield> |
| BricktronicsMotor | GitHub：<https://github.com/wayneandlayne/BricktronicsMotor> |

EEPROM 库是 AVR 开发板支持包自带的，不需要另外安装。

## 2. 方式一：Arduino IDE

### 2.1 安装 IDE

从 <https://www.arduino.cc/en/software> 下载并安装 Arduino IDE 2.3.10 或更新版本。

- Windows：也可以从 Microsoft Store 安装。第一次连接开发板时，按提示安装驱动。
- Linux：使用 AppImage 版本时，需要把当前用户加入 `dialout` 组才能访问串口。执行 `sudo usermod -aG dialout $USER`，然后重新登录。

### 2.2 获取代码

```bash
git clone https://github.com/gonewx/arducuber.git
```

或者在 GitHub 页面上选择 **Code → Download ZIP** 并解压。

> Arduino 要求**文件夹名与 `.ino` 文件名一致**。ZIP 解压出来的文件夹叫 `arducuber-main`，请改名为 `arducuber`。否则 IDE 打开时会提示新建文件夹并移动文件。

### 2.3 安装开发板支持

1. 打开 **工具 → 开发板 → 开发板管理器**，搜索 `Arduino AVR Boards` 并安装最新版。
2. 选择 **工具 → 开发板 → Arduino AVR Boards → Arduino Mega or Mega 2560**。
3. 选择 **工具 → 处理器 → ATmega2560 (Mega 2560)**。

### 2.4 安装库管理器中的库

打开 **工具 → 管理库…**（或点左侧栏的书本图标），依次搜索并安装：

1. `Adafruit TCS34725`。弹出“是否安装依赖”时选择 **全部安装**，这样会一并装上 Adafruit BusIO。
2. `LiquidCrystal I2C`，选择作者为 **Frank de Brabander** 的那一个。
3. `HCSR04 ultrasonic sensor`，选择作者为 **gamegine** 的那一个。

### 2.5 安装 Bricktronics 库

这两个库不在库管理器中，需要手动安装：

1. 在 GitHub 上下载 ZIP（**Code → Download ZIP**）：
   - <https://github.com/wayneandlayne/BricktronicsMegashield>
   - <https://github.com/wayneandlayne/BricktronicsMotor>
2. 解压后把文件夹名里的 `-master` 去掉，分别改名为 `BricktronicsMegashield` 和 `BricktronicsMotor`。
3. 放到项目文件夹（sketchbook）下的 `libraries` 目录。项目文件夹的位置可以在 **文件 → 首选项 → 项目文件夹地址** 中查看，默认是：
   - Windows：`文档\Arduino\libraries`
   - macOS：`~/Documents/Arduino/libraries`
   - Linux：`~/Arduino/libraries`
4. 重启 IDE。

BricktronicsMotor 自带了 Encoder 和 PID 的实现，不需要另外安装这两个库。

### 2.6 编译、上传

1. 用 **文件 → 打开** 打开 `arducuber/arducuber.ino`。
2. 用 USB 线连接 Mega 2560，在 **工具 → 端口** 中选中它（Windows 上是 `COMx`，macOS 上是 `/dev/cu.usbmodem…`，Linux 上是 `/dev/ttyACM0`）。
3. 点击 **验证**（✓）进行编译。编译成功后，点击 **上传**（→）。
4. 打开 **工具 → 串口监视器**，把波特率设为 **115200**，就能看到运行日志。

## 3. 方式二：arduino-cli 命令行

### 3.1 安装 arduino-cli

| 系统 | 命令 |
| --- | --- |
| macOS | `brew install arduino-cli` |
| Windows | `winget install ArduinoSA.CLI` |
| Linux | `curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh \| sh`（会安装到 `./bin`，请把它加入 `PATH`） |

其他安装方式见 <https://arduino.github.io/arduino-cli/latest/installation/>。

### 3.2 使用脚本

仓库中的 `scripts/arduino.sh` 封装了常用步骤。它是 bash 脚本，Windows 上请使用 Git Bash 或 WSL 运行：

```bash
cd arducuber
scripts/arduino.sh setup              # 安装 AVR 核心和全部依赖库，只需执行一次
scripts/arduino.sh ports              # 查看开发板所在端口
scripts/arduino.sh upload /dev/ttyACM0
scripts/arduino.sh monitor /dev/ttyACM0
```

`setup` 会执行 `arduino-cli config set library.enable_unsafe_install true`，这是从 GitHub 地址安装 Bricktronics 库的前提。

### 3.3 手动命令

不使用脚本时，等价的命令如下：

```bash
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli lib install "Adafruit TCS34725" "LiquidCrystal I2C" "HCSR04 ultrasonic sensor"
arduino-cli config set library.enable_unsafe_install true
arduino-cli lib install --git-url https://github.com/wayneandlayne/BricktronicsMegashield.git \
                                  https://github.com/wayneandlayne/BricktronicsMotor.git

arduino-cli compile --fqbn arduino:avr:mega:cpu=atmega2560 .
arduino-cli upload  --fqbn arduino:avr:mega:cpu=atmega2560 -p /dev/ttyACM0 .
arduino-cli monitor -p /dev/ttyACM0 --config baudrate=115200
```

## 4. 首次运行

按键说明（ADKeyboard）：**确认** 用于启动，**左 / 右** 用于微调转盘位置，**上** 用于白平衡校准。

1. **接好电池，打开电源。** 程序启动时会检测颜色传感器。如果串口打印 `No Color Sensor found`，程序会停住，请检查 I2C 接线。
2. **按“确认”键。** LCD 背光亮起，机器人依次执行：
   - `Reset scan...`：扫描臂转到机械限位处校准；
   - `Reset tilt...`：翻转臂转到机械限位处校准。

   校准过程中马达会顶住限位一小段时间，这是正常现象。请不要用手阻挡机械结构。
3. **（可选）白平衡校准。** 先把魔方放进转盘，让**白色中心块朝上**。在扫描臂和翻转臂收回后、LCD 显示 `Insert cube...` 之前，**按住“上”键**。LCD 显示 `Cal White...`，完成后显示 `Cal White suc`。校准结果保存在 EEPROM 中，断电后仍然有效。每次开机最多校准一次。不做校准时，程序使用 EEPROM 中上次保存的值；从未校准过时，使用默认值 255。
4. **对准转盘。** LCD 显示 `Insert cube...` 时，用 **左 / 右** 键微调转盘，直到转盘与机身中轴线垂直。
5. **放入打乱的魔方。** 超声波传感器检测到魔方（16cm 以内）并持续约 2 秒后，自动开始工作。

## 5. 日常使用流程

| LCD 显示 | 机器人在做什么 | 你需要做什么 |
| --- | --- | --- |
| `Insert cube...` | 等待放入魔方 | 放入打乱的魔方 |
| `Scanning...` | 依次扫描 6 个面 | 等待 |
| `Determine Clr...` / `Check valid ...` | 识别颜色并校验，最多尝试 6 种红橙区分方式；失败会重新扫描，最多 3 轮 | 等待 |
| `Solving ...` + `Move i of n` | 按解法逐步拧动 | 等待 |
| `Remove cube...` | 还原完成（转盘会转一圈展示） | 取出魔方，再放入下一个 |

如果 3 轮扫描都识别失败，机器人会直接转到 `Remove cube...`，串口打印 `Scan error...`。可以检查光线条件、做一次白平衡校准，或者参考 readme 调整颜色传感器的位置。

## 6. 调参与测试

- 需要在实机上调整的参数集中在 `global.h`，说明见 [readme 的可调参数一节](../readme.md#可调参数)。
- 在 `global.h` 中取消 `// #define DEBUG` 的注释，可以在串口看到每一步马达动作和颜色读数的详细日志。
- 颜色识别和求解逻辑可以在电脑上测试，不需要硬件（需要 g++ 和 make）：

  ```bash
  make test
  ```

## 7. 常见问题

**编译报错 `no matching function for call to 'HCSR04::HCSR04(int&, int&)'` 或 `'class HCSR04' has no member named 'dist'`**
装错了同名的 HCSR04 库（例如另一个作者的 v2 版本，它的 API 不同）。请在库管理器中卸载它，改装作者为 **gamegine** 的 `HCSR04 ultrasonic sensor`。

**编译报错 `BricktronicsMotor.h: No such file or directory`**
Bricktronics 库没有放在正确的位置。确认 `libraries/BricktronicsMotor/BricktronicsMotor.h` 这个路径存在（注意文件夹不能多套一层），然后重启 IDE。

**上传失败 / 找不到端口**
- 确认已选择 **Arduino Mega or Mega 2560** 开发板，以及正确的端口。
- 关闭占用串口的程序，比如另一个串口监视器。
- Linux 上需要把用户加入 `dialout` 组（见 2.1 节）。

**LCD 不亮或只显示方块**
- 程序在按下“确认”键之后才打开背光，这是正常行为。
- 调节 LCD 背面 I2C 转接板上的对比度电位器。
- 有些模块的地址是 `0x3F`，请修改 `device.h` 中的 `LiquidCrystal_I2C lcd(0x27,16,2)`。

**按键没有反应或按错键**
ADKeyboard 各按键对应的电压范围定义在 `btn.h` 的 `keysRange` 中，不同批次的模块可能有差异。可以临时在 `loop()` 中用 `Serial.println(analogRead(...))` 打印读数，然后调整这些范围。

**马达无力、翻转不到位、成功率随电量下降**
Megashield 使用的 L293D 驱动芯片本身会有 2~3V 压降，所以电池电压对表现影响很大。请先充满电池再试。

**串口出现 `waitForArrival timeout`**
马达在 `MOTOR_WAIT_TIMEOUT_MS`（默认 5 秒）内没有到达目标位置，通常是魔方卡住或机械结构受阻。程序不会停住，会继续执行，但本次还原很可能失败。请检查魔方是否松紧合适、转盘是否对正。

**卡在 `Cal White...` 不结束**
白平衡校准会一直读取，直到任一颜色通道读数超过 250。如果白色面没有正对传感器，或者传感器离魔方太远，校准就不会结束。遇到这种情况，请按 Mega 上的复位键重启，跳过校准或调整位置后重试。

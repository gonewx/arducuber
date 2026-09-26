#!/usr/bin/env bash
# 使用 arduino-cli 安装依赖、编译、上传、打开串口监视器
#
#   scripts/arduino.sh setup                 安装 AVR 核心和全部依赖库 (只需执行一次)
#   scripts/arduino.sh compile               编译
#   scripts/arduino.sh upload <端口>         编译并上传, 例如 /dev/ttyACM0 或 COM3
#   scripts/arduino.sh monitor <端口>        打开串口监视器 (115200)
#   scripts/arduino.sh clear-eeprom <端口>   上传清空 EEPROM 的程序 (清除白平衡校准值)
#   scripts/arduino.sh motor-test <端口>     上传马达诊断程序并打开串口监视器
#   scripts/arduino.sh ports                 列出已连接的开发板
set -euo pipefail

FQBN="arduino:avr:mega:cpu=atmega2560"
SKETCH_DIR="$(cd "$(dirname "$0")/.." && pwd)"

need_cli() {
    if ! command -v arduino-cli >/dev/null 2>&1; then
        echo "未找到 arduino-cli, 安装方法见 readme 中的 \"命令行安装\"" >&2
        exit 1
    fi
}

need_port() {
    if [ -z "${1:-}" ]; then
        echo "请指定端口, 可用 '$0 ports' 查看" >&2
        exit 1
    fi
}

cmd="${1:-}"
case "$cmd" in
setup)
    need_cli
    arduino-cli config init >/dev/null 2>&1 || true
    arduino-cli core update-index
    arduino-cli core install arduino:avr
    # Library Manager 中的库 (Adafruit BusIO 会作为依赖自动安装)
    arduino-cli lib install "Adafruit TCS34725" "LiquidCrystal I2C" "HCSR04 ultrasonic sensor"
    # Bricktronics 库不在 Library Manager 中, 需从 GitHub 安装
    arduino-cli config set library.enable_unsafe_install true
    arduino-cli lib install --git-url \
        https://github.com/wayneandlayne/BricktronicsMegashield.git \
        https://github.com/wayneandlayne/BricktronicsMotor.git \
        https://github.com/wayneandlayne/BricktronicsButton.git
    ;;
compile)
    need_cli
    arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"
    ;;
upload)
    need_cli
    need_port "${2:-}"
    arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"
    arduino-cli upload --fqbn "$FQBN" -p "$2" "$SKETCH_DIR"
    ;;
monitor)
    need_cli
    need_port "${2:-}"
    arduino-cli monitor -p "$2" --config baudrate=115200
    ;;
clear-eeprom)
    need_cli
    need_port "${2:-}"
    arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR/tools/eeprom_clear"
    arduino-cli upload --fqbn "$FQBN" -p "$2" "$SKETCH_DIR/tools/eeprom_clear"
    echo "EEPROM 已清空 (板载 LED 常亮)。请重新执行: $0 upload $2"
    ;;
motor-test)
    need_cli
    need_port "${2:-}"
    arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR/tools/motor_test"
    arduino-cli upload --fqbn "$FQBN" -p "$2" "$SKETCH_DIR/tools/motor_test"
    echo "诊断程序已上传, 按 Ctrl+C 退出串口监视器; 测试完请重新执行: $0 upload $2"
    arduino-cli monitor -p "$2" --config baudrate=115200
    ;;
ports)
    need_cli
    arduino-cli board list
    ;;
*)
    sed -n '2,11p' "$0"
    exit 1
    ;;
esac

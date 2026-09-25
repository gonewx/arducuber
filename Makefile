# 主机端测试: 在 PC 上编译并运行颜色识别 / 校验 / 求解逻辑的单元测试
# 用法: make test

CXX ?= g++
CXXFLAGS ?= -std=gnu++11 -O2 -Wall -Wextra -Werror=return-type
HOST_FLAGS = -Itest/host/shim -include test/host/shim/arduino_shim.h
BUILD = build

HEADERS = global.h color.h validator.h solver.h mcmoves.h fakedata.h

.PHONY: test clean

test: $(BUILD)/host_tests
	./$(BUILD)/host_tests

$(BUILD)/host_tests: test/host/test_main.cpp $(HEADERS) test/host/shim/arduino_shim.h
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(HOST_FLAGS) -o $@ $<

clean:
	rm -rf $(BUILD)

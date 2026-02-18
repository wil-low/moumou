# sudo snap install arduino-cli

ARDUINO_PATH=$(HOME)/.arduino15/packages/arduino
SRC_PATH=moumou

DEPS= \
	$(SRC_PATH)/core.cpp \
	$(SRC_PATH)/core.h \
	$(SRC_PATH)/ai.cpp \
	$(SRC_PATH)/ai.h \
	$(SRC_PATH)/ui.cpp \
	$(SRC_PATH)/ui.h \
	$(SRC_PATH)/config.h \
	$(SRC_PATH)/moumou.ino

all: bin/MOUMOU.HEX

clean:
	rm -rf bin/MOUMOU.HEX build/*

build/moumou.ino.hex: $(DEPS)
	clear
	mkdir -p build bin
	arduino-cli compile \
		--fqbn ATTinyCore-1.5.2:avr:attinyx5micr \
		--libraries "$(HOME)/Arduino/libraries" \
		--build-path build \
		--warnings all \
		$(SRC_PATH)

bin/MOUMOU.HEX: build/moumou.ino.hex
	cp build/moumou.ino.hex bin/MOUMOU.HEX

.PHONY: all clean

# sudo snap install arduino-cli

ARDUINO_PATH=$(HOME)/.arduino15/packages/arduino
SIM_PATH=$(HOME)/Arduino/Simbuino4Web-patched/hex
SRC_PATH=moumou

DEPS= \
	$(SRC_PATH)/core.cpp \
	$(SRC_PATH)/core.h \
	$(SRC_PATH)/ai.cpp \
	$(SRC_PATH)/ai.h \
	$(SRC_PATH)/config.h \
	$(SRC_PATH)/moumou.ino \
	$(SRC_PATH)/card.cpp \
	$(SRC_PATH)/card.h \
	$(SRC_PATH)/pile.cpp \
	$(SRC_PATH)/pile.h

all: bin/MOUMOU.HEX

clean:
	rm -rf bin/MOUMOU.HEX build/*

build/moumou.ino.hex: $(DEPS)
	clear
	mkdir -p build bin
	arduino-cli compile \
		--fqbn arduino:avr:uno \
		--libraries "$(HOME)/Arduino/libraries" \
		--build-path build \
		$(SRC_PATH)

bin/MOUMOU.HEX: build/moumou.ino.hex
	cp build/moumou.ino.hex bin/MOUMOU.HEX
	cp build/moumou.ino.hex $(SIM_PATH)/

sim: bin/MOUMOU.HEX stop_sim
	cd $(HOME)/Arduino/Simbuino4Web-patched && bash start.sh &
	xdg-open http://localhost:8000/Index.html?hex=hex/moumou.ino.hex &

stop_sim:
	fuser -k 8000/tcp || true

.PHONY: all clean sim stop_sim

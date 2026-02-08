# OS Detection
UNAME_S := $(shell uname -s)

# Default Port Settings (macOS vs Linux/WSL)
ifeq ($(UNAME_S),Darwin)
    # macOS
    DXL_PORT ?= /dev/tty.usbserial-FTAO9VZP
    PICO_PORT ?= /dev/tty.usbmodemF412FA77615C2
else
    # Linux / WSL
    DXL_PORT ?= /dev/ttyUSB0
    PICO_PORT ?= /dev/ttyACM0
endif

# Python Settings
PYTHON_CMD ?= python3
VENV_DIR = venv
PYTHON = $(VENV_DIR)/bin/python
PIP = $(PYTHON) -m pip

.PHONY: all run install flash clean help setup_venv

# デフォルトターゲット
all: help

# ヘルプ表示
help:
	@echo "使用可能なコマンド:"
	@echo "  make run      : メイン制御プログラムを実行します (venvを使用)"
	@echo "  make install  : venvを作成しライブラリをインストールします"
	@echo "  make clean    : 一時ファイルとvenvを削除します"

# venv作成
setup_venv:
	@if ! command -v $(PYTHON_CMD) > /dev/null; then \
		echo "Error: $(PYTHON_CMD) が見つかりません。Pythonをインストールしてください。"; \
		exit 1; \
	fi
	$(PYTHON_CMD) -m venv $(VENV_DIR)

# 依存ライブラリインストール
install: setup_venv
	$(PIP) install --upgrade pip
	$(PIP) install -r requirements.txt

# テスト実行 (自動制御)
test:
	@if [ ! -d "$(VENV_DIR)" ]; then echo "venvが見つかりません。'make install'を実行してください。"; exit 1; fi
	$(PYTHON) src/main.py test

# キーボード操作モード
keyboard:
	@if [ ! -d "$(VENV_DIR)" ]; then echo "venvが見つかりません。'make install'を実行してください。"; exit 1; fi
	$(PYTHON) src/main.py keyboard

# ファームウェア書き込み (Arduino CLI使用)
flash:
	arduino-cli compile --fqbn rp2040:rp2040:rpipico firmware/pico_motor_driver/pico_motor_driver.ino
	@echo "コンパイル完了。書き込みには 'arduino-cli upload -p <PORT> ...' を実行してください。"

clean:
	find . -type d -name "__pycache__" -exec rm -rf {} +
	rm -rf firmware/pico_motor_driver/build
	rm -rf $(VENV_DIR)

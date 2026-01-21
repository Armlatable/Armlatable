# proto_pico Makefile

# Virtual Environment Settings
VENV_DIR = venv
PYTHON = $(VENV_DIR)/bin/python
PIP = $(VENV_DIR)/bin/pip

.PHONY: all run install flash clean help setup_venv test keyboard

# デフォルトターゲット
all: help

# ヘルプ表示
help:
	@echo "proto_pico - Raspberry Pi Pico 版"
	@echo ""
	@echo "使用可能なコマンド:"
	@echo "  make install  : venvを作成しライブラリをインストールします"
	@echo "  make test     : 自動テストモードを実行します"
	@echo "  make keyboard : キーボード操作モードを実行します"
	@echo "  make flash    : ファームウェアをコンパイルします"
	@echo "  make clean    : 一時ファイルとvenvを削除します"

# venv作成
setup_venv:
	python3 -m venv $(VENV_DIR)

# 依存ライブラリインストール
install: setup_venv
	$(PIP) install --upgrade pip
	$(PIP) install -r requirements.txt

# テスト実行 (自動制御)
test:
	@if [ ! -d "$(VENV_DIR)" ]; then echo "venvが見つかりません。'make install'を実行してください。"; exit 1; fi
	cd src && ../$(PYTHON) main.py test

# キーボード操作モード
keyboard:
	@if [ ! -d "$(VENV_DIR)" ]; then echo "venvが見つかりません。'make install'を実行してください。"; exit 1; fi
	cd src && ../$(PYTHON) main.py keyboard

# ファームウェア書き込み (Arduino CLI使用)
flash:
	arduino-cli compile --fqbn rp2040:rp2040:rpipico firmware/pico_motor_driver/pico_motor_driver.ino
	@echo "コンパイル完了。書き込みには 'arduino-cli upload -p <PORT> ...' を実行してください。"

clean:
	find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	rm -rf firmware/pico_motor_driver/build
	rm -rf $(VENV_DIR)

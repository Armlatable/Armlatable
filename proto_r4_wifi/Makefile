# proto_r4_wifi Makefile

# Virtual Environment Settings
VENV_DIR = venv
PYTHON = $(VENV_DIR)/bin/python
PIP = $(VENV_DIR)/bin/pip

# Arduino Settings
FQBN = arduino:renesas_uno:unor4wifi
SKETCH = firmware/r4_motor_driver/r4_motor_driver.ino
# 自動検出を試みる (接続されている最初のR4 WiFi)
PORT := $(shell arduino-cli board list | grep "UNO R4 WiFi" | head -n1 | awk '{print $$1}')

# 書き込み完了マーカー
FLASH_MARKER = .flash_complete

.PHONY: all run install flash force-flash clean help setup_venv test keyboard

# デフォルトターゲット: 必要ならFlashしてRun
all: $(FLASH_MARKER) run

# ヘルプ表示
help:
	@echo "proto_r4_wifi - Arduino R4 WiFi 版"
	@echo "  make          : (更新があれば)書き込み -> キーボード操作"
	@echo "  make run      : キーボード操作のみ"
	@echo "  make flash    : (更新があれば)ファームウェア書き込み"
	@echo "  make force-flash : 強制的に書き込み"
	@echo "  make install  : 環境構築"
	@echo "  make clean    : 掃除"

# venv作成
setup_venv:
	if [ ! -d "$(VENV_DIR)" ]; then python3 -m venv $(VENV_DIR); fi

# 依存ライブラリインストール
install: setup_venv
	$(PIP) install --upgrade pip
	$(PIP) install -r requirements.txt

# キーボード操作モード (Default run)
run: keyboard

keyboard:
	@if [ ! -d "$(VENV_DIR)" ]; then echo "venvが見つかりません。'make install'を実行してください。"; exit 1; fi
	cd src && ../$(PYTHON) main.py keyboard

test:
	@if [ ! -d "$(VENV_DIR)" ]; then echo "venvが見つかりません。'make install'を実行してください。"; exit 1; fi
	cd src && ../$(PYTHON) main.py test

# ソースコードが更新されていたら書き込み
$(FLASH_MARKER): $(SKETCH)
	@$(MAKE) force-flash

# エイリアス: ユーザーが明示的に make flash した時もチェック
flash: $(FLASH_MARKER)

# 強制書き込み
force-flash:
	@echo "Compiling..."
	arduino-cli compile --fqbn $(FQBN) $(SKETCH)
	@if [ -z "$(PORT)" ]; then \
		echo "Arduino UNO R4 WiFi が見つかりません。接続を確認するか、'make flash PORT=/dev/...' のようにポートを指定してください。"; \
		exit 1; \
	fi
	@echo "Uploading to $(PORT)..."
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(SKETCH)
	@touch $(FLASH_MARKER)

clean:
	find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	rm -rf firmware/r4_motor_driver/build
	rm -rf $(VENV_DIR)
	rm -f $(FLASH_MARKER)

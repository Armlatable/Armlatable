import serial
import time
import sys
import threading

# ポート設定 (環境に合わせて変更してください)
PORT = "/dev/tty.usbmodemF412FA77615C2"
BAUD = 115200

def read_thread(ser):
    while True:
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"[Arduino]: {line}")
        except:
            break

def test_drive():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Connected to {PORT}")

        # 受信スレッド開始
        t = threading.Thread(target=read_thread, args=(ser,), daemon=True)
        t.start()

        # Arduinoのリセット
        ser.dtr = False
        time.sleep(0.1)
        ser.dtr = True

        print("Waiting for Arduino ready...")
        time.sleep(2)

        print("--- Commands ---")
        print(" DC Motor: M,<id>,<pwm>  (e.g., M,0,100)")
        print(" Dynamixel: D,<id>,<pos> (e.g., D,1,2048)")
        print(" Stop: S")
        print(" Quit: Q")

        while True:
            cmd = input("Cmd > ")
            if cmd.lower() == 'q':
                break
            if cmd:
                ser.write((cmd + '\n').encode('utf-8'))

        ser.close()
        print("Test Complete.")

    except serial.SerialException as e:
        print(f"Serial Error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        PORT = sys.argv[1]

    test_drive()

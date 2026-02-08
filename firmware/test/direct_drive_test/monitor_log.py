import serial
import time
import sys

PORT = "/dev/tty.usbmodemF412FA77615C2"
BAUD = 115200

def monitor():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Connected to {PORT}")

        # Reset Arduino via DTR
        ser.dtr = False
        time.sleep(0.1)
        ser.dtr = True
        print("Reset signal sent via DTR.")

        print("Waiting for logs... (Press Ctrl+C to stop)")

        while True:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"[Arduino]: {line}")
            except KeyboardInterrupt:
                break
            except Exception:
                pass
        ser.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    monitor()

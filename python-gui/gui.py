import tkinter as tk
from tkinter import ttk
import serial
import serial.tools.list_ports
import sys

BAUD_RATE = 115200
NUM_SERVOS = 22

# =========================
# AUTO SERIAL DETECTION
# =========================
ports = list(serial.tools.list_ports.comports())

if not ports:
    print("❌ No serial device found.")
    sys.exit()

SERIAL_PORT = None

for port in ports:
    if "ttyUSB" in port.device or "ttyACM" in port.device:
        SERIAL_PORT = port.device
        break

if SERIAL_PORT is None:
    print("❌ No Arduino serial port detected.")
    sys.exit()

print(f"✅ Using port: {SERIAL_PORT}")

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print("❌ Serial Error:", e)
    sys.exit()

# =========================
# SEND FUNCTION
# =========================
def send_angle(servo_id, value):
    angle = int(float(value))
    data = f"{servo_id},{angle}\n"
    ser.write(data.encode())
    angle_labels[servo_id].config(text=str(angle))

# =========================
# GUI SETUP
# =========================
root = tk.Tk()
root.title("🕷 Hexapod 22 Servo Controller")
root.geometry("1200x700")

main_frame = ttk.Frame(root, padding=10)
main_frame.pack(fill="both", expand=True)

sliders = []
angle_labels = []

for i in range(NUM_SERVOS):
    frame = ttk.LabelFrame(main_frame, text=f"Servo {i}", padding=10)
    frame.grid(row=i // 4, column=i % 4, padx=10, pady=10)

    slider = tk.Scale(
        frame,
        from_=0,
        to=180,
        orient=tk.HORIZONTAL,
        length=200,
        command=lambda val, sid=i: send_angle(sid, val)
    )
    slider.set(90)
    slider.pack()

    angle_label = ttk.Label(frame, text="90", font=("Arial", 10, "bold"))
    angle_label.pack()

    sliders.append(slider)
    angle_labels.append(angle_label)

# =========================
# RESET BUTTON
# =========================
def reset_all():
    for i in range(NUM_SERVOS):
        sliders[i].set(90)
        send_angle(i, 90)

reset_button = ttk.Button(root, text="Reset All to 90°", command=reset_all)
reset_button.pack(pady=10)

# =========================
# CLEAN EXIT
# =========================
def on_close():
    ser.close()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_close)

root.mainloop()

import os
import time
import csv
import serial # pip3 install pyserial
import keyboard # pip3 install keyboard
import enquiries # pip3 install enquiries
import numpy as np

arduino = serial.Serial(port="/dev/ttyACM0")
pause = True
record = False
GRAVITY_CONTANT = 9.82

def read_data():
    try:
        [time,weight] = arduino.readline().decode().split(',')# time in ms, weight in g
        time = int(time)
        weight = int(weight)
        force = int(weight) * GRAVITY_CONTANT # F = m*g
        return time,force,weight
    except:
        pass
def write_data(x):
    arduino.write(bytes(x,'utf-8'))
    time.sleep(0.05)

def toggle_pause():
    global pause
    pause = not pause
    if pause:
        print("pausing ...")
    else:
        print("resuming ...")

def toggle_record(filename):
    global record
    record = not record
    if record:
        print("recording ...")
    else:
        print("recording stopped ...")
        print("saving data to " + filename)
        exit()
    

timestamp = time.strftime("%Y%m%d-%H%M%S")
options = ['Road-tire friction', 'Tire stiffness', 'Coaster test']
choice = enquiries.choose('Experiment to run: ', options)
filename = ""
if choice == 'Road-tire friction':
    filename = "road_tire_friction_"
elif choice == 'Tire stiffness':
    filename = "tire_stiffness_"
elif choice == "Coaster test":
    filename = "coaster_test_"
    print("Cannot perform coaster test in this file")
    exit()

filename += timestamp + ".csv"
file_path = os.path.abspath(os.path.join(os.path.realpath(__file__),"..","..","data",filename))

print("")
print("")
print("THE FOLLOWING KEYBOARD COMMANDS ARE AVAILABLE")
print("")
print("Press:       0           ->         tare")
print("             UP          ->         increase scaling factor")
print("             DOWN        ->         decrease scaling factor") 
print("             SPACEBAR    ->         pause/resume program")
print("             r           ->         start/stop data recording")


with open(file_path, 'w', encoding="UTF8", newline="") as file:
    header = ["time[ms]", "force[N]", "weight[kg]"]
    writer = csv.writer(file)
    writer.writerow(header)
    while True:
        if not pause:
            data = read_data()
            if record:
                writer.writerow(data)
                print("Recording", data)
            else:
                print(data)
        if keyboard.is_pressed('SPACE'):  # if key 'q' is pressed 
            toggle_pause()
            time.sleep(0.5)
        if not pause:
            if keyboard.is_pressed('UP'):
                write_data('+')
            if keyboard.is_pressed('DOWN'):
                write_data("-")
            if keyboard.is_pressed('0'):
                write_data("0")
            if keyboard.is_pressed('r'):
                toggle_record(filename)
                time.sleep(0.5)
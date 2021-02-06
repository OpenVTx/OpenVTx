
import serial
import time

x = [" ", " ", " ", " "]
freq = "0"
Vpd = "0"
PWM = "0"
avPower = "0"   
maxPower = "0"   

#IMRC Power Meter
serialPowMeter = serial.Serial(
    port='COM4',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=2
)

#Betaflight passthrough
serialFC = serial.Serial(
    port='COM6',
    baudrate=4800,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=2
)

serialFC.flushInput()
serialPowMeter.flushInput()
serialFC.flushOutput()
serialPowMeter.flushOutput()

#print(serialPowMeter.name)
#print(serialFC.name)

f = open("log.csv", "a")

print("freq,PWM,VpdSetPoint,Vpd,avPower,maxPower")
f.write("freq,PWM,VpdSetPoint,Vpd,avPower,maxPower\n")

while True:
    serialInput = serialFC.readline().rstrip() # rstrip to remove return at end of string
    
    if serialInput:
        x = serialInput.split(",")
        freq = x[0].rstrip() 
            
        if freq == "5600":
            serialPowMeter.write("F7\n") # Set Freq 5600
        elif  freq == "5650":
            serialPowMeter.write("F8\n") # Set Freq 5650
        elif  freq == "5700":
            serialPowMeter.write("F9\n") # Set Freq 5700
        elif  freq == "5750":
            serialPowMeter.write("F10\n") # Set Freq 5750
        elif  freq == "5800":
            serialPowMeter.write("F11\n") # Set Freq 5800
        elif  freq == "5850":
            serialPowMeter.write("F12\n") # Set Freq 5850
        elif  freq == "5900":
            serialPowMeter.write("F13\n") # Set Freq 5900
        elif  freq == "5950":
            serialPowMeter.write("F14\n") # Set Freq 5950
        elif  freq == "6000":
            serialPowMeter.write("F15\n") # Set Freq 6000
            
        # read reply from setting freq
        serialPowMeter.readline()
        serialPowMeter.readline()
            
        serialPowMeter.write("E\n") # Average Power
        maxPower = serialPowMeter.readline().rstrip()
        serialPowMeter.readline()
            
        serialPowMeter.write("D\n") # Average Power
        avPower = serialPowMeter.readline().rstrip()
        serialPowMeter.readline()
            
        print(serialInput + "," + avPower + "," + maxPower)
        f.write(serialInput + "," + avPower + "," + maxPower + "\n")

        
serialPowMeter.close()
serialFC.close()   
f.close()
        
        
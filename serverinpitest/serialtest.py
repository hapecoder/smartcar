import serial
import time
serial=serial.Serial("/dev/ttyUSB0",9600,timeout=0.1)
pwm_1=15555
pwm_2=151
send_data="t"+str(pwm_1)+" "+str(pwm_2)+"\r\n"

serial.write(send_data.encode('gbk','ignore'))
            
for i in range(10):
    rcv=serial.readline()
    rcv=rcv.decode('gbk','ignore') 

    time.sleep(0.3)
    if rcv:
        if rcv[0]=="r":
                rcv=rcv.split()
                rcv[0]=rcv[0].replace("r", "")
        
                
                if rcv[0].replace(".", "").isdigit() and rcv[1].replace(".", "").isdigit() : 
                        speed=rcv[0];distance=rcv[1]
                        print(speed,distance)


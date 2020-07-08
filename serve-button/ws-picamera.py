# encoding:utf-8
from threading import Lock
from flask import Flask, render_template, session, request, \
    copy_current_request_context, redirect, url_for, jsonify, Response
from flask_socketio import SocketIO, emit, join_room, leave_room, disconnect
from user import read_user
import os
import base64
import picamera
from PIL import Image
from io import BytesIO
import serial
import sys
from engineio.payload import Payload
from flask_cors import CORS


Payload.max_decode_packets = 50
sys.setrecursionlimit(1000000)





# try:
#     serial=serial.Serial("/dev/ttyAMA0",9600,timeout=0.1) 
#     serial_ready=1
# except:
#     serial_ready=0
try:
    serial=serial.Serial("/dev/ttyUSB0",9600,timeout=0.1) 
    serial_ready=1
except:
    serial_ready=0


os.environ["GEVENT_SUPPORT"] = "True"
async_mode =None
app = Flask(__name__, static_folder='', static_url_path='', template_folder='')
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=async_mode)
# thread = None
# thread_lock = Lock()
CORS(app)
camera=picamera.PiCamera() 
camera.start_preview()
camera.resolution = (320, 240) 

f_flag=0;r_flag=0;l_flag=0;b_flag=0
speed = 1
distance=1
pwm_1=1500
pwm_2=1500
count_flag=False
camera_flag=0
speedlimit=1600
mode=3

def control(status):
    global pwm_1,pwm_2,f_flag,r_flag,l_flag,b_flag  #电机，舵机
    if status == "front":
        pwm_1=1550
        f_flag=1
    elif status == "left":
        pwm_2=1700
        l_flag=1
    elif status == "right":
        pwm_2=1300
        r_flag=1
    elif status == "back":
        pwm_1=1450
        b_flag=1
    elif status == "stop":
        pwm_1=1500
        pwm_2=1500
        f_flag=0;r_flag=0;l_flag=0;b_flag=0
    

def stop(status):
    global pwm_1,pwm_2,f_flag,r_flag,l_flag,b_flag
    if status == "front":
        f_flag=0
        pwm_1=1500

    elif status == "left":
        l_flag=0
        pwm_2=1500

    elif status == "right":
        r_flag=0
        pwm_2=1500

    elif status == "back":
        b_flag=0
        pwm_1=1500


    


def background_thread():  #后台线程 发送数据
    while True:
        global pwm_1,pwm_2,f_flag,r_flag,l_flag,b_flag,speedlimit,speed,distance,camera_flag 
        socketio.sleep(0.1)  # 秒
        
        if count_flag==True:
            if f_flag ==1:
                if pwm_1<int(speedlimit):
                    pwm_1+=1
            if l_flag ==1:
                if pwm_2<2300:
                    pwm_2+=10    
            if r_flag ==1:
                if pwm_2>800:
                    pwm_2-=10
            if b_flag ==1:
                if pwm_1>1300:
                    pwm_1-=1

        if(serial_ready==1): 
            if mode==3:
                send_data=str(pwm_1)+" "+str(pwm_2)+"\r\n"
                serial.write(send_data.encode(encoding='gbk'))
            rcv=serial.readline()
            rcv=rcv.decode(encoding='gbk') 
            if rcv:
                if rcv[0]=="r":
                    rcv=rcv.split()
                    rcv[0]=rcv[0].replace("r", "")
                    if rcv[0].replace(".", "").isdigit() and rcv[1].replace(".", "").isdigit() : 
                        speed=rcv[0];distance=rcv[1]

        socketio.emit('my_response', {
                      'speed': speed, 'distance': distance,'pwm_1':pwm_1,"pwm_2":pwm_2}, namespace='/test')

        if(camera_flag==1):
            try:
                    output_buffer = BytesIO() #创建一个BytesIO
                    camera.capture(output_buffer , format='jpeg') 
                    byte_data = output_buffer.getvalue() #在内存中读取
                    image_stream = base64.b64encode(byte_data).decode()
                    socketio.emit('c_response', image_stream, namespace='/test')  
                    socketio.sleep(0.05)  # 秒
                    camera_flag=0
            except:
                    pass
 
        

@app.route('/')
def hello_world():
    return render_template("login.html")


@app.route('/login', methods=['POST'])
def index():
    username = request.form['username']
    password = request.form['password']
    if read_user(username, password):
        # return redirect(url_for('gotoindex'))
        return render_template("index.html")
    else:
        return render_template("error.html")



@socketio.on('connect', namespace='/test')
def mtest_connect():
    print('websocket connected')
    global thread
    thread = socketio.start_background_task(background_thread)


@socketio.on('disconnect', namespace='/test')
def mtest_disconnect():
    print('Client disconnected')


@socketio.on('my_event', namespace='/test')
def mtest_message(message):
    global camera_flag,count_flag,speedlimit
    # session['receive_count'] = session.get('receive_count', 0) + 1
    # print(message)

    if message == 'pic':
        # with open('1.png', 'rb') as f:
        #         image = f.read()
        #         image_stream = base64.b64encode(image).decode()
        #         emit('c_response', image_stream)  # 返回json格式)
        camera_flag=1
        

        
        
    elif len(message)==1:
        speedlimit=message['speed']
    else:
        adss=message['commander']
        count_flag=message['count']
        print(count_flag)
        if count_flag==True: #按下
            control(adss)
            print("按下了按钮:"+adss)
        else:             #抬起
            stop(adss) 
        # print(message['data'])  #dict类型
    
@socketio.on('my_mode', namespace='/test')
def modeset(message):
    global mode
    try:
        if message['mode'] ==1:
            mode=1
        elif message['mode']==2:
            mode=2
        else:
            mode=3
        if(serial_ready==1):     
            send_data=str(mode)+"\r\n"
            serial.write(send_data.encode(encoding='gbk'))    
    except:
        pass

if __name__ == '__main__':

    socketio.run(app, host='0.0.0.0',debug=True)




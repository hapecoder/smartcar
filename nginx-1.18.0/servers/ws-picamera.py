# encoding:utf-8
from threading import Lock
from flask import Flask, render_template, session, request, \
    copy_current_request_context, redirect, url_for, jsonify, Response
from flask_socketio import SocketIO, emit, join_room, leave_room, disconnect
from user import read_user
import os
import base64
# from gevent import monkey
# from cv2 import cv2
from PIL import Image
from io import BytesIO
import picamera
# import serial
# serial=serial.Serial("/dev/ttyAMA0",9600,timeout=0.1) 

os.environ["GEVENT_SUPPORT"] = "True"
async_mode = None
app = Flask(__name__, static_folder='', static_url_path='', template_folder='')
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=async_mode)
# thread = None
# thread_lock = Lock()

cmdd = 'vue -V'
f_flag=0;r_flag=0;l_flag=0;b_flag=0
speed = 1
distance=1
pwm_1=1500
pwm_2=1500
count_flag=False
#cap = cv2.VideoCapture(0)
camera_flag=0
speedlimit=1600
runningtime=0
camera = picamera.PiCamera()
camera.resolution = (320, 240)
#camera.framerate = 16

def control(status):
    global pwm_1,pwm_2,f_flag,r_flag,l_flag,b_flag  #电机，舵机
    if status == "front":
        pwm_1=1540
        f_flag=1
       # os.system(cmdd) #执行cmd 命令
    elif status == "left":
        pwm_2=1600
        l_flag=1
    elif status == "right":
        pwm_2=1400
        r_flag=1
    elif status == "back":
        pwm_1=1450
        b_flag=1
    elif status == "stop":
        pwm_1=1500
        pwm_2=1500

    

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
        global pwm_1,pwm_2,f_flag,r_flag,l_flag,b_flag,speedlimit,runningtime,camera_flag  #电机，舵机
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

        socketio.emit('my_response', {
                      'speed': speed, 'distance': distance,'pwm_1':pwm_1,"pwm_2":pwm_2}, namespace='/test')

        if(camera_flag==1):
            
            
                #socketio.sleep(2)
            output_buffer = BytesIO() #创建一个BytesIO
            camera.capture(output_buffer , format='jpeg')
                
            byte_data = output_buffer.getvalue() #在内存中读取
                #显示图像窗口在树莓派的屏幕上
            image_stream = base64.b64encode(byte_data).decode()
            socketio.emit('c_response', image_stream, namespace='/test')  # 返回json格式)\
            socketio.sleep(0.05)  # 秒
            runningtime+=1
            
        if(runningtime==1):
            print(runningtime)
            runningtime=0
            camera_flag=0
        # send_data=str(pwm_1)+" "str(pwm_2)+"\r\n"
        # serial.write(send_data.encode(encoding='gbk'))
        # rcv=serial.readline()
        # rcv=rcv.decode(encoding='gbk') 
        # print(rcv)
       

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
    global camera_flag,count_flag,speedlimit,camera
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
    


if __name__ == '__main__':
    # monkey.patch_all()
    socketio.run(app, host='0.0.0.0')

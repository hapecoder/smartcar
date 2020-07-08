# encoding:utf-8
from threading import Lock
from flask import Flask, render_template, session, request, \
    copy_current_request_context, redirect, url_for, jsonify, Response
from flask_socketio import SocketIO, emit, join_room, leave_room, disconnect
from user import read_user
import os
import base64
from cv2 import cv2
from PIL import Image
from io import BytesIO
from gevent import monkey
from gevent.pywsgi import WSGIServer
import sys
sys.setrecursionlimit(10000000)

import serial
try:
    serial=serial.Serial("COM3",9600,timeout=0.1) 
except:
    pass

os.environ["GEVENT_SUPPORT"] = "True"
async_mode = 'gevent'
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
cap = cv2.VideoCapture(0)
camera_flag=0
speedlimit=1600
mode=3
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
        global pwm_1,pwm_2,f_flag,r_flag,l_flag,b_flag,speedlimit,speed,distance,camera_flag  #电机，舵机
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
        try:
            if mode==3:
                send_data=str(pwm_1)+" "+str(pwm_2)+"\r\n"
                serial.write(send_data.encode(encoding='gbk'))
            rcv=serial.readline()
            rcv=rcv.decode(encoding='gbk') 
            rcv=rcv.split()
            speed=rcv[0];distance=rcv[1]
        except:
            pass

        socketio.emit('my_response', {
                      'speed': speed, 'distance': distance,'pwm_1':pwm_1,"pwm_2":pwm_2}, namespace='/test')

        if(camera_flag):
            if( cap.isOpened() ):
                #USB摄像头工作时,读取一帧图像
                ret, frame = cap.read()
                frame  = cv2.resize(frame,(640,640))             
                b,g,r = cv2.split(frame)
                frame_rgb = cv2.merge([r,g,b])    
                img = Image.fromarray(frame_rgb) #将每一帧转为Image
                output_buffer = BytesIO() #创建一个BytesIO
                img.save(output_buffer, format='JPEG') #写入output_buffer
                byte_data = output_buffer.getvalue() #在内存中读取
                #显示图像窗口在树莓派的屏幕上
                image_stream = base64.b64encode(byte_data).decode()
                socketio.emit('c_response', image_stream, namespace='/test')  # 返回json格式)\
                socketio.sleep(0.05)  # 秒
                camera_flag=0
        
       

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

@app.route('/cont', methods=['GET'])
def cont():
    return render_template("cont.html")

@app.route('/index', methods=['GET'])
def ind():
    return render_template("index.html")
        

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
        camera_flag=1
    if len(message)==1:
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
        send_data=str(mode)+"\r\n"
        serial.write(send_data.encode(encoding='gbk'))    
    except:
        pass

@socketio.on('my_cont', namespace='/test')
def contset(message):
    global pwm_1,pwm_2
    pwm_1=int(int(message['y'])/375*(1500-speedlimit)+1500)
    pwm_2=int(-int(message['x'])/375*500+1500)


if __name__ == '__main__':
    monkey.patch_all()
    http_server = WSGIServer(('0.0.0.0', int(5000)), app)
    http_server.serve_forever()
   # socketio.run(app, host='0.0.0.0', debug=True)

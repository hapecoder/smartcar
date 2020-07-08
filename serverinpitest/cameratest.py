import time
import picamera
from io import BytesIO
with picamera.PiCamera() as camera:
    camera.resolution = (1024, 768)
    camera.start_preview()
    #摄像头预热2秒
    time.sleep(2)
    output_buffer = BytesIO() #创建一个BytesIO
    camera.capture(output_buffer , format='jpeg')

print(output_buffer)
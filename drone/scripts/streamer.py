import base64
import cv2
import zmq
import picamera
import time
import np

context = zmq.Context()
footage_socket = context.socket(zmq.PUSH)
footage_socket.connect('tcp://192.168.0.8:5555')

#setup camera
camera = picamera.PiCamera()
camera.resolution = (640, 480)
camera.framerate = 24
time.sleep(2)
output = np.empty((480, 640, 3), dtype=np.uint8)


#while True:
    #camera.capture(output, 'rgb')
    #print("caputed")


#while True:
#    try:
#        print("about to caputre")
#        camera.capture(output, 'rgb')
#        print("captured")
#        #grabbed, frame = camera.read()  # grab the current frame
#        #frame = cv2.resize(frame, (640, 480))  # resize the frame
#        encoded, buffer = cv2.imencode('.jpg', output)
#        jpg_as_text = base64.b64encode(buffer)
#        footage_socket.send_string(jpg_as_text)
#        print("sent")
#        #if(grabbed):
#        #    cv2.imshow("Stream",frame)
#        #    cv2.waitKey(1)
#
#    except KeyboardInterrupt:
#        break

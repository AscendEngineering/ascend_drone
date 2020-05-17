import cv2
import zmq
import base64
import numpy as np
import pickle

context = zmq.Context()
footage_socket = context.socket(zmq.PUSH)
footage_socket.connect('tcp://10.0.0.60:5555')
#footage_socket.connect('tcp://205.178.63.47:5555')

camera = cv2.VideoCapture(0)  # init the camera

counter = 0

while True:
    try:
        grabbed, frame = camera.read()  # grab the current frame
        #frame = cv2.resize(frame, (640, 480))  # resize the frame
        #encoded, buffer = cv2.imencode('.jpg', frame)
        
        print(counter)
        counter += 1

        footage_socket.send(pickle.dumps(frame),zmq.NOBLOCK)
        #footage_socket.send_string(frame.tostring())
        #footage_socket.send_string(str(buffer))

        if cv2.waitKey(1) == 27:
            break

    except KeyboardInterrupt:
        camera.release()
        cv2.destroyAllWindows()
        break

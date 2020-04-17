#import cv2
#import zmq
#import base64
#import numpy as np
#
#context = zmq.Context()
#footage_socket = context.socket(zmq.SUB)
#footage_socket.bind('tcp://*:5555')
#footage_socket.setsockopt_string(zmq.SUBSCRIBE, np.unicode(''))
#
#while True:
#    try:
#        frame = footage_socket.recv_string()
#        img = base64.b64decode(frame)
#        npimg = np.fromstring(img, dtype=np.uint8)
#        source = cv2.imdecode(npimg, 1)
#        cv2.imshow("image", source)
#        cv2.waitKey(1)
#
#    except KeyboardInterrupt:
#        cv2.destroyAllWindows()
#        break

import cv2
import zmq
import base64
import numpy as np
import pickle
import _thread


image = b''

def receive_image():
    context = zmq.Context()
    footage_socket = context.socket(zmq.PULL)
    footage_socket.bind('tcp://*:35636')
    #footage_socket.setsockopt_string(zmq.SUBSCRIBE, np.unicode(''))
    
    global image

    while True:
        image = footage_socket.recv()

def main():

    _thread.start_new_thread(receive_image,())

    counter = 0    

    while True:

        if(image == b''):
            continue

        try:
            source = pickle.loads(image)

            print(counter)
            counter += 1

            #display
            cv2.imshow("image", source)
            if cv2.waitKey(1) == 27:
                break

        except KeyboardInterrupt:
            cv2.destroyAllWindows()
            break


if __name__ == "__main__":
    main()

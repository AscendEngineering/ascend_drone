import numpy as np
import cv2
import socket
UDP_IP = "10.0.0.60"
UDP_PORT = 5555
cap = cv2.VideoCapture(0)
while(True):
    ret, frame = cap.read()
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    d = frame.flatten ()
    s = d.tostring ()
    print(len(s))
    for i in range(20):
        sock.sendto (s[i*46080:(i+1)*46080],(UDP_IP, UDP_PORT))
        #sock.sendto(str(i).encode(),(UDP_IP, UDP_PORT))
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv2.destroyAllWindows()

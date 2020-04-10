import cv2

#ffmpeg -f v4l2 -i /dev/video0 -preset ultrafast -vcodec libx264 -tune zerolatency -b 900k -f h264 udp://10.0.0.48:5000
#ffmpeg -f dshow -i video="Virtual-Camera" -preset ultrafast -vcodec libx264 -tune zerolatency -b 900k -f mpegts udp://10.0.0.60:5000

cap = cv2.VideoCapture('udp://*:5000',cv2.CAP_FFMPEG)
if not cap.isOpened():
    print('VideoCapture not opened')
    exit(1)

while True:
    ret, frame = cap.read()

    if not ret:
        print('frame empty')
        break

    cv2.imshow('image', frame)

    if cv2.waitKey(1)&0XFF == ord('q'):
        break

cap.release()
waitKey(1)
cv2.destroyAllWindows()
waitKey(1)

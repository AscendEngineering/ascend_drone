import argparse
import socket
import netifaces as ni
import tempfile
import subprocess
import time
import urllib.request
import paramiko
import threading
import signal
import time
import sys
from functools import partial


sdp_file_template = '''
SDP:
v=0
o=- 0 0 IN IP4 127.0.0.1
s=No Name
c=IN IP4 {}
t=0 0
a=tool:libavformat 58.20.100
m=video 5555 RTP/AVP 96
a=rtpmap:96 H264/90000
a=fmtp:96 packetization-mode=1
'''

#video_transmission_template = 'ffmpeg -thread_queue_size 4 -i /dev/video0 -preset ultrafast -vcodec libx264 -tune zerolatency -an -f rtp rtp://{}:5555 -sdp_file saved_sdp_file.sdp'
#video_transmission_template = 'ffmpeg -re -i /dev/video0 -c:v libx264 -b:v 1600k -preset ultrafast -tune zerolatency -b 900k -c:a libfdk_aac -b:a 128k -s 1920x1080 -x264opts keyint=50 -g 25 -pix_fmt yuv420p -f rtp rtp://{}:5555 -sdp_file saved_sdp_file.sdp'
video_transmission_template = 'ffmpeg -thread_queue_size 4 -i /dev/video0 -preset ultrafast -vcodec libx264 -an -vf "transpose=2,transpose=2" -f rtp rtp://{}:5555 -sdp_file saved_sdp_file.sdp'

def get_local_ip_helper(interface):

    retval = ""
    try:
        ni.ifaddresses(interface)
        retval = ni.ifaddresses(interface)[ni.AF_INET][0]['addr']
    except:
        pass
    
    #check if local ip
    if(len(retval) >= 3 and retval[0:3]=="127"):
        retval = ""

    return retval


def get_local_ip():

    #SEARCH ORDER
        #wired
        #wirelss
        #old wired
        #old wireless

    found = False
    retval = ""
    interfaces = ni.interfaces()

    for interface in interfaces:
        retval = get_local_ip_helper(interface)
        if(retval != ""):
            break
        
    return retval

def end_ssh(client,signal,frame):
    client.exec_command('killall ffmpeg')
    time.sleep(1)
    client.close()
    exit(0)

def ssh_setup_cleanup(client):
    signal.signal(signal.SIGINT, partial(end_ssh,client))

def ssh_and_transmit(drone_ip,our_ip):

    #setup ssh
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    #cleanup on close
    ssh_setup_cleanup(client)

    #connect and stream
    client.connect(drone_ip,username='pi',password='ZachLikesAnal917')
    stdin, stdout, stderr = client.exec_command(video_transmission_template.format(our_ip))
    for line in stdout:
        print("std: ", line)
    for line in stderr:
        print("err: ", line)
    client.close()

def open_viewer(our_ip):
    #create temp sdp file
    sdp_file = tempfile.NamedTemporaryFile(mode='w', suffix='.sdp')
    sdp_file.write(sdp_file_template.format(our_ip))
    sdp_file.flush()

    print("\nSend to:",our_ip,"\n")

    #execute ffplay with the sdp file
    viewer_cmd = "ffplay -protocol_whitelist rtp,udp,file -i " + sdp_file.name
    print(viewer_cmd)
    subprocess.run(viewer_cmd.split())



def main():
    
    parser = argparse.ArgumentParser(description='view stream from drone')
    parser.add_argument('ip_addr',help='ip address of the device to open video from')
    parser.add_argument('-r','--remote',action='store_true', help="use if streamer & viewer not on local network")
    args = parser.parse_args()

    ip = ""
    if(args.remote):
        ip = urllib.request.urlopen('https://api.ipify.org').read().decode('utf-8')
    else:
        ip = get_local_ip()
        if(ip == ""):
            print("No internet connection")
            exit(1)

    #open viewer
    viewer = threading.Thread(target=open_viewer,args=(ip,))
    viewer.start()

    #ssh and transmit video
    time.sleep(5)
    ssh_and_transmit(args.ip_addr,ip)



if __name__ == "__main__":
    main()

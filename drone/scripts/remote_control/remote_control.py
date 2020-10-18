import msgDef_pb2
import argparse
from pynput.keyboard import Key, Listener
import copy
import zmq
import time


#setup socket
context = zmq.Context()
socket = context.socket(zmq.PUSH)

#setup global variables
RATE_INCREMENT=0.2
SENSITIVITY=0.05
YAW_RATE=5

remote_open = True
send_update = True
max_rate=2
drone_movements = {
    "x":0,
    "y":0,
    "z":0,
    "rate":1,
    "yaw":0
}




def same_dict(dict1,dict2):
    return (dict1["y"]==dict2["y"] and 
            dict1["x"]==dict2["x"] and 
            dict1["z"]==dict2["z"] and 
            dict1["yaw"]==dict2["yaw"] and 
            dict1["rate"]==dict2["rate"])

def get_char(key):
    key_char = ''
    try:
        key_char = key.char
    except:
        pass
    return key_char

def send_me(new_msg):
    global drone_movements
    if(not same_dict(new_msg,drone_movements)):
        global send_update
        drone_movements = copy.deepcopy(new_msg)
        send_update = True

def on_press(key):

    #get key
    key_char = get_char(key)
    if(key_char == ''):
        return

    #process input
    global drone_movements
    global max_rate
    new_msg = copy.deepcopy(drone_movements)
    
    if(key_char=='w' and abs(new_msg["y"]) < max_rate):
        new_msg["y"] += RATE_INCREMENT
    elif(key_char=='s' and abs(new_msg["y"]) < max_rate):
        new_msg["y"] -= RATE_INCREMENT
    elif(key_char=='d' and abs(new_msg["x"]) < max_rate):
        new_msg["x"] += RATE_INCREMENT
    elif(key_char=='a' and abs(new_msg["x"]) < max_rate):
        new_msg["x"] -= RATE_INCREMENT
    elif(key_char=='e' and abs(new_msg["yaw"]) < max_rate):
        new_msg["yaw"] += (YAW_RATE*RATE_INCREMENT)
    elif(key_char=='q' and abs(new_msg["yaw"]) < max_rate):
        new_msg["yaw"] -= (YAW_RATE*RATE_INCREMENT)
    elif(key_char=='r' and abs(new_msg["z"]) < max_rate):
        new_msg["z"] -= RATE_INCREMENT
    elif(key_char=='f' and abs(new_msg["z"]) < max_rate):
        new_msg["z"] += RATE_INCREMENT
    elif(key_char=='t'):
        max_rate += RATE_INCREMENT
        print(max_rate)
    elif(key_char=='g'):
        max_rate -= RATE_INCREMENT
        print(max_rate)

    send_me(new_msg)

def on_release(key):

    #get key
    key_char = get_char(key)
    if(key_char == ''):
        return

    #setup variables
    global drone_movements
    new_msg = copy.deepcopy(drone_movements)

    #process input
    if(key_char == 'x'):
        # Stop listener
        global remote_open
        remote_open = False
        return False
    elif(key_char=='w'):
        while(new_msg["y"] > 0):
            new_msg["y"] -= RATE_INCREMENT
            send_me(new_msg)
            time.sleep(SENSITIVITY)
    elif(key_char=='s'):
        while(new_msg["y"] < 0):
            new_msg["y"] += RATE_INCREMENT
            send_me(new_msg)
            time.sleep(SENSITIVITY)
    elif(key_char=='d'):
        while(new_msg["x"] > 0):
            new_msg["x"] -= RATE_INCREMENT
            send_me(new_msg)
            time.sleep(SENSITIVITY)
    elif(key_char=='a'):
        while(new_msg["x"] < 0):
            new_msg["x"] += RATE_INCREMENT
            send_me(new_msg)
            time.sleep(SENSITIVITY)
    elif(key_char=='e'):
        while(new_msg["yaw"] > 0):
            new_msg["yaw"] -= (YAW_RATE*RATE_INCREMENT)
            send_me(new_msg)
            time.sleep(SENSITIVITY)
    elif(key_char=='q'):
        while(new_msg["yaw"] < 0):
            new_msg["yaw"] += (YAW_RATE*RATE_INCREMENT)
            send_me(new_msg)
            time.sleep(SENSITIVITY)
    elif(key_char=='r'):
        while(new_msg["z"] < 0):
            new_msg["z"] += RATE_INCREMENT
            send_me(new_msg)
            time.sleep(SENSITIVITY)
    elif(key_char=='f'):
        while(new_msg["z"] > 0):
            new_msg["z"] -= RATE_INCREMENT
            send_me(new_msg)
            time.sleep(SENSITIVITY)


    

def send_msg(msg,ip):
    global socket
    socket.send(msg.SerializeToString())
    


def convert_to_proto(dictionary_msg):
    sendme = msgDef_pb2.msg()
    sendme.name = "any_drone"
    sendme.offset.x = dictionary_msg["x"]
    sendme.offset.y = dictionary_msg["y"]
    sendme.offset.z = dictionary_msg["z"]
    sendme.offset.yaw = dictionary_msg["yaw"]
    sendme.offset.rate = dictionary_msg["rate"]
    return sendme

def main():

    #parse args
    parser = argparse.ArgumentParser(description='Remotely fly drone')
    parser.add_argument('ip_address',help='ip address of drone')
    args = parser.parse_args()
    print("Connecting to",args.ip_address)

    #connect to drone
    try:
        global socket
        socket.connect("tcp://" + args.ip_address + ":5556")
    except:
        print("Could not connect to",args.ip_address)

    #start key listener
    listener =  Listener(on_press=on_press,on_release=on_release)
    listener.start()
    listener.wait()

    #start controlling
    global remote_open
    global send_update
    global drone_movements
    while(remote_open):

        if(send_update):
            #send message
            print(drone_movements)
            proto_drone_movements = convert_to_proto(drone_movements)
            send_msg(proto_drone_movements,args.ip_address)
            send_update = False
        continue


if __name__ == "__main__":
    main()











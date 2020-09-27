import msgDef_pb2
import argparse
from pynput.keyboard import Key, Listener
import copy

remote_open = True
send_update = True
drone_movements = {
    "x":0,
    "y":0,
    "z":0,
    "rate":0,
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

def on_press(key):

    #get key
    key_char = get_char(key)
    if(key_char == ''):
        return

    #process input
    global drone_movements
    new_msg = copy.deepcopy(drone_movements)
    
    if(key_char=='w'):
        new_msg["y"] = 1
    elif(key_char=='s'):
        new_msg["y"] = -1
    elif(key_char=='d'):
        new_msg["x"] = 1
    elif(key_char=='a'):
        new_msg["x"] = -1
    elif(key_char=='e'):
        new_msg["yaw"] = 1
    elif(key_char=='q'):
        new_msg["yaw"] = -1
    elif(key_char=='r'):
        new_msg["z"] = 1
    elif(key_char=='f'):
        new_msg["z"] = -1
    elif(key_char=='t'):
        new_msg["rate"] += 0.5
    elif(key_char=='g'):
        new_msg["rate"] -= 0.5

    #check if messages the same
    if(not same_dict(new_msg,drone_movements)):
        global send_update
        drone_movements = new_msg
        send_update = True



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
        new_msg["y"] = 0
    elif(key_char=='s'):
        new_msg["y"] = 0
    elif(key_char=='d'):
        new_msg["x"] = 0
    elif(key_char=='a'):
        new_msg["x"] = 0
    elif(key_char=='e'):
        new_msg["yaw"] = 0
    elif(key_char=='q'):
        new_msg["yaw"] = 0
    elif(key_char=='r'):
        new_msg["z"] = 0
    elif(key_char=='f'):
        new_msg["z"] = 0

    #check if messages the same
    if(not same_dict(new_msg,drone_movements)):
        global send_update
        drone_movements = new_msg
        send_update = True



def main():

    #parse args
    parser = argparse.ArgumentParser(description='Remotely fly drone')
    parser.add_argument('ip_address',help='ip address of drone')
    args = parser.parse_args()


    print(args.ip_address)

    #start key listener
    listener =  Listener(on_press=on_press,on_release=on_release)
    listener.start()
    listener.wait()

    global remote_open
    global send_update
    while(remote_open):

        if(send_update):
            #send message
            print(drone_movements)
            send_update = False
        continue


if __name__ == "__main__":
    main()











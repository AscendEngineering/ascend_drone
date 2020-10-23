import msgDef_pb2
import argparse
from pynput.keyboard import Key, Listener
import copy
import zmq
import time
import inputs
import keyboard
import game_controller
import constants
    

    
def convert_to_proto(input_msg,rate):
    sendme = msgDef_pb2.msg()
    sendme.name = "any_drone"
    sendme.offset.x = input_msg.x * rate
    sendme.offset.y = input_msg.y * rate
    sendme.offset.z = input_msg.z * rate
    sendme.offset.yaw = input_msg.r * rate
    sendme.offset.rate = 1

    return sendme

def input_print(input_msg):
    print("X:", input_msg.x, " Y:", input_msg.y, " Z:", input_msg.z, " Yaw:", input_msg.r, " Max-Rate:", input_msg.max_rate )

def main():

    #parse args
    parser = argparse.ArgumentParser(description='Remotely fly drone')
    parser.add_argument('ip_address',help='ip address of drone')
    parser.add_argument('-g','--game_controller',action='store_true',help='use game controller instead of computer')
    args = parser.parse_args()
    print("Connecting to...",args.ip_address)

    #connect to drone
    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    try:
        socket.connect("tcp://" + args.ip_address + ":5556")
    except:
        print("Could not connect to",args.ip_address)

    #input
    if(args.game_controller):
        print("game controler")
    else:
        print("keboard")

    remote_open = True
    send_update = True
    last_device_input = constants.control_inputs()
    max_rate = 0.5
    current_rate = 0.0
    while(remote_open):

        #read input
        if(args.game_controller):
            device_input = game_controller.get_input()
        else:
            device_input = keyboard.get_input()

        #update
        if(not constants.same_input(last_device_input,device_input)):

            #max rate
            if(device_input.max_rate != last_device_input.max_rate):
                max_rate += device_input.max_rate 
                max_rate = max(0,max_rate)

            #adjust and send
            proto_drone_movements = convert_to_proto(device_input,max_rate)

            #print and replace
            input_print(device_input)
            last_device_input = device_input
            socket.send(proto_drone_movements.SerializeToString())
            
        continue


if __name__ == "__main__":
    main()











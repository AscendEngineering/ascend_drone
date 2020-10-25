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
    sendme.offset.z = input_msg.z * rate * constants.HEIGHT_ADJUSTMENT
    sendme.offset.yaw = input_msg.r * rate * constants.YAW_ADJUSTMENT
    sendme.offset.rate = 1

    return sendme

def input_print(input_msg,rate):
    print("X:", input_msg.x, " Y:", input_msg.y, " Z:", input_msg.z, " Yaw:", input_msg.r, " Max-Rate:", rate )

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
    drone_movements = constants.control_inputs()
    max_rate = 1.0
    while(remote_open):

        #read input
        if(args.game_controller):
            device_input = game_controller.get_input()
        else:
            device_input = keyboard.get_input()

        #update controls
        for key,val in device_input.items():
            if(key=='x'):
                drone_movements.x = val
            elif(key=='y'):
                drone_movements.y = val
            elif(key=='z'):
                drone_movements.z = val
            elif(key=='r'):
                drone_movements.r = val
            elif(key=='max_rate'):
                max_rate += val
                max_rate = min(3,max_rate)

        #adjust and send
        proto_drone_movements = convert_to_proto(drone_movements,max_rate)

        #print and replace
        input_print(drone_movements,max_rate)
        socket.send(proto_drone_movements.SerializeToString())
            
        continue


if __name__ == "__main__":
    main()











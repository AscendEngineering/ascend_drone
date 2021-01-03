import msgDef_pb2
import argparse
import zmq
import time
import inputs
import constants 
import threading
import signal
import asyncio

global_dict = {"rate": 1.0}
global_lock = threading.RLock()

class movement_axis:
    x=0.0
    y=0.0
    z=0.0
    r=0.0
    rate=0.5

class buttons:
    kill1 = 0
    kill2 = 0
    takeoff = 0
    land = 0
    pickup = 0
    dropoff = 0
    rate = 1.0

def now_milli():
    return int(round(time.time()*1000))

def normalize_joystick(value):
	return round((value - 127)/127,2)

def normalize_rate(value):
    ret_rate = min(3.0,value)
    if(ret_rate < 0):
        ret_rate = 0.0
    return ret_rate

def get_inputs():
    while(True):
        events = inputs.get_gamepad()
        for event in events:

            #triggered
            with global_lock:
                if(event.ev_type=="Absolute"):
                    if(event.code=="ABS_Y"):
                        global_dict["y"] = -1 * normalize_joystick(event.state)
                    elif(event.code=="ABS_X"):
                        global_dict["x"] = normalize_joystick(event.state)
                    elif(event.code=="ABS_Z"):
                        global_dict["r"] = normalize_joystick(event.state)

                elif(event.ev_type == "Key"):
                    if(event.code == "BTN_BASE"):
                        global_dict["z"] = (event.state)
                    elif(event.code == "BTN_BASE2"):
                        global_dict["z"] = (event.state) * -1
                    elif(event.code == "BTN_TOP2" and event.state):
                        global_dict["rate"] = normalize_rate(global_dict["rate"] - 0.5)
                    elif(event.code == "BTN_PINKIE" and event.state):
                        global_dict["rate"] = normalize_rate(global_dict["rate"] + 0.5)
                    elif(event.code == "BTN_TOP"):
                        global_dict["takeoff"] = event.state
                    elif(event.code == "BTN_THUMB"):
                        global_dict["land"] = event.state
                    elif(event.code == "BTN_TRIGGER"):
                        global_dict["pickup"] = event.state
                    elif(event.code == "BTN_THUMB2"):
                        global_dict["dropoff"] = event.state
                    elif(event.code == "BTN_BASE3"):
                        global_dict["kill1"] = event.state
                    elif(event.code == "BTN_BASE4"):
                        global_dict["kill2"] = event.state


def controls_to_proto(input_msg):

    sendme = msgDef_pb2.msg()
    sendme.name = "drone"
    sendme.offset.x = eliminate_trivial(input_msg.x) * input_msg.rate
    sendme.offset.y = eliminate_trivial(input_msg.y) * input_msg.rate
    sendme.offset.z = eliminate_trivial(input_msg.z) * input_msg.rate * constants.HEIGHT_ADJUSTMENT
    sendme.offset.yaw = eliminate_trivial(input_msg.r) * input_msg.rate * constants.YAW_ADJUSTMENT
    sendme.offset.rate = 1

    print("X:", sendme.offset.x, " Y:", sendme.offset.y, " Z:", sendme.offset.z, " Yaw:", sendme.offset.yaw, " Rate:", input_msg.rate)

    return sendme

def command_to_proto(cmd_enum):
    sendme = msgDef_pb2.msg()
    sendme.name = "drone"
    sendme.action_cmd.cmd = cmd_enum
    return sendme

def eliminate_trivial(input_number):
    if(abs(input_number) <= 0.05):
        return 0
    else:
        return input_number

def same_input(input1,input2):
    return(
        input1.x==input2.x 
        and input1.y==input2.y
        and input1.z==input2.z
        and input1.r==input2.r
        and input1.rate==input2.rate)

def main():

    cntr = 0

    #parse args
    parser = argparse.ArgumentParser(description='Remotely fly drone')
    parser.add_argument('ip_address',help='ip address of drone')
    args = parser.parse_args()
    print("Connecting to...",args.ip_address)

    #connect to drone
    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    try:
        socket.connect("tcp://" + args.ip_address + ":5556")
    except:
        print("Could not connect to",args.ip_address)

    #listen to controller
    controller_thread = threading.Thread(target=get_inputs)
    controller_thread.start()

    #start control
    last_movement = movement_axis()
    global_buttons = buttons()
    remote_open = True
    global global_dict
    while(remote_open):

        drone_movements = movement_axis()
        with global_lock:
            for key,val in global_dict.items():
                if(key=='x'):
                    drone_movements.x = val
                elif(key=='y'):
                    drone_movements.y = val
                elif(key=='z'):
                    drone_movements.z = val
                elif(key=='r'):
                    drone_movements.r = val
                elif(key=='rate'):
                    drone_movements.rate = val
                elif(key=='kill1'):
                    if(val==0):
                        global_buttons.kill1 = 0
                    elif(global_buttons.kill1==0):
                        global_buttons.kill1 = now_milli()
                elif(key=='kill2'):
                    if(val==0):
                        global_buttons.kill2 = 0
                    elif(global_buttons.kill2==0):
                        global_buttons.kill2 = now_milli()
                elif(key=='takeoff'):
                    if(val==0):
                        global_buttons.takeoff = 0
                    elif(global_buttons.takeoff==0):
                        global_buttons.takeoff = now_milli()
                elif(key=='land'):
                    if(val==0):
                        global_buttons.land = 0
                    elif(global_buttons.land==0):
                        global_buttons.land = now_milli()
                elif(key=='pickup'):
                    if(val==0):
                        global_buttons.pickup = 0
                    elif(global_buttons.pickup==0):
                        global_buttons.pickup = now_milli()
                elif(key=='dropoff'):
                    if(val==0):
                        global_buttons.dropoff = 0
                    elif(global_buttons.dropoff==0):
                        global_buttons.dropoff = now_milli()

            #send
            if(not same_input(drone_movements,last_movement)):
                proto_drone_movements = controls_to_proto(drone_movements)
                socket.send(proto_drone_movements.SerializeToString())
                last_movement = drone_movements

        #do checks here
        now = now_milli()
        if(global_buttons.kill1 > 0 
            and global_buttons.kill2 > 0
            and (now-global_buttons.kill1) > 500
            and (now-global_buttons.kill2) > 500):
            global_buttons.kill1 = 0
            global_buttons.kill2 = 0
            kill_cmd = command_to_proto(msgDef_pb2.action_cmd_enum.KILL)
            socket.send(kill_cmd.SerializeToString())
            print("Kill")
        if((global_buttons.takeoff > 0) and (now-global_buttons.takeoff > 1000)):
            global_buttons.takeoff = 0
            kill_cmd = command_to_proto(msgDef_pb2.action_cmd_enum.TAKEOFF)
            socket.send(kill_cmd.SerializeToString())
            print("Takeoff")
        if((global_buttons.land > 0) and (now-global_buttons.land > 1000)):
            global_buttons.land = 0
            kill_cmd = command_to_proto(msgDef_pb2.action_cmd_enum.LAND)
            socket.send(kill_cmd.SerializeToString())
            print("Land")
        if((global_buttons.pickup > 0) and (now-global_buttons.pickup > 1000)):
            global_buttons.pickup = 0
            kill_cmd = command_to_proto(msgDef_pb2.action_cmd_enum.PICKUP)
            socket.send(kill_cmd.SerializeToString())
            print("Pickup")
        if((global_buttons.dropoff > 0) and (now-global_buttons.dropoff > 1000)):
            global_buttons.dropoff = 0
            kill_cmd = command_to_proto(msgDef_pb2.action_cmd_enum.DROPOFF)
            socket.send(kill_cmd.SerializeToString())
            print("Dropoff")

if __name__ == "__main__":
    main()














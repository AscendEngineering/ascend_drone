import inputs
import constants

def normalize_joystick(value):
	return round((value - 127)/127,2)


def get_input():
	
	#print all events: print(event.ev_type, event.code, event.state)
    retval = constants.control_inputs()
    
    return_input = False
    while(return_input == False):
        events = inputs.get_gamepad()
        for event in events:

            #triggered
            if(event.ev_type=="Absolute"):
                return_input = True
                if(event.code=="ABS_Y"):
                    retval.y = -1 * normalize_joystick(event.state)
                if(event.code=="ABS_X"):
                    retval.x = normalize_joystick(event.state)
                if(event.code=="ABS_Z"):
                    retval.r = normalize_joystick(event.state)

            elif(event.ev_type == "Key"):
                return_input = True
                if(event.code == "BTN_BASE"):
                    retval.z = (event.state)
                elif(event.code == "BTN_BASE2"):
                    retval.z = (event.state) * -1
                elif(event.code == "BTN_TOP2" and event.state==1):
                    retval.max_rate = (event.state) * -1
                elif(event.code == "BTN_PINKIE" and event.state==1):
                    retval.max_rate =  event.state
                elif(event.code == "BTN_TOP" and event.state==1):
                    print("Takeoff")
                elif(event.code == "BTN_THUMB" and event.state==1):
                    print("Land")

    return retval
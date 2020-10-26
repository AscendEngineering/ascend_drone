import keyboard
import constants



def get_input():
    retval = constants.control_inputs()

    if keyboard.is_pressed('w'):
        retval.y = 1.0
    elif keyboard.is_pressed('s'):
        retval.y = -1.0

    return retval
    
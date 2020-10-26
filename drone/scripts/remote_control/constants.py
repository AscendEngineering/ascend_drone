RATE_INCREMENT=0.2
MAX_ACC=0.5
YAW_ADJUSTMENT=5
HEIGHT_ADJUSTMENT=0.2


class control_inputs:
    x=0.0
    y=0.0
    z=0.0
    r=0.0
    max_rate=0.5

def same_input(input1,input2):
    return(
        input1.x==input2.x 
        and input1.y==input2.y
        and input1.z==input2.z
        and input1.r==input2.r
        and input1.max_rate==input2.max_rate)

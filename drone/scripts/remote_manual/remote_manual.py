import argparse
from pynput.keyboard import Key, Listener
import msgDef_pb2


msg = 0
send_msg = True

def key_down(key):
    print("DOWN",key)

def key_up(key):
    print("UP", key)


def main():
    
    #get the ip address
    parser = argparse.ArgumentParser(description='controls drone remotely')
    parser.add_argument('ip_address', help='ip address')
    args = parser.parse_args()

    with Listener(
            on_press=key_down,
            on_release=key_up) as listener:
        listener.join()

    #start while loop
    run = True
    while(run):

        if(send_msg):
            send()
            send_msg = False
        continue

        #if key pressed then modify the parameters
        #if key is pressed revert back to 0
        #send message if any action happened

    




if __name__ == "__main__":
    main()

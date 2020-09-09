import nexmo
import serial


client = nexmo.Client(key='f7cc0105', secret='skbMFLX1yz0rGVZc')
def send(text):
    client.send_message({
        'from': '12262770917',
        'to': '17789980302',
        'text': text,
        })
def msg(cmd , uid):
    if int(cmd) == 2:
        message = "ID: " + uid + " has registered for attendance"
        return message
    else:
        return "Unknown Command"
    
    

ser = serial.Serial(
    port='COM8',
    #baudrate=115200,
    baudrate = 9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

while True:
    # make sure only one thread is accessing serial at this given time to avoid conflict.
    data_raw = ser.readline().decode().strip()

    # ignore data if it is not an integer.
    if data_raw:
        result = data_raw
        splitVal = result.split()
        send(msg(splitVal[0], splitVal[1]))
             
    else:
        print("No Data")
        

import nexmo
import serial


client = nexmo.Client(key='f7cc0105', secret='skbMFLX1yz0rGVZc')
def send(text):
    client.send_message({
        'from': '12262770917',
        'to': '17789980302',
        'text': text,
        })

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
        print(splitVal[0])
    else:
        print("No Data")
        

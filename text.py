import nexmo


client = nexmo.Client(key='f7cc0105', secret='skbMFLX1yz0rGVZc')

client.send_message({
    'from': '12262770917',
    'to': '17789980302',
    'text': 'Fuck you python',
})

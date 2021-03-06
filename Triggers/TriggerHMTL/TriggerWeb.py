#!/usr/local/bin/python3
#
# Note: This only works with Python3

from wsgiref.simple_server import make_server
from io import StringIO
import re
import serial

triggers = []
values = [0, 64, 128, 196, 255]

trig_state = {}

#
# Return a trigger and value from 'trig=<trig>&value=<val>'
#
trig_matcher = re.compile('trig=([0-9]+)')
value_matcher = re.compile('value=([0-9]+)')
def get_trig_value(str_data):
    trigmatch = trig_matcher.search(str_data)
    valmatch = value_matcher.search(str_data)
    if (trigmatch and valmatch):
        trig = int(trigmatch.group(1))
        val = int(valmatch.group(1))
        return (trig, val)
    return (None, None)

#
# Serial communication
#
device = '/dev/tty.usbserial-AM01SJR1'
ser = serial.Serial(device, 9600, timeout=1)
def waitForReady():
    """Wait for the Arduino to send its ready signal"""
    while True:
        data = ser.readline().strip().decode()
        print("Read from serial: %s" % (data))
        if (data == "Ready for commands"):
            print("Recieved ready from Arduino")
            break

# Send request for current values and parse into the trigger state
def getTriggerValues():
    ser.write(bytes('s\n', 'utf-8'))
    while True:
        data = ser.readline().strip().decode()
        print("Read from serial: '%s'" % (data))
        if (data == "Done"):
            break
        if (data):
            (trig, val) = get_trig_value(data)
            if ((trig != None) and (val != None)):
                trig_state[trig] = val

def serial_update(trig, val):
    trig_state[trig] = val

    data = "t %d %d\n" % (trig, val)
    print("Will send: %s" % (data))
    ser.write(bytes(data, 'utf-8'))

#
# Parse form data
#
def handle_post(post_data):
    str_data = post_data.decode()
    print("Received post data '%s'" % (str_data))
    (trig, val) = get_trig_value(str_data)
    if ((trig != None) and (val != None)):
        print("Trigger=%d Value=%d" % (trig, val))
        serial_update(trig, val)
    
#
# Generate the form
#

header = "<html><header> <title>Trigger Board Control</title></header><body>"
footer = "</body></html>"
def html_page(content):
    page = "%s\n%s\n%s" % (header, content, footer)
    return page.encode()

def form_app(environ, start_response):
    status = '200 OK'
#    headers = [('Content-Type', 'text/hmtl; charset=utf-8')] # Don't know???
    headers=[(str('Content-Type'), str('text/html; charset=utf-8'))]
    start_response(status, headers)

    output = StringIO()
    print("<H1>This sends commands to Adam's trigger board</H1>", file=output)
    if environ['REQUEST_METHOD'] == 'POST':
        size = int(environ['CONTENT_LENGTH'])
        post_str = environ['wsgi.input'].read(size)
        handle_post(post_str)

    for trig in trig_state:
        print("Trigger %d: %d<p>" % (trig, trig_state[trig]), file=output)

    print('<form method="POST">', file=output)

    print('Trigger:<select name=trig>',
          "".join(['<option value="%d">%d</option>' % (t, t) for t in triggers]),
          '</select>',
          file=output)

    print('Value:<select name=value>',
          "".join(['<option value="%d">%d</option>' % (t, t) for t in values]),
          '</select>',
          file=output)

    print('<input type="submit" value="Send"></form>', file=output)

    return [html_page(output.getvalue())]

# Wait for serial to initialize and request the trigger list
waitForReady()
getTriggerValues()
triggers = list(trig_state.keys())

#httpd = make_server('127.0.0.1', 8000, form_app)
httpd = make_server('', 8000, form_app)
print("Serving on port 8000...")

httpd.serve_forever()

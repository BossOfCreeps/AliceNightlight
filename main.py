#!/usr/bin/env python3
from flask import Flask, request
app = Flask(__name__)

RGB = 16777215
LED = 1
BRI = 100

import sqlite3, os
from sqlite3 import Error
import paho.mqtt.client as mqtt

def sql():
    global RGB
    global LED
    global BRI

    conn = None
    try:
        conn = sqlite3.connect("sensordata.db")
    except Error as e:
        print(e)

    cur = conn.cursor()

    cur.execute("SELECT param  FROM sql_table WHERE device='test/rgb' ORDER BY id DESC LIMIT 1;")
    rows = cur.fetchall()
    for row in rows:
        RGB = int(row[0])

    cur.execute("SELECT param FROM sql_table WHERE device='test/led' ORDER BY id DESC LIMIT 1;")
    rows = cur.fetchall()
    for row in rows:
        LED = int(row[0])

    cur.execute("SELECT param FROM sql_table WHERE device='test/bright' ORDER BY id DESC LIMIT 1;")
    rows = cur.fetchall()
    for row in rows:
        BRI = int(row[0])


@app.route('/')
def hello_world():
    global RGB
    global BRI
    global LED

    sql()

    str_on_off = ""
    if (LED):
        R = str(hex(int(BRI /100 * RGB / 256 / 256)))[2:4]
        G = str(hex(int(BRI /100 * RGB / 256 % 256)))[2:4]
        B = str(hex(int(BRI /100 * RGB % 256)))[2:4]
        str_on_off = "checked"
    else:
        R = 0
        G = 0
        B = 0

    if len(R)==1:
        R="0"+R
    if len(G)==1:
        G="0"+G
    if len(B)==1:
        B="0"+B

    html = "<table bgcolor=#"+R+G+B+" height='300' width='300'><tr><td></td></tr></table> </br>" \
           "<form method='POST' action='/test'>" \
           "R: <input type='range' min='0' max='255' value="+R+" name='red' /></br>" \
           "G: <input type='range' min='0' max='255' value="+G+" name='green' /></br>" \
           "B: <input type='range' min='0' max='255' value="+B+" name='blue' /></br>"\
           "Яркость: <input type='range' min='0' max='100' value="+str(BRI)+" name='bright' /></br>" \
           "Вкл/выкл: <input type='checkbox' name='on_off' "+str_on_off+"></br></br>" \
           "<input type='submit' value='submit'/><form>"
    return html

@app.route("/test", methods=["POST"])
def test():
    on_off=0
    try:
        on_off = int(request.form["on_off"]=="on")
    except Exception:
        pass

    bright = int(request.form["bright"])
    r = int(request.form["red"])
    g = int(request.form["green"])
    b = int(request.form["blue"])

    mqttc = mqtt.Client()
    mqttc.username_pw_set(username="",password="")
    mqttc.connect("farmer.cloudmqtt.com", 10593, 6)

    mqttc.publish("test/rgb", str((r*255+g)*255+b))
    mqttc.publish("test/bright", str(bright))
    mqttc.publish("test/led", str(on_off))

    hex_r = str(hex(int(r * bright / 100 * on_off)))[2:4]
    hex_g = str(hex(int(g * bright / 100 * on_off)))[2:4]
    hex_b = str(hex(int(b * bright / 100 * on_off)))[2:4]

    dec_r = str(int(r * bright / 100 * on_off))
    dec_g = str(int(g * bright / 100 * on_off))
    dec_b = str(int(b * bright / 100 * on_off))

    if len(hex_r)==1:
        hex_r="0"+hex_r
    if len(hex_g)==1:
        hex_g="0"+hex_g
    if len(hex_b)==1:
        hex_b="0"+hex_b

    str_on_off = ""
    if (on_off==1):
        str_on_off = "checked"

    html = "<table bgcolor=#" + hex_r + hex_g + hex_b + " height='300' width='300'><tr><td></td></tr></table> </br>" \
           "<form method='POST' action='/test'>" \
           "R: <input type='range' min='0' max='255' value=" + dec_r + " name='red' /></br>" \
           "G: <input type='range' min='0' max='255' value=" + dec_g + " name='green' /></br>" \
           "B: <input type='range' min='0' max='255' value=" + dec_b + " name='blue' /></br>"\
           "Яркость: <input type='range' min='0' max='100' value="+str(bright)+" name='bright' /></br>" \
           "Вкл/выкл: <input type='checkbox' name='on_off' "+str_on_off +" ></br></br>" \
           "<input type='submit' value='submit'/><form>"
    return html

app.run("0.0.0.0", 6004)

import serial
import time
import random
import itertools
from struct import pack

class Spark:
    def __init__(self,speed,length,startpos=0,lifetime=180,blinker=False) -> None:
        def make_pattern(length):
            return [(40,30,10),
                    (40,20,3),
                    (20,5,1),
                    (10,2,0),
                    (4,0,0),
                    (2,0,0)
                    ]
        self.speed=speed
        self.length=length
        self.lifetime=lifetime
        self.pos=startpos
        self.pattern=make_pattern(length)
        self.blinker=blinker
        if self.speed>0:
            self.pattern.reverse()
        self.length=len(self.pattern)

    def tick(self):
        self.pos+=self.speed
        self.lifetime-=1
    
    def render(self,frame):
        def add_colors(color_a, color_b):
            a_r,a_g,a_b = color_a
            b_r,b_g,b_b = color_b
            r=min(a_r+b_r,255)
            g=min(a_g+b_g,255)
            b=min(a_b+b_b,255)
            return (r,g,b)
    
        if self.blinker and self.lifetime % 2 == 0:
            return

        framesize=len(frame)
        startpos=int(self.pos+0.5)
        for i in range(self.length):
            if (startpos+i < framesize) and ( startpos+i>=0):
                frame[startpos+i]=add_colors(self.pattern[i],frame[startpos+i])
            

com=serial.Serial(port="COM10",baudrate=112500)
sparks=[]
sparks.append(Spark(-2,6,startpos=160))

def make_data_bytes(frame):
    data=b''
    for i in range(256):
        data+=bytes(frame[i])
    return data



while True:
    if random.random()<0.03:
        if (random.random()<0.5):
            sparks.append(Spark(-(random.random()*3+1),6,256,lifetime=280,blinker=random.random()>0.7))
        else:
            sparks.append(Spark((random.random()*3+1),6,-6,lifetime=280,blinker=random.random()>0.7))
            
    frame=[(0,0,0) for _ in range(256)]
    for s in sparks:
        s.render(frame)
        s.tick()
        if s.lifetime<=0:
            sparks.remove(s)
    
    #print(f"sending: {data} ")
    com.write(make_data_bytes(frame))
    time.sleep(1/60)

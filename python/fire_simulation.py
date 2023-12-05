import serial
from math import pow
from random import random,randrange
from time import sleep

class Fire:
    @classmethod
    def make_heat_ramp(self,length):
        def lerp(a,b,t):
            if t<0 : return a
            if t>1 : return b
            return a*(1-t)+b*t
        def pow_lerp(a,b,t,exp):
            if t<0 : return a
            if t>1 : return b
            t=pow(t,exp)
            return a*(1-t)+b*t

        t0=0
        t1=int(length/3)
        t2=int(length/3*2)
        t3=length-1
        e=2
        colors=[]
        for i in range(length):
            t=i/t3
            colors.append(
                (int(pow_lerp(0,255,t*3,e)), # red
                 int(pow_lerp(0,255,(t-1/3)*3,e)), # green
                 int(pow_lerp(0,200,(t-2/3)*3,e))) # blue
            )
        return colors


    def __init__(self,size):
        self.display=[0.0 for _ in range(size)]
        # each element represents the "temperature", a floating point value from 0.0 to 1.0
        self.color_scale_length=1024
        self.color_scale=self.make_heat_ramp(self.color_scale_length)

    def temperature_to_rgb(self,temperature):
        temperature=max(min(1.0,temperature),0.0)
        i =int( temperature*(self.color_scale_length-1))
        return self.color_scale[i]

    def transfer_heat(self,heat_transfer_factor = 0.4,cool_down_factor = 0.99):
        new_display=[0 for _ in range(len(self.display))]
        for i in range(1,len(self.display)-1):
            delta_1= self.display[i-1]-self.display[i] # >0 means, heat coming in
            delta_2= self.display[i+1]-self.display[i] # <0 means, heat going out
            delta = (delta_1+delta_2)/2 * heat_transfer_factor
            new_display[i]=(self.display[i]+delta)*cool_down_factor
        delta_2= self.display[1]-self.display[0] # <0 means, heat going out
        delta = delta_2 * heat_transfer_factor
        new_display[0]=(self.display[0]+delta)*cool_down_factor
        delta_1= self.display[-2]-self.display[-1] # <0 means, heat going out
        delta = delta_1 * heat_transfer_factor
        new_display[-1]=(self.display[-1]+delta)*cool_down_factor
        self.display=new_display

    def spawn_spark(self):
        temp=random()*0.6+0.2
        pos=randrange(0,len(self.display))
        self.display[pos]+=temp

    def update(self):
        if random()<1:
            self.spawn_spark()
        self.transfer_heat()

    def get_rgb_colors(self):
        return [self.temperature_to_rgb(t) for t in self.display]

def make_data_bytes(colors):
    data=b''
    for i in range(len(colors)):
        data+=bytes(colors[i])
    len_lo=len(data) % 256
    len_hi=len(data) // 256
    data = bytes([len_lo,len_hi])+data
    return data

# main
com=serial.Serial("COM10")
com.write(bytes([0,0]))

fire=Fire(250)
fire.display[30]=1.0
try:
    while True:
        com.write(make_data_bytes(fire.get_rgb_colors()))
        fire.update()
        sleep(1/50)
except :
    com.write(bytes([0,0]))

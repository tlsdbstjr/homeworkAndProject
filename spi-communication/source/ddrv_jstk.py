import spidev
import time

#initializing spi bus
spiToZync = spidev.SpiDev()
spiToZync.open(0,0)
spiToZync.max_speed_hz = 5000
spiToZync.mode = 0b00

spiToSeg = spidev.SpiDev()
spiToSeg.open(0,1)
spiToSeg.max_speed_hz = 5000
spiToSeg.mode = 0b00



refreshPeriod = 0.25    #chage this! the period to refresh the data printed
toggle = 1  #this represents the integer "0x00000001" byte by byte, so this list has 4 components

transferList = spiToZync.readbytes(4)  #read the 4 byte for int
read = ((transferList[0] << 24)&0xFF000000) | ((transferList[1] << 16)&0xFF0000) | ((transferList[2] << 8)&0xFF00) | (transferList[3]&0xFF)   #make a int type data by four byte data
while True:
    #parsing read data to x and y coordinate data and button click data
    y = read & 0x3FF
    x = (read >> 10) & 0x3FF
    btn = (read >> 20) & 0x3
    ToJSTK = read >> 20 #set ToJSTK variable with the data from joy stick's button bits
    #set ToJSTK by LED control signals
    if x > 640:
        ToJSTK |= 1 << 7

    if x < 384:
        ToJSTK |= 1 << 6

    if y > 640:
        ToJSTK |= 1 << 5

    if y < 384:
        ToJSTK |= 1 << 4

    #transfer the data with ZYNQ part
    transferList = [0xff,0xff,ToJSTK,toggle]    #0xff, 0xff at [31:16] is important part
    spiToZync.writebytes(transferList)  #write the control data to ZYNQ board
    transferList = spiToZync.readbytes(4)   #read the data from ZYNQ board
    read = ((transferList[0] << 24)&0xFF000000) | ((transferList[1] << 16)&0xFF0000) | ((transferList[2] << 8)&0xFF00) | (transferList[3]&0xFF)   #make a int type data by four byte data

    #print X, Y coordinate and Button click info, toggle info, which means the lights at jstk turn on or not
    print("X: %04d\tY: %04d\tB: %01d%01d\ttoggle:%d"%(x,y,btn>>1,btn&1,toggle))
    #change the toggle state
    if toggle == 1:
        toggle = 0
    else:
        toggle = 1
    #write the control data to blink the LED on jstk
    x = x / 10
    y = y / 10
    time.sleep(refreshPeriod)
    spiToSeg.writebytes([0x76,int(x/10),int(x%10),int(y/10),int(y%10)])
#    spiToSeg.writebytes([0x76,1,2,3,4])
    time.sleep(refreshPeriod)   #wait for the next read time
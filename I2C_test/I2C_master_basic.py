import smbus

bus = smbus.SMBus(1)

def sendData(slaveAddress, data):
    intsOfData = list(map(ord, data))
    bus.write_i2c_block_data(slaveAddress, intsOfData[0], intsOfData[1:])

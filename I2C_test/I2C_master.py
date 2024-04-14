import smbus2
from PIL import Image

# SMBUS1 WORKED FINE FOR THE BELOW


bus = smbus2.SMBus(1)

def sendData(slaveAddress, data):
    intsOfData = list(map(ord, data))
    print(intsOfData)
    bus.write_i2c_block_data(slaveAddress, intsOfData[0], intsOfData[1:])




# IGNORE THE BELOW, DOESNT WOKR YET

# smbus 2 allows for 512 max bytes per msg, smbus1 allows for 32 only

image = Image.open('4kb_image.png')
image_bytes = image.tobytes()


print(image_bytes)

def send_bytes(image_bytes):
	
	# for i in range(0, len(image_bytes), 32):
	# bus.write_i2c_block_data(0x03,image_bytes[0], image_bytes[1:2])
	# bus.write_i2c_block_data(0x03, image_bytes[0], list(image_bytes)[:64])
	bus.i2c_rdwr(smbus2.i2c_msg.write(0x03, list(image_bytes)[:512]))



send_bytes(image_bytes)

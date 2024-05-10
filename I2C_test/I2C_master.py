import smbus2
import array
from PIL import Image


bus = smbus2.SMBus(1)

def sendString(slaveAddress, data):
    intsOfData = list(map(ord, data)) # Converts string chars to ints
    print(intsOfData)
    bus.write_i2c_block_data(slaveAddress, intsOfData[0], intsOfData[1:])

# sendString(0x03, "hello")


# smbus 2.i2c.rdwr allows for 512 max bytes per msg, smbus1 allows for 32 only

image = Image.open('4kb_image.png')
image_bytes = image.tobytes()
image_size = len(image_bytes)
image_width, image_height = image.size
print("Image Byte Size: " + str(image_size))
print("Image Width: " + str(image_width))
print("Image height: " + str(image_height))


image_size_bytes = image_size.to_bytes(4, byteorder='big')
image_width_bytes = image_width.to_bytes(2, byteorder='big')
image_height_bytes = image_height.to_bytes(2, byteorder='big')

# send image size, width and height first
bus.write_i2c_block_data(0x03, image_size_bytes[0], image_size_bytes[1:])
bus.write_i2c_block_data(0x03, image_width_bytes[0], image_width_bytes[1:])
bus.write_i2c_block_data(0x03, image_height_bytes[0], image_height_bytes[1:])


def send_bytes(image_bytes):
    for i in range(0, len(image_bytes), 512):  
        bus.i2c_rdwr(smbus2.i2c_msg.write(0x03, list(image_bytes)[i: i + 512]))


send_bytes(image_bytes)


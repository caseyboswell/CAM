#include <pigpio.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

void runSlave();
void closeSlave();
int getControlBits(int, bool);
void saveImageFromBytes(const std::vector<uint8_t>& imageData, uint16_t width, uint16_t height, const std::string& outputPath);



const int slaveAddress = 0x03; // <-- Your address of choice
bsc_xfer_t xfer; // Struct to control data flow
vector<uint8_t> imageBytes;
uint32_t imageSize = 0;
uint16_t imageWidth = 0;
uint16_t imageHeight = 0;
std::string outputPath = "output_image.jpg";

int main(){
    // Chose one of those two lines (comment the other out):
    runSlave();
    // closeSlave();

    return 0;
}

void runSlave() {
    gpioInitialise();
    cout << "Initialized GPIOs\n";
    // Close old device (if any)
    xfer.control = getControlBits(slaveAddress, false); // To avoid conflicts when restarting
    bscXfer(&xfer);
    // Set I2C slave Address to slaveAddress
    xfer.control = getControlBits(slaveAddress, true);
    int status = bscXfer(&xfer); // Should now be visible in I2C-Scanners
    
    if (status >= 0)
    {
        cout << "Opened slave\n";
        xfer.rxCnt = 0;
        while(1){
            bscXfer(&xfer);
            if(xfer.rxCnt > 0) {
                if (imageSize == 0) {
                    if (xfer.rxCnt >= 4) {
                        imageSize = (xfer.rxBuf[0] << 24) | (xfer.rxBuf[1] << 16) | (xfer.rxBuf[2] << 8) | xfer.rxBuf[3];
                        cout << "Received image size: " << imageSize << " bytes\n";
                        imageBytes.reserve(imageSize);
                    } else {
                        cout << "Error: Received less than 4 bytes for image size\n";
                    }
                } else if (imageWidth == 0) {
                    if (xfer.rxCnt >= 2) {
                        imageWidth = (xfer.rxBuf[0] << 8) | xfer.rxBuf[1];
                        cout << "Received image width: " << imageWidth << " pixels\n";
                    } else {
                        cout << "Error: Received less than 2 bytes for image width\n";
                    }
                } else if (imageHeight == 0) {
                    if (xfer.rxCnt >= 2) {
                        imageHeight = (xfer.rxBuf[0] << 8) | xfer.rxBuf[1];
                        cout << "Received image height: " << imageHeight << " pixels\n";
                    } else {
                        cout << "Error: Received less than 2 bytes for image height\n";
                    }
                } else {
                    for(int i = 0; i < xfer.rxCnt; i++) {
                        imageBytes.push_back(xfer.rxBuf[i]);
                    }
                    if (imageBytes.size() == imageSize) {
                        cout << "Received image matching image size\n";

                        // Save image here
                        saveImageFromBytes(imageBytes, imageWidth, imageHeight, outputPath);
                    }
                }
            }
    }
    }else
        cout << "Failed to open slave!!!\n";
}

void saveImageFromBytes(const std::vector<uint8_t>& imageData, uint16_t width, uint16_t height, const std::string& outputPath) {
    cv::Mat bgrImage(height, width, CV_8UC3, const_cast<uint8_t*>(imageData.data()));
    // cv::Mat image(height, width, CV_8UC3, const_cast<uint8_t*>(imageData.data()));
    cv::Mat rgbImage;
    cv::cvtColor(bgrImage, rgbImage, cv::COLOR_BGR2RGB);
    cv::imwrite(outputPath, rgbImage);
}

void closeSlave() {
    gpioInitialise();
    cout << "Initialized GPIOs\n";

    xfer.control = getControlBits(slaveAddress, false);
    bscXfer(&xfer);
    cout << "Closed slave.\n";

    gpioTerminate();
    cout << "Terminated GPIOs.\n";
}


int getControlBits(int address /* max 127 */, bool open) {
    /*
    Excerpt from http://abyz.me.uk/rpi/pigpio/cif.html#bscXfer regarding the control bits:

    22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    a  a  a  a  a  a  a  -  -  IT HC TF IR RE TE BK EC ES PL PH I2 SP EN

    Bits 0-13 are copied unchanged to the BSC CR register. See pages 163-165 of the Broadcom 
    peripherals document for full details. 

    aaaaaaa defines the I2C slave address (only relevant in I2C mode)
    IT  invert transmit status flags
    HC  enable host control
    TF  enable test FIFO
    IR  invert receive status flags
    RE  enable receive
    TE  enable transmit
    BK  abort operation and clear FIFOs
    EC  send control register as first I2C byte
    ES  send status register as first I2C byte
    PL  set SPI polarity high
    PH  set SPI phase high
    I2  enable I2C mode
    SP  enable SPI mode
    EN  enable BSC peripheral
    */

    // Flags like this: 0b/*IT:*/0/*HC:*/0/*TF:*/0/*IR:*/0/*RE:*/0/*TE:*/0/*BK:*/0/*EC:*/0/*ES:*/0/*PL:*/0/*PH:*/0/*I2:*/0/*SP:*/0/*EN:*/0;

    int flags;
    if(open)
        flags = /*RE:*/ (1 << 9) | /*TE:*/ (1 << 8) | /*I2:*/ (1 << 2) | /*EN:*/ (1 << 0);
    else // Close/Abort
        flags = /*BK:*/ (1 << 7) | /*I2:*/ (0 << 2) | /*EN:*/ (0 << 0);

    return (address << 16 /*= to the start of significant bits*/) | flags;
}
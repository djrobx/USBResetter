#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USBSpec.h>

//
//  main.cpp
//  USBResetter
//
//  Created by Rob Raper on 6/20/23.
//

#include <iostream>



int main(int argc, const char * argv[]) {
    CFMutableDictionaryRef matchingDictionary = NULL;
    SInt32 idVendor = 0x06cb; // set vendor id
    SInt32 idProduct = 0x7100; // set product id
    io_iterator_t usb_iterator = 0;
    io_iterator_t iterator = 0;
    io_service_t usbRef;
    SInt32 score;
    IOCFPlugInInterface** plugin;
    IOUSBDeviceInterface** usbDevice = NULL;
    IOReturn ret;
    IOUSBConfigurationDescriptorPtr config;
    IOUSBFindInterfaceRequest interfaceRequest;
    IOUSBInterfaceInterface** usbInterface;
 
    unsigned char prius[] = {"\x01\x00\x11\x00\x00\x81\x00\x00\x00\x00\x00\x05" \
                          "\x00\x00\x00\x50\x52\x49\x55\x53\xd6\x00\x00\x00\x00\x00\x00\x00" \
                          "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                          "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                          "\x00\x00"};
                          
    unsigned char out[] = {"\x01\x00\x0c\x00\x00\xb1\x00\x2c\x02\x20\x20\x04" \
        "\x00\x00\x00\xd1\x20\x00\x71\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
        "\x00\x00\x00\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
        "\x00\x00"
    };
    unsigned char out2[] = {"\x01\x00\x10\x00\x00\xa1\x00\x1c\x02\x20\x20\x04" \
        "\x00\x00\x00\xf5\x00\x00\x00\xf8\x00\x00\x00\x00\x00\x00\x00\x00" \
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
        "\x00\x00\x00\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
        "\x00\x00"
    };
    
    char* in;
    UInt32 numBytes;
    
    matchingDictionary = IOServiceMatching(kIOUSBDeviceClassName);
    CFDictionaryAddValue(matchingDictionary,
                            CFSTR(kUSBVendorID),
                            CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &idVendor));
    CFDictionaryAddValue(matchingDictionary,
                            CFSTR(kUSBProductID),
                            CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &idProduct));
    IOServiceGetMatchingServices(kIOMasterPortDefault,
                                    matchingDictionary, &usb_iterator);
    
    usbRef = IOIteratorNext(usb_iterator);

    if (usbRef == 0)
    {
        printf("No USB VMM7100 device found\n");
        return -1;
    }
    do
    {
        IOCreatePlugInInterfaceForService(usbRef, kIOUSBDeviceUserClientTypeID,
                                          kIOCFPlugInInterfaceID, &plugin, &score);
        IOObjectRelease(usbRef);
        kern_return_t err = (*plugin)->QueryInterface(plugin,
                                  CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
                                  (LPVOID *)&usbDevice);
        if(err != kIOReturnSuccess){
                   printf("QueryInterface kIOUSBDeviceInterface #1 failed: %x\n", err);
                    return -1;
        }
        IODestroyPlugInInterface(plugin);
        
/*        ret = (*usbDevice)->USBDeviceOpen(usbDevice);
        if (ret == kIOReturnSuccess)
        {
            // set first configuration as active
            ret = (*usbDevice)->GetConfigurationDescriptorPtr(usbDevice, 0, &config);
            if (ret != kIOReturnSuccess)
            {
                printf("Could not set active configuration (error: %x)\n", ret);
                return -1;
            }
            (*usbDevice)->SetConfiguration(usbDevice, config->bConfigurationValue);
        }
        else if (ret == kIOReturnExclusiveAccess)
        {
            printf("Exclusive");// this is not a problem as we can still do some things
        }
        else
        {
            printf("Could not open device (error: %x)\n", ret);
            continue;
        }*/
        
        interfaceRequest.bInterfaceClass = kIOUSBFindInterfaceDontCare;
        interfaceRequest.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
        interfaceRequest.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
        interfaceRequest.bAlternateSetting = kIOUSBFindInterfaceDontCare;
        (*usbDevice)->CreateInterfaceIterator(usbDevice,
                                              &interfaceRequest, &iterator);
       // IOIteratorNext(iterator); // skip interface #0
        usbRef = IOIteratorNext(iterator);
        IOObjectRelease(iterator);
        IOCreatePlugInInterfaceForService(usbRef,
                                          kIOUSBInterfaceUserClientTypeID,
                                          kIOCFPlugInInterfaceID, &plugin, &score);
        IOObjectRelease(usbRef);
        err = (*plugin)->QueryInterface(plugin,
                                  CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID),
                                  (LPVOID *)&usbInterface);
        if(err != kIOReturnSuccess){
            printf("QueryInterface failed #2: %x for usbRef %d\n", err, usbRef);
            goto next;
        }
        IODestroyPlugInInterface(plugin);
 
        ret = (*usbInterface)->USBInterfaceOpen(usbInterface);
        if (ret != kIOReturnSuccess)
        {
            printf("Could not open interface (error: %x) for usbRef %d\n", ret, usbRef);
            return -1;
        }
        
        UInt8 endpoints;
        (*usbInterface)->GetNumEndpoints(usbInterface,&endpoints);
        printf("Endponts: %d", endpoints);
        
        IOUSBDevRequest     request;
        request.bmRequestType = 0x21;
        request.bRequest = 0x9;
        request.wValue = 0x0201;
        request.wIndex = 0;
        request.wLength = sizeof(out)-1;
        request.pData = prius;
        request.wLenDone = 0;
        
        ret = (*usbInterface)->ControlRequest(usbInterface, 0, &request);
        if (ret != kIOReturnSuccess)
        {
            printf("Control failed (error: %x)\n", ret);
        }
        sleep(1);
        request.pData = out;
        request.wLenDone = 0;
        ret = (*usbInterface)->ControlRequest(usbInterface, 0, &request);
        if (ret != kIOReturnSuccess)
        {
            printf("Control failed (error: %x)\n", ret);
        }
        sleep(1);
        request.pData = out2;
        request.wLenDone = 0;
        ret = (*usbInterface)->ControlRequest(usbInterface, 0, &request);
        if (ret != kIOReturnSuccess)
        {
            printf("Control failed (error: %x)\n", ret);
        }
        sleep(1);
        (*usbInterface)->USBInterfaceClose(usbInterface);
        (*usbInterface)->Release(usbInterface);
     //   (*usbDevice)->USBDeviceClose(usbDevice);
next:
        usbRef = IOIteratorNext(usb_iterator);
    } while(usbRef != 0);
    IOObjectRelease(usb_iterator);

    return 0;
}

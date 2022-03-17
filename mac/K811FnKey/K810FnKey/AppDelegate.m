//
//  AppDelegate.m
//  K811FnKey
//
//  Created by NuRi on 1/26/14.
//  Copyright (c) 2014 NuRi. All rights reserved.
//

#import "AppDelegate.h"
#include <IOKit/hid/IOHIDManager.h>

@implementation AppDelegate

@synthesize button;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    
}

- (IBAction)pushSet:(id)sender{
    NSLog(@"K811FnKey: start setting");

    if ([button.title isEqual: @"end"]) {
        NSLog(@"K811FnKey: exit");
        exit(0);
    }
    // Create an HID Manager
    IOHIDManagerRef hidManager = IOHIDManagerCreate(kCFAllocatorDefault,
                                                    kIOHIDOptionsTypeNone);
    
    if (CFGetTypeID(hidManager) != IOHIDManagerGetTypeID()) {
        NSLog(@"K811FnKey: fail to create IOHIDManager");
        return;
    }
    
    // Create a Matching Dictionary
    CFMutableDictionaryRef matchDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                                 2,
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks);
    
    // Specify a device manufacturer in the Matching Dictionary
    long HID_VENDOR_ID_LOGITECH = 0x046D;
    long HID_PRODUCT_ID_K811 = 0xB317;
    
    CFDictionarySetValue(matchDict,
                         CFSTR(kIOHIDVendorIDKey),
                         CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &HID_VENDOR_ID_LOGITECH ));
    
    
    CFDictionarySetValue(matchDict,
                         CFSTR(kIOHIDProductIDKey),
                         CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &HID_PRODUCT_ID_K811 ));
    
    // Register the Matching Dictionary to the HID Manager
    IOHIDManagerSetDeviceMatching(hidManager, matchDict);
    
    // We're done with the matching dictionary
    if(matchDict) CFRelease(matchDict);
    
    // Register a callback for USB device detection with the HID Manager
    IOHIDManagerRegisterDeviceMatchingCallback(hidManager, &Handle_DeviceMatchingCallback, NULL);
    
    // Register a callback fro USB device removal with the HID Manager
    IOHIDManagerRegisterDeviceRemovalCallback(hidManager, &Handle_DeviceRemovalCallback, NULL);
    
    IOHIDManagerRegisterInputValueCallback(hidManager, &Handle_InputCallback, NULL);
    
    
    // Register the HID Manager on our app’s run loop
    IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
    
    // Open the HID Manager
    IOReturn IOReturn = IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);
    if(IOReturn) NSLog(@"IOHIDManagerOpen failed.");  //  Couldn't open the HID manager! TODO: proper error handling
    NSLog(@"K811FnKey: end setting");
    button.title = @"end";

}

// New USB device specified in the matching dictionary has been added (callback function)
static void Handle_DeviceMatchingCallback(void *inContext,
                                          IOReturn inResult,
                                          void *inSender,
                                          IOHIDDeviceRef inIOHIDDeviceRef){
    
    // Retrieve the device name & serial number
    NSString *devName = [NSString stringWithUTF8String:CFStringGetCStringPtr(IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR("Product")), kCFStringEncodingMacRoman)];
    
    NSString *devSerialNumber = [NSString stringWithUTF8String:CFStringGetCStringPtr(IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR("SerialNumber")), kCFStringEncodingMacRoman)];
    
    // Log the device Name, Serial Number & device count
    NSLog(@"\nK811 device added: %p\nModel: %@\nSerial Number:%@",
          inIOHIDDeviceRef,
          devName,
          devSerialNumber);
    // populate output buffer
    // ....
    //    size_t bufferSize  = 7;
    // use f1 with fn
    // use function key without fn
    const char unsigned data[] = {0x10, 0xff, 0x06, 0x15, 0x00, 0x00, 0x00};
    
    IOReturn sendRet = IOHIDDeviceSetReport(inIOHIDDeviceRef, kIOHIDReportTypeOutput, data[0], data, sizeof(data));
    if (sendRet != kIOReturnSuccess) {
        NSLog(@"fail to send. ret=0x%08X", sendRet);
    }
    else {
        NSLog(@"Sended %lu", sizeof(data));
    }

}

// USB device specified in the matching dictionary has been removed (callback function)
static void Handle_DeviceRemovalCallback(void *inContext,
                                         IOReturn inResult,
                                         void *inSender,
                                         IOHIDDeviceRef inIOHIDDeviceRef){
    
    // Log the device ID & device count
    NSLog(@"\nK811 device removed: %p", (void *)inIOHIDDeviceRef);
    
    // TODO: make sure your application doesn't try to do anything with the removed device
}

static void Handle_InputCallback(void *inContext,
                                 IOReturn inResult,
                                 void *inSender,
                                 IOHIDValueRef inIOHIDValueRef){
    // https://developer.apple.com/library/mac/documentation/devicedrivers/conceptual/HID/new_api_10_5/tn2187.html
    // http://dragonsandbytecode.wordpress.com/2012/02/26/game-developers-diary-3-getting-in-control/
    // NSLog(@"%s(context: %p, result: %p, sender: %p, value: %p).\n",
    //      //__PRETTY_FUNCTION__
    //      "key ", inContext, (void *) inResult, inSender, (void*) inIOHIDValueRef);

    IOHIDElementRef element = IOHIDValueGetElement(inIOHIDValueRef);
    
    int usagePage = IOHIDElementGetUsagePage(element);
    int usage = IOHIDElementGetUsage(element);
    NSLog(@"usagePage=%d usage=%d value=%ld", usagePage, usage, IOHIDValueGetIntegerValue(inIOHIDValueRef));
    
}

@end

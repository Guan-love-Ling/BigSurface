//
//  SurfaceHIDDriver.cpp
//  SurfaceHID
//
//  Created by Xavier on 2022/5/16.
//  Copyright © 2022 Xia Shangning. All rights reserved.
//

#include "SurfaceHIDDriver.hpp"
#include "SurfaceHIDDevice.hpp"

#define LOG(str, ...)    IOLog("%s::" str "\n", "SurfaceHIDDriver", ##__VA_ARGS__)

#define super IOService
OSDefineMetaClassAndStructors(SurfaceHIDDriver, IOService)

void SurfaceHIDDriver::eventReceived(SurfaceHIDNub *sender, SurfaceHIDDeviceType device, UInt8 *buffer, UInt16 len) {
    switch (device) {
        case SurfaceLegacyKeyboardDevice:
        case SurfaceKeyboardDevice:
            memcpy(kbd_report, buffer, len);
            kbd_report_len = len;
            kbd_interrupt->interruptOccurred(nullptr, this, 0);
            break;
        case SurfaceTouchpadDevice:
            memcpy(tpd_report, buffer, len);
            tpd_report_len = len;
            tpd_interrupt->interruptOccurred(nullptr, this, 0);
            break;
        default:
            LOG("WTF? Unknown event type");
            break;
    }
}

void SurfaceHIDDriver::keyboardInputReceived(IOInterruptEventSource *sender, int count) {
    if (!awake)
        return;
    
    IOBufferMemoryDescriptor *report = IOBufferMemoryDescriptor::withBytes(kbd_report, kbd_report_len, kIODirectionNone);
    if (keyboard->handleReport(report) != kIOReturnSuccess)
        LOG("Handle keyboard report error!");
    
    OSSafeReleaseNULL(report);
}

void SurfaceHIDDriver::touchpadInputReceived(IOInterruptEventSource *sender, int count) {
    if (!awake)
        return;
    
    IOBufferMemoryDescriptor *report = IOBufferMemoryDescriptor::withBytes(tpd_report, tpd_report_len, kIODirectionNone);
    if (touchpad->handleReport(report) != kIOReturnSuccess)
        LOG("Handle touchpad report error!");

    OSSafeReleaseNULL(report);
}

bool SurfaceHIDDriver::init(OSDictionary *properties) {
    if (!super::init(properties))
        return false;
    
    memset(kbd_report, 0, sizeof(kbd_report));
    memset(tpd_report, 0, sizeof(tpd_report));
    return true;
}

IOService *SurfaceHIDDriver::probe(IOService *provider, SInt32 *score) {
	if (!super::probe(provider, score))
        return nullptr;

    nub = OSDynamicCast(SurfaceHIDNub, provider);
    if (!nub)
        return nullptr;
    
    OSBoolean *type = OSRequiredCast(OSBoolean, nub->getProperty(SURFACE_LEGACY_HID_STRING));
    legacy = type->getValue();
    
	return this;
}

bool SurfaceHIDDriver::start(IOService *provider) {
	if (!super::start(provider))
		return false;

    work_loop = IOWorkLoop::workLoop();
    if (!work_loop) {
        LOG("Could not create work loop!");
        goto exit;
    }
    
    kbd_interrupt = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &SurfaceHIDDriver::keyboardInputReceived));
    if (!kbd_interrupt) {
        LOG("Could not create keyboard interrupt event!");
        goto exit;
    }
    work_loop->addEventSource(kbd_interrupt);
    
    if (!legacy) {
        tpd_interrupt = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &SurfaceHIDDriver::touchpadInputReceived));
        if (!tpd_interrupt) {
            LOG("Could not create touchpad interrupt event!");
            goto exit;
        }
        work_loop->addEventSource(tpd_interrupt);
    }
    
    if (nub->registerHIDEvent(this, OSMemberFunctionCast(SurfaceHIDNub::EventHandler, this, &SurfaceHIDDriver::eventReceived)) != kIOReturnSuccess) {
        LOG("HID event registration failed!");
        goto exit;
    }
    
    // create keyboard & touchpad devices
    keyboard = OSTypeAlloc(SurfaceHIDDevice);
    if (!keyboard) {
        LOG("Could not create keyboard device");
        goto exit;
    }
    if (legacy) {
        keyboard->device = SurfaceLegacyKeyboardDevice;
        keyboard->device_name = OSString::withCString("Surface Legacy Keyboard");
        keyboard->setName("SurfaceLegacyKeyboard");
    } else {
        keyboard->device = SurfaceKeyboardDevice;
        keyboard->device_name = OSString::withCString("Surface Keyboard");
        keyboard->setName("SurfaceKeyboard");
    }
    if (!keyboard->init() || !keyboard->attach(this)) {
        LOG("Could not init keyboard device");
        goto exit;
    }
    if (!keyboard->start(this)) {
        keyboard->detach(this);
        LOG("Could not start keyboard device");
        goto exit;
    }
    
    if (!legacy) {
        touchpad = OSTypeAlloc(SurfaceHIDDevice);
        if (!touchpad) {
            LOG("Could not create touchpad device");
            goto exit;
        }
        touchpad->device = SurfaceTouchpadDevice;
        touchpad->device_name = OSString::withCString("Surface Touchpad");
        touchpad->setName("SurfaceTouchpad");
        if (!touchpad->init() || !touchpad->attach(this)) {
            LOG("Could not init Surface touchpad device");
            goto exit;
        }
        if (!touchpad->start(this)) {
            touchpad->detach(this);
            LOG("Could not start Surface touchpad device");
            goto exit;
        }
    }
    
    PMinit();
    nub->joinPMtree(this);
    registerPowerDriver(this, myIOPMPowerStates, kIOPMNumberPowerStates);
    
	return true;
exit:
    releaseResources();
    return false;
}

void SurfaceHIDDriver::releaseResources() {
    nub->unregisterHIDEvent(this);
    if (kbd_interrupt) {
        kbd_interrupt->disable();
        work_loop->removeEventSource(kbd_interrupt);
        OSSafeReleaseNULL(kbd_interrupt);
    }
    if (tpd_interrupt) {
        tpd_interrupt->disable();
        work_loop->removeEventSource(tpd_interrupt);
        OSSafeReleaseNULL(tpd_interrupt);
    }
    OSSafeReleaseNULL(work_loop);
    
    if (keyboard) {
        keyboard->stop(this);
        keyboard->detach(this);
        OSSafeReleaseNULL(keyboard);
    }
    if (touchpad) {
        touchpad->stop(this);
        touchpad->detach(this);
        OSSafeReleaseNULL(touchpad);
    }
}

void SurfaceHIDDriver::stop(IOService *provider) {
    PMstop();
    releaseResources();
    super::stop(provider);
}

void SurfaceHIDDriver::free() {
    super::free();
}

IOReturn SurfaceHIDDriver::setPowerState(unsigned long whichState, IOService *device) {
    if (device != this)
        return kIOReturnInvalid;
    if (whichState == 0) {
        if (awake) {
            awake = false;
            work_loop->disableAllEventSources();
            LOG("Going to sleep");
        }
    } else {
        if (!awake) {
            awake = true;
            work_loop->enableAllEventSources();
            LOG("Woke up");
        }
    }
    return kIOPMAckImplied;
}

IOReturn SurfaceHIDDriver::getHIDDescriptor(SurfaceHIDDeviceType device, SurfaceHIDDescriptor *desc) {
    return nub->getHIDDescriptor(device, desc);
}

IOReturn SurfaceHIDDriver::getHIDAttributes(SurfaceHIDDeviceType device, SurfaceHIDAttributes *attr) {
    return nub->getHIDAttributes(device, attr);
}

IOReturn SurfaceHIDDriver::getReportDescriptor(SurfaceHIDDeviceType device, UInt8 *buffer, UInt16 len) {
    return nub->getReportDescriptor(device, buffer, len);
}

IOReturn SurfaceHIDDriver::getRawReport(SurfaceHIDDeviceType device, UInt8 report_id, UInt8 *buffer, UInt16 len) {
    return nub->getHIDRawReport(device, report_id, buffer, len);
}

void SurfaceHIDDriver::setRawReport(SurfaceHIDDeviceType device, UInt8 report_id, bool feature, UInt8 *buffer, UInt16 len) {
    nub->setHIDRawReport(device, report_id, feature, buffer, len);
}
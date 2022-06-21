# BigSurface
The name comes from macOS Big Sur.
Big Sur + Surface -> Big Surface (LOL)

PS : If you have a better name, please let me know.

**A proposition for a fully intergrated kext for all Surface related hardwares**

## How to install

You will need to first DELETE all the original VoodooI2C series Kext, and then add the kext into opencore's `config.plist` in the order specified as below<img width="932" alt="截屏2022-01-03 下午5 43 49" src="https://user-images.githubusercontent.com/18528518/147917419-bf24e033-d4f4-43b0-b5f0-561b75c34e7c.png">

## Which Surface series are supported ?
Check your UART device id in Windows, currently device id `34a8` is supported perfectly, `9d27` still needs some ameliorations but usable
- Surface Pro 7 & Laptop 3 & Book 3     -> 34a8
- Surface Pro 5, 6 & Book 1, 2 & Laptop 1, 2    -> 9d27

See my sub-repo `VoodooSerial` for details

## What works
- Surface Type Cover
  > The code is based on VoodooI2CHID.kext, but added **integrated and hot pluggable touchpad&keyboard support**.
- Buttons
  > Power/VolumeUp/VolumeDown buttons all works properly
- Ambient Light Sensor
  > ACPI device name: ACSD, attached under I2C4
  > 
  > You can set baseline of ALI in info.plist, the calculation is `base_ali+ALI value from the sensor`
  > Currently disabled by default (in Info.plist nameMatch)
- Battery status--Surface Serial Hub **Finally works**
  > I have implemented the UART driver as well as MS's SAM module driver. Now it seems to be working fine and so far no problems are found during the test on serval SP7 devices. 
  > 
  > (Other devices **should work** but need some modifications, **post an issue** and I will try my best to help you get it working)
- Performance mode
  > Right now it is set by `PerformanceMode` in `SurfaceBattery` (default 0x01), changing it to other values is not observed to have any effects. If you find any difference (fan speed or battery life), please let me know
  > 
  > It can only be set by changing the plist or using `ioio`
  > We need a userspace software to control it if it actually has something useful.
- Surface Laptop3's keyboard & touchpad
  > Works now, all keys and gestures are recognised properly.
  > 
  > Known issue: neither keyboard nor touchpad can wake up the system.
- Touch Screen & Stylus
  > The code is ported from linux, including `mei` and surface-linux's `ipts` & `iptsd` drivers.
  > 
  > 10 finger touch & stylus
  
Possible values for Performance mode are:

      State              Value
      
    Recommended          0x01
    
    Battery Saver        0x02 (Only in battery mode)
    
    Better Performance   0x03
    
    Best Performance     0x04
    
## TODO
- Cameras                            Impossible so far
  > ACPI devices: CAMR,CAMF,CAM3(infrared camera)
  > 
  > Corresponding device id: OV8865,OV5693,OV7251
  > 
  > Even Linux failed to drive the cameras on SP7 (IPU4), SP6 and before (IPU3) might be possible but I do not have the device.


## If you like my project, please consider to `star` this project.
### If you appreciate my effort and would like to pay me a coffee, here is my PayPal address: `1750546761@qq.com`

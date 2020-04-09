# Describe

ATE TBox Api with nodejs

# Install

```shell
npm install ate_tbox
```

# Method description

##  Attribute: drivers(array)

Host driver array, driver contents include:<br>
name: device/driver name.<br>
vendor: device vendor.<br>
library: driver library path.<br>

## Method: OpenDevice(library)

This function is used to establish a connection and intialize the Pass-Thru Device and setup connect protocol channel.<br>
library: String, device library path.<br>
return: int, 0 is success.<br>

## Method: CloseDevice()

This function is used to close the connection to a Pass-Thru Device.<br>
return: int, 0 is success.<br>

## Method: SelectDrawer(index)

This function select drawer to connect.<br>
index: drawer index, range: 0 - 20, 0 is close all drawer connect.<br>

## Method: SetupDutPower(flags, limit_max, limit_min)

This function setup drawer dut power supply and protection Parameters.<br>
flags: dut power supply switch flags.<br>
limit_max: max current(mA), when the current is greater than that value, the power is off.<br>
limit_min: min current(mA), when the current is less than that value, the power is off.<br>

## Method: StartSingleTest(interval, updateCB, exitCB)

This function is used to start single test.<br>
interval: drawer refresh interval(s).<br>
updateCB: drawer update callback.<br>
exitCB: test exit callback.<br>

## Method: StartMultiTest(interval, duration, flags, updateCB, exitCB)

This function is used to start multi drawer test.<br>
interval: drawer refresh interval(s).<br>
duration: duration test time(s).<br>
flags: drawer refresh list flags.<br>
updateCB: drawer update callback.<br>
exitCB: test exit callback.<br>

## Method: StopMultiTest()

This function is used to stop multi drawer test.<br>

# Usage Example

```javascript
const AteApi = require('..');

function updateHandle(infos) {
    for(var i = 0; i < infos.duts.length; i++){
        console.log("<%d-%d>: sw[%d] avg[%d] min[%d] max[%d]", 
            infos.index,
            i, 
            infos.duts[i].sw,
            infos.duts[i].avg,
            infos.duts[i].min,
            infos.duts[i].max);
    }   
}

function testDone(infos) {
    console.log("test done");   
    /* 关闭设备 */
    if (0 != AteApi.CloseDevice()) {
        console.log("device close failure.");
    }
    console.log("device close success."); 
    //process.exit();
}

function singleTest() {
    var interval = 2;
    if (0 != AteApi.StartSingleTest(interval, updateHandle, testDone)) {
        console.log("Start Single Test failure."); 
        AteApi.CloseDevice();
        process.exit();
    } 
}

function multiTest() {
    var interval = 2;
    var duration = 30;
    var flags = 0x5;
    if (0 != AteApi.StartMultiTest(interval, duration, flags, updateHandle, testDone)) {
        console.log("Start Single Test failure."); 
        AteApi.CloseDevice();
        process.exit();
    } 
}

function doTest() {
    for(let i = 0; i < AteApi.drivers.length; i++){
        console.log("Discover Driver: ", AteApi.drivers[i].name);
    }
    /* 打开设备 */
    if (0 != AteApi.OpenDevice(AteApi.drivers[2].library)) {
        console.log("device open failure."); 
        process.exit();
    } 
    console.log("device open success."); 
    /* 选择通信抽屉 */
    if (0 != AteApi.SelectDrawer(0)) {
        console.log("Select Drawer failure."); 
        AteApi.CloseDevice();
        process.exit();
    } 
    /* 设置DUT电源 */
    if (0 != AteApi.SetupDutPower(0xFFF, 800, 200)) {
         console.log("Setup Dut Power failure."); 
         AteApi.CloseDevice();
         process.exit();
    } 
    singleTest();
    //multiTest();
}

doTest();
```
const AteApi = require('..');

function eventHandle(event) {
    if ("update" == event.event) {
        /* 更新dut信息 */
        for(var i = 0; i < event.duts.length; i++){
            console.log("<%d-%d>: sw[%d] avg[%d] min[%d] max[%d]", 
                event.index,
                i, 
                event.duts[i].sw,
                event.duts[i].avg,
                event.duts[i].min,
                event.duts[i].max);
        } 
    } else if ("exit" == event.event) {
        console.log("test done");   
        /* 关闭设备 */
        if (0 != AteApi.CloseDevice()) {
            console.log("device close failure.");
        } else {
            console.log("device close success.");   
        }
    } else {
        console.log("undefinition event %s.", event.event);  
    }
}

function singleTest() {
    var interval = 2;
    if (0 != AteApi.StartSingleTest(interval, eventHandle)) {
        console.log("Start Single Test failure."); 
        AteApi.CloseDevice();
        process.exit();
    } 
}

function multiTest() {
    var interval = 2;
    var duration = 30;
    var flags = 0x5;
    if (0 != AteApi.StartMultiTest(interval, duration, flags, eventHandle)) {
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
    // if (0 != AteApi.SelectDrawer(0)) {
    //     console.log("Select Drawer failure."); 
    //     AteApi.CloseDevice();
    //     process.exit();
    // } 
    
    /* 设置DUT电源 */
    if (0 != AteApi.SetupDutPower(0xFFF, 800, 200)) {
         console.log("Setup Dut Power failure."); 
         AteApi.CloseDevice();
         process.exit();
    } 
    //singleTest();
    multiTest();
}

doTest();
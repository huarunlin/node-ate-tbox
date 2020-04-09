#ifndef _ATE_H
#define _ATE_H

#include "PassThruDevice.h"

#define DEFAULT_TIMEOUT     500
#define DUT_COUNT           12

typedef struct {
    unsigned char  sw;
    unsigned short avg;
    unsigned short max;
    unsigned short min;
} DutInfo;

class CATE : public PassThruDevice
{
public:
    CATE();
    virtual ~CATE(); 

    virtual bool OpenDev(const char* libraryPath);
    virtual bool CloseDev();

    virtual bool SetupDutPower(unsigned short flags, unsigned short max, unsigned short min, unsigned long timeout = DEFAULT_TIMEOUT);
    virtual bool SelectDrawer(unsigned char index, unsigned long timeout = DEFAULT_TIMEOUT);
    virtual bool GetDutInfo(unsigned char index, DutInfo *info, unsigned long timeout = DEFAULT_TIMEOUT);
    virtual bool GetAllDutInfo(DutInfo *info, unsigned long timeout = DEFAULT_TIMEOUT);

private: 
    unsigned long  _frameFilter;
    unsigned long  _drawerFilter;
};

#endif
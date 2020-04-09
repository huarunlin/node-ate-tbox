#ifndef _PASSTHRUDEVICE_H
#define _PASSTHRUDEVICE_H

#include <windows.h>
#include "j2534_v0404.h"

#define IS_29BIT_ID(_id_)       (_id_ > 0x7FF)

typedef struct _PassThru_Api_ {
    HINSTANCE	_dll;
    PTOPEN		_open;
    PTCLOSE		_close;
    PTCONNECT		_connect;
    PTDISCONNECT	_disconnect;
    PTREADMSGS		_readMsgs;
    PTWRITEMSGS		_writeMsgs;
    PTSTARTPERIODICMSG	_startPeriodicMsg;
    PTSTOPPERIODICMSG	_stopPeriodicMsg;
    PTSTARTMSGFILTER	_startMsgFilter;
    PTSTOPMSGFILTER		_stopMsgFilter;
    PTSETPROGRAMMINGVOLTAGE _setProgrammingVoltage;
    PTREADVERSION	_readVersion;
    PTGETLASTERROR	_getLastError;
    PTIOCTL _ioctl;
} PassThru_Api;

class PassThruDevice 
{
public:
    PassThruDevice();
    virtual ~PassThruDevice();

    virtual bool open(const char* path);
    virtual bool close();
    virtual bool connect(unsigned long protocol,
                 unsigned long baudrate,
                 unsigned long flags,
                 bool echo = false);
    virtual bool disconnect();

    virtual bool startMsgFilter(unsigned long filterType,
                                unsigned int mask,
                                unsigned int pattern,
                                unsigned int flowControl,
                                unsigned long *filterId);
    virtual bool stopMsgFilter(unsigned long filterId);  

    virtual bool startPeriodicMsg(unsigned int canId,
                                    const unsigned char* data, 
                                    unsigned long len,
                                    unsigned long interval,
                                    unsigned long *msgId);
    virtual bool stopPeriodicMsg(unsigned long msgId);

    virtual bool send(unsigned int canId,
                        const unsigned char* data, 
                        unsigned long len,
                        unsigned long timeout);

    virtual bool recv(unsigned int *canId,
                        unsigned long *protocol,
                        unsigned char **data, 
                        unsigned long *len,
                        unsigned long *flags,
                        unsigned long timeout);
protected:
    void lock();
    void unlock();
    
    bool loadApi(const char* path);
    bool unloadApi();

    HANDLE		   _mutex;
    bool           _isOpen;
    bool           _isConnect;
    
    PassThru_Api   _api;
    unsigned long  _device;
    unsigned long  _channel;
    unsigned long  _protocol;

    PASSTHRU_MSG    _maskMsg;
    PASSTHRU_MSG    _patternMsg;
    PASSTHRU_MSG    _flowControlMsg;
    PASSTHRU_MSG    _readMsg;
    PASSTHRU_MSG    _writeMsg;
};

#endif // _PASSTHRUDEVICE_H

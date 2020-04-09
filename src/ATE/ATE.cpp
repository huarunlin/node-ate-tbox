#include "ATE.h"
#include "Common.h"

#define CANID_FRAME_HOST   0x7C5
#define CANID_FRAME_ECU    0x7CD
#define CANID_DRAWER_HOST  0x7C6
#define CANID_DRAWER_ECU   0x7CE

#define SID_WRITE_DATA_BY_IDENTIFIER  0x2E
#define SID_READ_DATA_BY_IDENTIFIER   0x22

#define FRAME_DID_DRAWER_SELECT  0x0110
#define DRAWER_DID_DUT_SWITCH    0x0110
#define DRAWER_DID_DUT_INFO      0x0111
#define DRAWER_DID_ALLDUT_INFO   0x0120

CATE::CATE() : PassThruDevice()
{

}

CATE::~CATE()
{
    CloseDev();
}

bool CATE::OpenDev(const char* libraryPath)
{
	bool ret = false;

	if (_isOpen) {
		tr_err("<%s> Device is open.\r\n", __FUNCTION__); 
		return false;
	}

	ret = open(libraryPath); 
	if (true != ret) {
		tr_err("<%s> Device open failure.\r\n", __FUNCTION__); 
		return false;
	}

	ret = connect(ISO15765, 500000, CAN_ID_BOTH);
	if (true != ret) {
		close();
		tr_err("<%s> Bus connect failure.\r\n", __FUNCTION__); 
		return false;
	}
	
	ret = startMsgFilter(FLOW_CONTROL_FILTER, 0x7FF, CANID_FRAME_ECU, CANID_FRAME_HOST, &_frameFilter);
	if (true != ret) {
		disconnect();
		close();
		tr_err("<%s> Start frame msg filter failure.\r\n", __FUNCTION__); 
		return false;
	}
	
	ret = startMsgFilter(FLOW_CONTROL_FILTER, 0x7FF, CANID_DRAWER_ECU, CANID_DRAWER_HOST, &_drawerFilter);
	if (true != ret) {
		disconnect();
		close();
		tr_err("<%s> Start drawer msg filter failure.\r\n", __FUNCTION__); 
		return false;
	}
	return true;
}

bool CATE::CloseDev()
{
	if (_isOpen) {
		stopMsgFilter(_drawerFilter);
		stopMsgFilter(_frameFilter);
		disconnect();
		close();
	}
	return true;
}

bool CATE::SelectDrawer(unsigned char index, unsigned long timeout) {
	bool ret = false;
    unsigned char request[] = { SID_WRITE_DATA_BY_IDENTIFIER, 
								(unsigned char)(FRAME_DID_DRAWER_SELECT >> 8), 
								(unsigned char)(FRAME_DID_DRAWER_SELECT), 
								index };
	unsigned char* respond; 
	unsigned int canId;
	unsigned long protocol, len, rxflags;
	unsigned short did;

    /* 发送设置请求 */
	ret = send(CANID_DRAWER_HOST, request, sizeof(request), timeout);
	if (true != ret) {
        tr_err("<%s> Send request failure.\r\n", __FUNCTION__);
        return false;
    }
    /* 等待回复 */
    while (1) {
        ret = recv(&canId, &protocol, &respond, &len, &rxflags, timeout);
		if (true != ret) {
            tr_err("<%s> Recv respond failure.\r\n", __FUNCTION__);
        	return false;
        }

        if ((TX_MSG_TYPE | START_OF_MESSAGE) & rxflags) {
            continue; //进入下一次循环
        }

        if (len != 3) {
            tr_err("<%s> Invalid respond length %d\r\n", __FUNCTION__, len);
        	return false;
        }

        if (respond[0] != (SID_WRITE_DATA_BY_IDENTIFIER | 0x40)) {
            tr_err("<%s> Invalid SID 0x%02X\r\n", __FUNCTION__, respond[0]);
         	return false;
        }

        did = (respond[1] << 8) | respond[2];
        if (FRAME_DID_DRAWER_SELECT != did) {
            tr_err("<%s> Invalid respond did 0x%04X\r\n", __FUNCTION__, did);
         	return false;
        }
        return true;
    }
}

bool CATE::SetupDutPower(unsigned short flags, unsigned short max, unsigned short min, unsigned long timeout)
{
	bool ret = false;
	unsigned char request[] = { SID_WRITE_DATA_BY_IDENTIFIER, 
								(unsigned char)(DRAWER_DID_DUT_SWITCH >> 8),
								(unsigned char)(DRAWER_DID_DUT_SWITCH),
								(unsigned char)(flags >> 8),
								(unsigned char)(flags),
								(unsigned char)(max >> 8),
								(unsigned char)(max),
								(unsigned char)(min >> 8),
								(unsigned char)(min) };
	unsigned char* respond; 
	unsigned int canId;
	unsigned long protocol, len, rxflags;
	unsigned short did;
	/* 发送请求 */
	ret = send(CANID_DRAWER_HOST, request, sizeof(request), timeout);
	if (true != ret) {
        tr_err("<%s> Send request failure.\r\n", __FUNCTION__);
        return false;
    }
	/* 等待回复 */
    while (1) {
        ret = recv(&canId, &protocol, &respond, &len, &rxflags, timeout);
		if (true != ret) {
            tr_err("<%s> Recv respond failure.\r\n", __FUNCTION__);
        	return false;
        }

        if ((TX_MSG_TYPE | START_OF_MESSAGE) & rxflags) {
            continue; //进入下一次循环
        }

        if (len != 3) {
            tr_err("<%s> Invalid respond length %d\r\n", __FUNCTION__, len);
        	return false;
        }

        if (respond[0] != (SID_WRITE_DATA_BY_IDENTIFIER | 0x40)) {
            tr_err("<%s> Invalid SID 0x%02X\r\n", __FUNCTION__, respond[0]);
         	return false;
        }

        did = (respond[1] << 8) | respond[2];
        if (DRAWER_DID_DUT_SWITCH != did) {
            tr_err("<%s> Invalid respond did 0x%04X\r\n", __FUNCTION__, did);
         	return false;
        }
        return true;
    }
}

bool CATE::GetDutInfo(unsigned char index, DutInfo *info, unsigned long timeout)
{
	bool ret = false;
	unsigned char request[] = { SID_READ_DATA_BY_IDENTIFIER, 
								(unsigned char)(DRAWER_DID_DUT_INFO >> 8),
								(unsigned char)(DRAWER_DID_DUT_INFO + index) };
	unsigned char* respond; 
	unsigned int canId;
	unsigned long protocol, len, rxflags;
	unsigned short did;
	/* 发送请求 */
	ret = send(CANID_DRAWER_HOST, request, sizeof(request), timeout);
	if (true != ret) {
        tr_err("<%s> Send request failure.\r\n", __FUNCTION__);
        return false;
    }
	/* 等待回复 */
    while (1) {
        ret = recv(&canId, &protocol, &respond, &len, &rxflags, timeout);
		if (true != ret) {
            tr_err("<%s> Recv respond failure.\r\n", __FUNCTION__);
        	return false;
        }

        if ((TX_MSG_TYPE | START_OF_MESSAGE) & rxflags) {
            continue; //进入下一次循环
        }

        if (len != 10) {
            tr_err("<%s> Invalid respond length %d\r\n", __FUNCTION__, len);
        	return false;
        }

        if (respond[0] != (SID_READ_DATA_BY_IDENTIFIER | 0x40)) {
            tr_err("<%s> Invalid SID 0x%02X\r\n", __FUNCTION__, respond[0]);
         	return false;
        }

        did = (respond[1] << 8) | respond[2];
        if ((DRAWER_DID_DUT_INFO + index) != did) {
            tr_err("<%s> Invalid respond did 0x%04X\r\n", __FUNCTION__, did);
         	return false;
        }

		info->sw = respond[3];		/* 电源开关状态，备注：电源状态为关闭，并且该DUT需要测试时，代表当前DUT异常 */
        info->avg = (respond[4] << 8) | respond[5];  /* 平均电流 */
        info->max = (respond[6] << 8) | respond[7];  /* 最大电流 */
        info->min = (respond[8] << 8) | respond[9];  /* 最小电流 */
        return true;
    }	
}

bool CATE::GetAllDutInfo(DutInfo *info, unsigned long timeout)
{
	bool ret = false;
	unsigned char request[] = { SID_READ_DATA_BY_IDENTIFIER, 
								(unsigned char)(DRAWER_DID_ALLDUT_INFO >> 8),
								(unsigned char)(DRAWER_DID_ALLDUT_INFO) };
	unsigned char* respond; 
	unsigned int canId;
	unsigned long protocol, len, rxflags;
	unsigned short did;
	/* 发送请求 */
	ret = send(CANID_DRAWER_HOST, request, sizeof(request), timeout);
	if (true != ret) {
        tr_err("<%s> Send request failure.\r\n", __FUNCTION__);
        return false;
    }
	/* 等待回复 */
    while (1) {
        ret = recv(&canId, &protocol, &respond, &len, &rxflags, timeout);
		if (true != ret) {
            tr_err("<%s> Recv respond failure.\r\n", __FUNCTION__);
        	return false;
        }

        if ((TX_MSG_TYPE | START_OF_MESSAGE) & rxflags) {
            continue; //进入下一次循环
        }

        if (len != (3 + DUT_COUNT * 7)) {
            tr_err("<%s> Invalid respond length %d\r\n", __FUNCTION__, len);
        	return false;
        }

        if (respond[0] != (SID_READ_DATA_BY_IDENTIFIER | 0x40)) {
            tr_err("<%s> Invalid SID 0x%02X", __FUNCTION__, respond[0]);
         	return false;
        }

        did = (respond[1] << 8) | respond[2];
        if (DRAWER_DID_ALLDUT_INFO != did) {
            tr_err("<%s> Invalid respond did 0x%04X\r\n", __FUNCTION__, did);
         	return false;
        }

        for(int pos = 3, index = 0; index < DUT_COUNT; pos = pos +7, index++) {
			info[index].sw = respond[pos];		/* 电源开关状态，备注：电源状态为关闭，并且该DUT需要测试时，代表当前DUT异常 */
        	info[index].avg = (respond[pos + 1] << 8) | respond[pos + 2];  /* 平均电流 */
        	info[index].max = (respond[pos + 3] << 8) | respond[pos + 4];  /* 最大电流 */
        	info[index].min = (respond[pos + 5] << 8) | respond[pos + 6];  /* 最小电流 */
		}
        return true;
    }	
}
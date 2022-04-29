#include "Device.h"

#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
//#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#define SHM_BUFF  50

HDCallbackCode HDCALLBACK copyDeviceDataCallback(void *pUserData) {
    Device *device = (Device *) pUserData;
    device->m_simuData = device->getHapticData();
    return HD_CALLBACK_CONTINUE;

}

HDCallbackCode HDCALLBACK stateCallback(void *pUserData) {
    Device *device = (Device *) pUserData;
    device->getFromDevice();
    return HD_CALLBACK_CONTINUE;
}
void Device::initDevice() {
    HDSchedulerHandle hStateHandle = HD_INVALID_HANDLE;
    if (s_schedulerRunning) // need to stop scheduler if already running before any init
    {
        hdStopScheduler();
        s_schedulerRunning = false;
    }
    m_hHD = hdInitDevice(HD_DEFAULT_DEVICE);

    if (hdCheckCalibration() != HD_CALIBRATION_OK) {
        // Possible values are : HD_CALIBRATION_OK || HD_CALIBRATION_NEEDS_UPDATE || HD_CALIBRATION_NEEDS_MANUAL_INPUT
        std::cout << "GeomagicDriver initialisation failed because device is not calibrated. "<<std::endl;
        return;
    }
    hdStartScheduler();
    s_schedulerRunning = true;
    hStateHandle = hdScheduleAsynchronous(stateCallback, this, HD_MAX_SCHEDULER_PRIORITY);
    m_hStateHandles.push_back(hStateHandle);

    hStateHandle = hdScheduleAsynchronous(copyDeviceDataCallback, this, HD_MIN_SCHEDULER_PRIORITY);
    m_hStateHandles.push_back(hStateHandle);
}

void Device::getFromDevice() {
    hdMakeCurrentDevice(m_hHD);
    hdBeginFrame(m_hHD);
    hdGetDoublev(HD_CURRENT_TRANSFORM, m_hapticData.transform);
    hdGetDoublev(HD_CURRENT_JOINT_ANGLES, m_hapticData.angle1);
    hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, m_hapticData.angle2);
    hdGetIntegerv(HD_CURRENT_BUTTONS, &m_hapticData.buttonState);
    hdEndFrame(m_hHD);
}

void Device::clearDevice(){

    hdMakeCurrentDevice(m_hHD);

    if (!m_hStateHandles.empty() && s_schedulerRunning) {
        hdStopScheduler();
        s_schedulerRunning = false;
    }

    for (auto schedulerHandle: m_hStateHandles) {
        if (schedulerHandle != HD_INVALID_HANDLE) {
            hdUnschedule(schedulerHandle);
        }
    }

    m_hStateHandles.clear();

    // clear device if valid
    if (m_hHD != HD_INVALID_HANDLE) {
        hdDisableDevice(m_hHD);
        m_hHD = HD_INVALID_HANDLE;
    }
}

void Device::createShm(){
    m_ShmData = new ShmData(this);
}
void Device::deleteShm() {
    delete m_ShmData;
}


Device::DeviceData Device::getHapticData() {
    return m_hapticData;
}

const Device::MHHD Device::getHHD() {
    return m_hHD;
}

Device::~Device() {
    if(m_ShmData)
        deleteShm();
    clearDevice();
}

Device::ShmData::ShmData(Device * const THIS):
parent(THIS),
key(ftok("/usr/bin", 2022)),
shmid(shmget(key, SHM_BUFF, IPC_CREAT|0666)),
shmadd((char*)shmat(shmid, 0, 0))
{
    if(shmadd == (void*)(-1))
    {
        perror("shmat");
        _exit(-1);
    }
    bzero(shmadd, SHM_BUFF);
}

Device::ShmData::~ShmData()
{
    shmctl(shmid, IPC_RMID, NULL);
}

void Device::ShmData::write(){

    std::string shared_vals;
    for(int i = 0; i < 16; i++) {
        double pos = parent->m_simuData.transform[i];
        shared_vals.append(std::to_string(pos));
        shared_vals.push_back(' ');
    }
    for(int i = 0; i < 3; i++) {
        double pos = parent->m_simuData.angle1[i];
        shared_vals.append(std::to_string(pos));
        shared_vals.push_back(' ');
    }
    for(int i = 0; i < 3; i++) {
        double pos = parent->m_simuData.angle2[i];
        shared_vals.append(std::to_string(pos));
        shared_vals.push_back(' ');
    }

    shared_vals.append(std::to_string(parent->m_simuData.buttonState));
    strcpy(shmadd,shared_vals.c_str());

}
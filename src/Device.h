#ifndef SRC_GETSTATUS_H
#define SRC_GETSTATUS_H
#include <HD/hd.h>
#include <string>
#include <vector>
class Device{
    typedef HDdouble MHDdouble;
    typedef HHD MHHD;
    typedef HDSchedulerHandle MHDSchedulerHandle;
public:
    struct DeviceData
    {
        MHDdouble angle1[3];
        MHDdouble angle2[3];
        MHDdouble transform[16];
        int buttonState;
    };
    class ShmData
    {
    public:
        ShmData(Device * const THIS);
        ~ShmData();
        void write();
        const key_t key;
        const int shmid;
        char* shmadd;
        Device * const parent;
    };
    Device(){};
    ~Device();
    void initDevice();
    void getFromDevice();
    void createShm();
    void deleteShm();
    void clearDevice();
    const MHHD getHHD();
    DeviceData getHapticData();
    DeviceData m_simuData;
    ShmData* m_ShmData;

private:
    DeviceData m_hapticData;
    MHHD m_hHD;
    inline static bool s_schedulerRunning = false;
    std::vector< MHDSchedulerHandle > m_hStateHandles; ///< List of ref to the workers scheduled
};

#endif //SRC_GETSTATUS_H

#define MODE_SET 0
#define MODE_MANUAL 1

#ifndef DEV_NAME_CLASS
#define DEV_NAME_CLASS

class DeviceName {
    public:
        DeviceName();
        void print();
        char* get();

    private:
        char deviceNameStr[32] = {'W', 'i', 'F', 'i', '-', 'P', 'o', 't', 'i', '-', 'F', 'a', 'n', '-', 
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

#endif

void initWifiAP();
char* getMqttServer();
unsigned long getMqttPort();
char* getMqttUsername();
char* getMqttPassword();
void loopWifiChecks();
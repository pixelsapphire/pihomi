#include "devices.hpp"

Clock::Clock(std::string port): arduino(phm::serial_port(port,9600))
{
    connected=true;
    if(!arduino.is_open())
    {
        std::cerr << "Could not open serial port on " << port << '\n';
        exit(-1);
    }
    clockThread = std::thread(&Clock::update,this);
    
}

void Clock::getTime()
{
    time_t utime = std::time(NULL);
    char buf[40];
    //std::strftime(buf,sizeof(buf),"%H:%M",std::localtime(&utime)); Imagine używać normalengo zegara
    std::strftime(buf,sizeof(buf),"%I:%M%p\n",std::localtime(&utime));
    time = buf;

}

void Clock::update()
{
    while(serverRunning)
    {
        if(connected)
        {
            getTime();
            status=updateTime();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));   
        }

    }

}

bool Clock::updateTime()
{
    arduino.write(time);
    std::string msg = arduino.read();//std::cout << time << std::endl;
    if(msg.size()>0)
    {
        if(msg.substr(0,2)=="OK")
        {
            if(msg[2]=='1')
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool Clock::getConnected()
{
    return 1;
}

int Clock::getHours()
{
    std::string hour = time.substr(0,2);
    return std::stoi(hour);
}

int Clock::getMinutes()
{
    std::string minutes = time.substr(3,2);
    return std::stoi(minutes);
}

Clock::~Clock()
{
    arduino.close();
    if(clockThread.joinable())
    {
        clockThread.join();
    }
}

bool WaterThingy::getConnected()
{
    return 1;
}


int WaterThingy::getWaterLevel()
{
    //Do communication
    return 4;
}

DeviceController::DeviceController(std::string port): clock(Clock(port))
{

}
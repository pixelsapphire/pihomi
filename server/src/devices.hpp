#pragma once
//Need some raspberry stff
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <thread>
#include <chrono>

#include "serial.hpp"

extern bool serverRunning;

struct Clock
{
    std::string _port;
    phm::serial_port arduino;
    bool connected;
    bool status;
    std::string time;
    std::thread clockThread;
    Clock(std::string port);
    ~Clock();
    void getTime();
    void update();
    bool updateTime();
    void setClock();
    bool getConnected();
    int getHours();
    int getMinutes();
};

struct WaterThingy
{
    bool connected;
    int waterLevel;
    float frequency;
    int amount;
//Check if connected
    bool getConnected(); 
//Check water level
    int getWaterLevel();
    void setFrequency(float f);
//send if connected
//send water level

};

struct Outlet
{
    bool connected;
//
};

struct DeviceController
{
    DeviceController(std::string port);
    Clock clock;
//
};



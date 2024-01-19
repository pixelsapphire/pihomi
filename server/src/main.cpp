#include <string>

#include "server.hpp"
#include "devices.hpp"

bool serverRunning=false;
int ssock;
void ctrl_c(int)
{
    write(ssock,"stopping",9);
    serverRunning=false;
}

struct Test
{
    std::thread ttrd;
    ~Test()
    {
        ttrd.join();
    }
    void tf()
    {
        std::cout << "UwU\n";
        for(int i = 0 ; i<5;i++)
        {
            std::cout << i <<std::endl;
        }
    }
    void uwu()
    {
        std::cout << "UwU\n";
        ttrd = std::thread(&Test::tf,this);
    }
};

int main(int argc,char** argv)
{
    if (argc < 2) {
        std::cerr << "Serial port has to be specified.\n";
        return -1;
    }
    serverRunning=true;
    signal(SIGINT, ctrl_c);
    Server server(std::stoi("3141"),argv[1]);
    ssock=server.sock();
    while (serverRunning)
    {

    }
    
    return 0;
}

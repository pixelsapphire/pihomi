#include "client.hpp"


//Client::Client(int cfd,std::vector<Client*> &cvec): fd(cfd), clients(cvec)
//{
//    connected=1;
//    clientThread = std::thread(&Client::updateClient,this);
//
//}

Client::Client(int fd,int epollfd,phm::controller &controller): _fd(fd), _epollFd(epollfd), _controller(controller)
{
    epoll_event ee {EPOLLIN | EPOLLRDHUP,{.ptr=this}};
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, _fd, &ee);

}
Client::~Client()
{
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, _fd, nullptr);
    shutdown(_fd, SHUT_RDWR);
    close(_fd);
}

void Client::write(std::string msg)
{
    ::write(_fd,msg.c_str(),msg.size());
    std::cout << msg;
}

void Client::sendCurrentState()
{
    //az;ap;ag;g1;g2;g3;g4;wl;pf;pv;
    std::stringstream ss;
    ss <<_controller.get_clock().get_state() <<";";
    ss <<_controller.get_irrigation().get_state()<<";";
    ss <<_controller.outlets_state()<<";";
    if(_controller.outlets_state())
    {
        for(int i=0;i<4;i++)
        {
            ss <<_controller.get_outlet(i).get_state()<<";";
        }
    }
    ss <<uint16_t(_controller.get_irrigation().get_water_level())<<";";
    ss <<_controller.get_irrigation().get_watering_delay()<<";";
    ss <<_controller.get_irrigation().get_watering_volume()<<"\n";
    std::string msg = ss.str();
    write(msg);    
}

void Client::remove(std::vector<Client*> &clients)
{
    std::erase(clients,this);
    delete this;
}

std::string Client::read()
{
    char buf[32]={0};
    ::read(_fd,buf,32);
    std::string buffer=buf;
    //buffer=buffer.substr(0,buffer.find('\n')+1);
    std::cout<<buf;
    return buffer;
}

void Client::handleEvent(uint32_t events,std::vector<Client*> &clients,std::array<bool,4> &outlets)
{
    if(events & EPOLLIN) 
    {
        std::string buffer = read();
        handleMessage(buffer,outlets);
        sendCurrentState();
    }
    if(events & ~EPOLLIN)
    {
        remove(clients);
    }
}
void Client::handleMessage(std::string&msg,std::array<bool,4> &outlets)
{

    if(msg=="o\n")
    {
        std::cout<<"Gniazdka coś\n";
        if(_controller.outlets_state())
        {
            for(int i=0;i<4;i++)
            {
                outlets[i]=_controller.get_outlet(i).get_state();
                _controller.get_outlet(i).set_state(false);
            }
        }
        else
        {
            for(int i=0;i<4;i++)
                _controller.get_outlet(i).set_state(outlets[i]);
        }
        _controller.set_outlets_state(!_controller.outlets_state());
    }


}
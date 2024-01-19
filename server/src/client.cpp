#include "client.hpp"


Client::Client()
{
    connected=1;
    clientThread = std::thread(&Client::updateClient,this);

}

Client::~Client()
{

}

void Client::updateClient()
{
    while (connected)
    {
        
    }
    
}
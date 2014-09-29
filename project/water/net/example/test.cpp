#include "listener.h"
#include "connector.h"
#include "connection.h"
#include "epoller.h"
#include "net_exception.h"

#include <iostream>
#include <string>
#include <memory>

#include <unistd.h> //usleep

using namespace std;

using namespace water;

const int16_t port = 1024;

#ifdef TEST_SERVER
enum class EpollSwitch { ON, OFF, };
int main()
{
    try
    {
        cout << "server is running ..." << endl;

        auto epoller = net::Epoller::create();
        auto listener = net::TcpListener::create();
        listener->bind(port);
        listener->listen();
        epoller->regSocket(listener.get(), net::Epoller::EventType::READ);

        EpollSwitch es = EpollSwitch::ON;

        net::TcpConnection::Ptr conn;
        auto listenerAccept = [&]()
        {
            conn = listener->accept();

            auto connectionRead = [&]()
            {
                char buf[128] = {0};
                uint32_t recvLen = conn->recv(buf, sizeof(buf));
                if(recvLen == 0 || recvLen == uint32_t(-1))
                {
                    conn->shutdown();
                    es = EpollSwitch::OFF;
                }
                cout << recvLen << ": " << buf << endl;
            };

            auto connectionError = [&]()
            {
                cout << "shutdown connection" << endl;
                conn->shutdown();
                es = EpollSwitch::OFF;
            };

            conn->setEpollReadCallback(connectionRead);
            conn->setEpollErrorCallback(connectionError);

            epoller->regSocket(conn.get(), net::Epoller::EventType::READ);
            epoller->regSocket(conn.get(), net::Epoller::EventType::ERROR);


            cout << "accept a client:" << conn->getRemoteEndpoint().ip
            << "[" << conn->getRemoteEndpoint().port << "]" << endl;
        };
        listener->setEpollReadCallback(listenerAccept);


        while(es == EpollSwitch::ON)
        {
            epoller->wait(0);
            ::usleep(0);
        }
    }
    catch(net::NetException& ex)
    {
        cerr << ex.what() << endl;
    }
    catch(...)
    {
        cerr << "unknown err" << endl;
    }
    return 0;
}

#endif

#ifdef TEST_CLIENT

int main()
{
    try
    {
        auto epoller = net::Epoller::create();
        auto connector = net::TcpConnector::create("127.0.0.1", port);
        cout << "connect to server ..." << endl;
        auto connection = connector->connect();
        cout << "connected to server:" << connection->getRemoteEndpoint().ip << "[" << connection->getRemoteEndpoint().port << "]" << endl;
        char msg[] = "hello socket";
        connection->send(msg, sizeof(msg));
        ::sleep(5);
        connection->shutdown();
    }
    catch(net::NetException& ex)
    {
        cerr << ex.what() << endl;
    }
    catch(...)
    {
        cerr << "unknown err" << endl;
    }
    return 0;
}

#endif
/*
#ifdef TEST_SERVER_EPOLL

class Server
{
public:
    void run();

private:
    void readHanlder(net::TcpSocket* socket)
    {
        if(socket == listener.get())
        {
            auto conn = listener->accept();
            if(conn == nullptr)
                return;

            connection.insert(conn);
        }
        else
        {
            auto conn = reinterpret_cast<net::TcpConnection*>(socket);
            char buf[128];
            conn.read(buf, 127);
            cout << buf << endl;
        }
    }

    void writeHanlder(net::TcpSocket* socket)
    {
    }

    void ErrorHanlder(net::TcpSocket* socket)
    {
    }

    void run()
    {
        std::thread
    }



private:
    net::Epoller::Ptr epoller;
    net::TcpListener::Ptr listener;
    std::vector<net::TcpConnection::Ptr> connections;
};

class Client
{
    void readHanlder(net::TcpSocket* socket)
    {
    }

    void writeHanlder(net::TcpSocket* socket)
    {
    }

    void errorHanlder(net::TcpSocket* socket)
    {
    }

    void run()
    {
    }

private:
    net::Epoller::Ptr epoller;
    net::TcpConnector::Ptr connector;
    std::unordered_set<net::TcpConnection::Ptr> connnections;
};

class Server
{
};

int main()
{
    try
    {
        net::TcpConnection connection;
        auto epoller = net::Epoller::create();
        auto connReadCallback = [&]()
        {

        };

        auto listener = net::TcpListener::create();
        listener->bind(port);
        listener->setEpollReadCallback([&](Socket* socket)
        {
//            net::TcpListener* listener = static_cast<net::TcpListener*>(socket);
            try
            {
                connection = listenner->accept();
                epoller->regSocket(connection.get(), net::Epoller::READ);
                epoller->regSocket(connection.get(), net::Epoller::ERROR);

                cout << "accept a client:" << connection->getRemoteEndpoint().ip
                << "[" << connection->getRemoteEndpoint().port << "]" << endl;
            }
            catch(net::NetException ex)
            {
                cerr << ex.what() << endl;
            }
        }
        net::TcpSocket::EpollCallback readCallback
            else
            {
                auto connection = reinterpret_cast<TcpConnection*>(socket);
            }
        });


        epoller->addSocket(listenner.get(), Epoller::EventType::READ);


        net::TcpListener::EventCallback writeCallback = [](int32_t socketfd)
        {
        }

        net::TcpListener::EventCallback errorCallback = [](int32_t socketfd)
        {
        }

        epoller->regReadCallback(readCallback);
        epoller->regWriteCallback(readCallback);
        epoller->regErrorCallback(readCallback);

        while(true)
        {
            epoller->wait(1);
        }

    }
    catch(net::NetException& ex)
    {
    }
    catch(...)
    {
        cerr << "unknown err" << endl;
    }
    return 0;
}

#endif

#ifdef TEST_CLIENT_EPOLL
#endif
*/

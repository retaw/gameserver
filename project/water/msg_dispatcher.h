#include <map>
#include <functional>



typedef std::function<void (Message*)> MsgHanlder;


class MessageDispatcher
{
public:
    MessageDispatcher();
    ~MessageDispatcher();

private:
    std::map<uint64_t, MsgHanlder> 
};

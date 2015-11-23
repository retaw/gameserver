#include <vector>
#include <iostream>
using namespace std;


#define SENQUENTIAL_CONTAINER_OSTREAM_OPERATOR(Container) \
template<typename T>\
std::ostream& operator << (std::ostream& os, const Container<T>& con)\
{\
    os << "{";\
    for(const auto& item : con)\
    os << item << " ";\
    os << "}";\
    return os;\
}

SENQUENTIAL_CONTAINER_OSTREAM_OPERATOR(std::vector)

int main()
{
    std::vector<int> vec {1,2,3};
    cout << vec;
}

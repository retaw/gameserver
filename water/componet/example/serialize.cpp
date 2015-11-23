#include "test.h"

#include "../serialize.h"
using namespace water;
using namespace componet;
using namespace test;

#define STRING_BUF

class S
{
template<typename Buffer>
friend Serialize<Buffer>& operator << (Serialize<Buffer>& ss, const S& s);

template<typename Buffer>
friend Deserialize<Buffer>& operator >> (Deserialize<Buffer>& ss, S& s);

friend std::ostream& operator << (std::ostream& os, const S& s);

public:
    S(int i_ = 0)
    {
        memset(this, 0, sizeof(*this));
        i = i_;
    }

    void set(int a)
    {
        i = a;
    }

    void print() const
    {
        cout << i << endl;
    }
private:
    int i;
};

std::ostream& operator << (std::ostream& os, const S& s)
{
    os << "S:" << s.i;
    return os;
}

template<typename Buffer>
Serialize<Buffer>& operator << (Serialize<Buffer>& ss, const S& s)
{
    ss << s.i;
    return ss;
}

template<typename Buffer>
Deserialize<Buffer>& operator >> (Deserialize<Buffer>& ss, S& s)
{
    ss >> s.i;
    return ss;
}




void serializeTest()
{
#ifdef STRING_BUF
    typedef Serialize<std::string> SS;
    typedef Deserialize<std::string> DS;
#else
    char memBuffer[1024];
    typedef Serialize<RawBuffer> SS;
    typedef Deserialize<RawBuffer> DS;
#endif
    RawBuffer buffer(memBuffer, 1024);
    SS ss(&buffer);

    {
        ss.reset();
        typedef std::string TA;
        TA a1{"1234"};
        ss << a1;
        cout << (a1);
        cout << endl;

        TA a2;
        DS ds(ss.buffer());
        ds >> a2;
        cout << (a2);
        cout << endl;
    }
    
    {
        ss.reset();
        typedef std::vector<int> TB;
        TB b1{1, 2, 3, 4, 5};
        ss << b1;
        cout << (b1);
        cout << endl;

        TB b2;
        DS ds(ss.buffer());
        ds >> b2;
        cout << (b2);
        cout << endl;
    }
    
    {
        ss.reset();
        typedef std::vector<std::set<std::string>> TC;
        TC c1{{"a", "b"}, {"c", "d"}};
        ss << c1;
        cout << (c1);
        cout << endl;

        TC c2;
        DS ds(ss.buffer());
        ds >> c2;
        cout << (c2);
        cout << endl;
    }

    {
        ss.reset();
        typedef std::vector<std::vector<int>> TD;
        TD d1{};
        cout << (d1);
        cout << endl;

        TD d2;
        DS ds(ss.buffer());
        ds >> d2;
        cout << (d2);
        cout << endl;
    }

    {
        ss.reset();
        typedef std::vector<std::pair<std::set<int>, std::string>> TE;
        TE e1
        {
            {{1, 2}, "12"}, 
            {{3, 4}, "34"}
        };
        ss << e1;
        cout << (e1);
        cout << endl;

        TE e2;
        DS ds(ss.buffer());
        ds >> e2;
        cout << (e2);
        cout << endl;
    }

    {
        ss.reset();
        typedef std::list<std::list<int>> TF;
        TF f1{{1,2,3}, {4,5,6}};
        ss << f1;
        cout << (f1);
        cout << endl;

        TF f2;
        DS ds(ss.buffer());
        ds >> f2;
        cout << (f2);
        cout << endl;
    }
   
    {
        ss.reset();
        typedef std::vector<S> TG;
        TG g1{1};
        ss << g1;
        cout << g1;
        cout << endl;

        TG g2;
        DS ds(ss.buffer());
        ds >> g2;
        cout << (g2);
        cout << endl;
    }

    {
        ss.reset();
        typedef std::forward_list<std::pair<int, std::string>> TH;
        TH h1{{1,"abc"}, {3,"def"}};
        ss << h1;
        cout << h1;
        cout << endl;

        TH h2;
        DS ds(ss.buffer());
        ds >> h2;
        cout << h2;
        cout << endl;
    }
}

int main()
{
    serializeTest();
}


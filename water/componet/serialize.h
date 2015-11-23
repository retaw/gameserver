/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-08-09 16:38 +0800
 *
 * Description: 通用序列化机制
 */

#ifndef WATER_COMPONET_SERIALIZE_STREAM_H
#define WATER_COMPONET_SERIALIZE_STREAM_H

#include <type_traits>
#include <cstring>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <forward_list>
#include <deque>


namespace water{
namespace componet{

template<typename Buffer>
class Serialize
{
public:
    typedef Buffer buffer_type;
    typedef typename Buffer::size_type size_type;

    explicit Serialize(buffer_type* buffer)
    : m_buffer(buffer), m_holdBuffer(false)
    {
    }

    explicit Serialize()
    : m_buffer(new buffer_type), m_holdBuffer(true)
    {
    }

    ~Serialize()
    {
        if(m_holdBuffer)
            delete m_buffer;
    }

    //non-copyable
    Serialize(Serialize& other) = delete;
    Serialize& operator = (Serialize& other) = delete;

    void assignOutBuffer(const void* buf, uint32_t bufSize)
    {
        typename buffer_type::value_type* p = (typename buffer_type::value_type*)buf;
        reset();
        m_buffer->assign(p, bufSize);
    }

    const buffer_type* buffer() const
    {
        return m_buffer;
    }

    size_type copy(void* buf, uint32_t bufSize)
    {
        return m_buffer->copy(reinterpret_cast<typename buffer_type::value_type*>(buf), bufSize);
    }

    size_type tellp() const
    {
        return m_opos;
    }

    void reset()
    {
        m_buffer->clear();
        m_opos = 0;
    }

    void reset(buffer_type* buffer)
    {
        if(m_holdBuffer)
            delete m_holdBuffer;

        m_holdBuffer = false;
        m_buffer = buffer;
        m_opos = 0;
    }

    //std::string
    Serialize& operator << (const std::string& str)
    {
        (*this) << str.size();

        m_buffer->append((const typename buffer_type::value_type*)str.data(), str.size());
        m_opos += str.size();

        return *this;
    }

    //std::pair
    template <typename T1, typename T2>
    Serialize& operator << (const std::pair<T1, T2>& pair)
    {
        (*this) << pair.first;
        (*this) << pair.second;

        return *this;
    }

    //std::vector
    template <typename T>
    Serialize& operator << (const std::vector<T>& vec)
    {
        serializeContainer(vec);
        return *this;
    }

    //std::basic_string
    template <typename T>
    Serialize& operator << (const std::basic_string<T>& vec)
    {
        serializeContainer(vec);
        return *this;
    }

    //std::set
    template <typename T>
    Serialize& operator << (const std::set<T>& s)
    {
        serializeContainer(s);
        return *this;
    }

    //std::multiset
    template <typename T>
    Serialize& operator << (const std::multiset<T>& s)
    {
        serializeContainer(s);
        return *this;
    }

    //std::unordered_set
    template <typename T>
    Serialize& operator << (const std::unordered_set<T>& s)
    {
        serializeContainer(s);
        return *this;
    }

    //std::unordered_multiset
    template <typename T>
    Serialize& operator << (const std::unordered_multiset<T>& s)
    {
        serializeContainer(s);
        return *this;
    }

    //std::map
    template <typename KeyT, typename ValueT>
    Serialize& operator << (const std::map<KeyT, ValueT>& m)
    {
        serializeContainer(m);
        return *this;
    }

    //std::multimap
    template <typename KeyT, typename ValueT>
    Serialize& operator << (const std::multimap<KeyT, ValueT>& m)
    {
        serializeContainer(m);
        return *this;
    }

    //std::unordered_map
    template <typename KeyT, typename ValueT>
    Serialize& operator << (const std::unordered_map<KeyT, ValueT>& m)
    {
        serializeContainer(m);
        return *this;
    }

    //std::unordered_multimap
    template <typename KeyT, typename ValueT>
    Serialize& operator << (const std::unordered_multimap<KeyT, ValueT>& m)
    {
        serializeContainer(m);
        return *this;
    }

    //std::list
    template <typename T>
    Serialize& operator << (const std::list<T>& l)
    {
        serializeContainer(l);
        return *this;
    }

    //std::deque
    template <typename T>
    Serialize& operator << (const std::deque<T>& l)
    {
        serializeContainer(l);
        return *this;
    }

    //std::forward_list
    template <typename T>
    Serialize& operator << (const std::forward_list<T>& fl)
    {
        typedef typename std::forward_list<T> FL;
        typename FL::size_type size = std::distance(fl.begin(), fl.end());
        (*this) << size;

        for(const typename FL::value_type& item : fl)
            (*this) << item;

        return *this;
    }

    //type without corresponding operator << ; non trivial type whill occur a compile error
    template <typename T>
    Serialize& operator << (const T& t)
    {
#if GCC_VERSION <= 40902
        static_assert(std::is_trivial<T>::value, "trival，  须定义operator<<");
#else 
        static_assert(std::is_trivially_copyable<T>::value, "非trivally copyable，须定义operator<<");
#endif
        m_buffer->append((const typename buffer_type::value_type*)&t, sizeof(t));
        m_opos += sizeof(t);
        return *this;
    }

private:
    //serialize trivial type
    template <typename TrivialType>
    void serialize(const TrivialType& t, std::true_type)
    {
        m_buffer->append((const typename buffer_type::value_type*)&t, sizeof(t));
        m_opos += sizeof(t);
    }

    //serialize non-trivial type
    template <typename TrivialType>
    void serialize(const TrivialType&, std::false_type) = delete;

    //serialize container
    template <typename ContainerT>
    void serializeContainer(const ContainerT& container)
    {
        typename ContainerT::size_type size = container.size();
        (*this) << size;

        for(const typename ContainerT::value_type& item : container)
            (*this) << item;
    }

private:
    buffer_type* m_buffer;
    bool m_holdBuffer;
    size_type m_opos = 0;
};

template<typename Buffer>
class Deserialize
{
public:
    typedef Buffer buffer_type;
    typedef typename Buffer::size_type size_type;

    explicit Deserialize(const buffer_type* buffer)
    : m_buffer(buffer)
    {
    }

    ~Deserialize() = default;

    Deserialize(const Deserialize& other) = delete;
    Deserialize& operator = (const Deserialize& other) = delete;

    const buffer_type* buffer() const
    {
        return buffer;
    }

    size_type tellg() const
    {
        return m_ipos;
    }

    void reset(const buffer_type* buffer)
    {
        m_buffer = buffer;
        m_ipos = 0;
    }

    //std::string
    Deserialize& operator >> (std::string& str)
    {
        std::string::size_type size = 0;
        (*this) >> size;

        str.assign((const char*)(m_buffer->data() + m_ipos), size);
        m_ipos += size;

        return *this;
    }

    //std::pair
    template <typename T1, typename T2>
    Deserialize& operator >> (std::pair<T1, T2>& pair)
    {
        (*this) >> pair.first;
        (*this) >> pair.second;

        return *this;
    }

    //std::vector
    template <typename T>
    Deserialize& operator >> (std::vector<T>& vec)
    {
        deserializeRandomAccessContainer(vec);
        return *this;
    }

    //std::basic_string
    template <typename T>
    Deserialize& operator >> (std::basic_string<T>& vec)
    {
        deserializeRandomAccessContainer(vec);
        return *this;
    }

    //std::set
    template <typename T>
    Deserialize& operator >> (std::set<T>& s)
    {
        deserializeNonRandomAccessContainer(s);
        return *this;
    }

    //std::multiset
    template <typename T>
    Deserialize& operator >> (std::multiset<T>& s)
    {
        deserializeNonRandomAccessContainer(s);
        return *this;
    }

    //std::unordered_set
    template <typename T>
    Deserialize& operator >> (std::unordered_set<T>& s)
    {
        deserializeNonRandomAccessContainer(s);
        return *this;
    }

    //std::unordered_multiset
    template <typename T>
    Deserialize& operator >> (std::unordered_multiset<T>& s)
    {
        deserializeNonRandomAccessContainer(s);
        return *this;
    }

    //std::map
    template <typename KeyT, typename ValueT>
    Deserialize& operator >> (std::map<KeyT, ValueT>& m)
    {
        deserializeNonRandomAccessContainer(m);
        return *this;
    }

    //std::multimap
    template <typename KeyT, typename ValueT>
    Deserialize& operator >> (std::multimap<KeyT, ValueT>& m)
    {
        deserializeNonRandomAccessContainer(m);
        return *this;
    }

    //std::unordered_map
    template <typename KeyT, typename ValueT>
    Deserialize& operator >> (std::unordered_map<KeyT, ValueT>& m)
    {
        deserializeNonRandomAccessContainer(m);
        return *this;
    }

    //std::unordered_multimap
    template <typename KeyT, typename ValueT>
    Deserialize& operator >> (std::unordered_multimap<KeyT, ValueT>& m)
    {
        deserializeNonRandomAccessContainer(m);
        return *this;
    }

    //std::list
    template <typename T>
    Deserialize& operator >> (std::list<T>& l)
    {
        deserializeNonRandomAccessContainer(l);
        return *this;
    }

    //std::deque
    template <typename T>
    Deserialize& operator >> (std::deque<T>& l)
    {
        deserializeNonRandomAccessContainer(l);
        return *this;
    }

    //std::forward_list
    template <typename T>
    Deserialize& operator >> (std::forward_list<T>& fl)
    {
        typedef typename std::forward_list<T> FL;

        typename FL::size_type size = 0;
        (*this) >> size;

        typename FL::value_type tmp;
        fl.push_front(std::move(tmp));
        typename FL::iterator iter = fl.begin();
        for(typename FL::size_type i = 0; i < size; ++i)
        {
            typename FL::value_type t;
            (*this) >> t;
            iter = fl.insert_after(iter, std::move(t));
        }
        fl.pop_front();

        return *this;
    }

    //type without corresponding operator << ; non trivial type whill occur a compile error
    template <typename T>
    Deserialize& operator >> (T& t)
    {

#if GCC_VERSION > 40901
        static_assert(std::is_trivially_copyable<T>::value, "非trivally copyable，须定义operator>>");
#else 
        static_assert(std::is_trivial<T>::value, "非trival，须定义operator>>");
#endif

        typename buffer_type::value_type* p = (typename buffer_type::value_type*)&t;
        for(size_type i = 0; i < sizeof(t) && m_ipos < m_buffer->size(); ++i)
        {
            *(p + i) = m_buffer->at(m_ipos);
            ++m_ipos;
        }
        return *this;
    }

private:
    //deserialize trivial type
    template <typename TrivialType>
    void deserialize(const TrivialType& t, std::true_type)
    {
        typename buffer_type::value_type* p = (typename buffer_type::value_type*)&t;
        for(size_type i = 0; i < sizeof(t) && m_ipos < m_buffer->size(); ++i)
        {
            *(p + i) = m_buffer->at(m_ipos);
            ++m_ipos;
        }
    }

    //deserialize non-trivial type
    template <typename TrivialType>
    void deserialize(const TrivialType&, std::false_type) = delete;

    //deserialize non-random access container
    template <typename ContainerT>
    void deserializeNonRandomAccessContainer(ContainerT& container)
    {
        container.clear();

        typename ContainerT::size_type size = 0;
        (*this) >> size;

        for(typename ContainerT::size_type i = 0; i < size; ++i)
        {
            typename ContainerT::value_type t;
            (*this) >> t;
            container.insert(container.end(), std::move(t));
        }
    }

    //deserialize random access container
    template <typename RandomAccessContainerT>
    void deserializeRandomAccessContainer(RandomAccessContainerT& container)
    {
        typename RandomAccessContainerT::size_type size = 0;
        (*this) >> size;

        container.resize(size);
        for(typename RandomAccessContainerT::size_type i = 0; i < size; ++i)
            (*this) >> container[i];
    }

private:
    const buffer_type* m_buffer;
    size_type m_ipos = 0;
};

}}
#endif


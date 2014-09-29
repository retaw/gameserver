#include "../lock_free_circular_queue_mm.h"
#include "../lock_free_circular_queue_ss.h"
#include "../circular_queue.h"

#include "test.h"

#include <thread>
#include <mutex>


using namespace water;
using namespace componet;
using namespace test;

const int64_t N = 1000000;
const int64_t queueSizePow = 3;

void singleThread()
{
    LockFreeCircularQueueSPSC<int64_t> queue(1 << queueSizePow);

    int64_t sum = 0;
    int64_t i = 0;
    while(true)
    {
        while(queue.push(i++))
        {
            if(i >= N)
                break;
        }

        int64_t item = 0;
        while(queue.pop(&item))
        {
            sum += item;
        }

        if(i >= N)
            break;
    }

    cout << sum << endl;
}

void singleThreadUnsafe()
{
    CircularQueue<int64_t> queue(1 << queueSizePow);

    int64_t sum = 0;
    int64_t i = 0;
    while(true)
    {
        while(!queue.full())
        {
            queue.push(i++);
            if(i >= N)
                break;
        }


        while(!queue.empty())
        {
            sum += queue.get();
            queue.pop();
        }

        if(i >= N)
            break;
    }

    cout << sum << endl;
}

void singleProducerAndSingleConsumerMutexVersion()
{
    CircularQueue<int64_t> queue(1 << queueSizePow);

    std::mutex mutex;

    auto producerDo = [&]()
    {
        int64_t i = 0; 
        while(i < N)
        {
            {
                std::unique_lock<std::mutex> lock(mutex);
                if(queue.full())
                    continue;

                queue.push(i);
            }
            ++i;
        }
    };

    auto consumerDo = [&]()
    {
        int64_t sum = 0;
        int64_t count = 0;
        while (count < N)
        {
            int64_t item = 0;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if(queue.empty())
                    continue;

                item = queue.get();
                queue.pop();
            }
            ++count;
            sum += item;
        }

        cout << sum << endl;
    };

    std::thread consumer(consumerDo);
    std::thread procuder(producerDo);

    procuder.join();
    consumer.join();
}

void singleProducerAndSingleConsumer()
{
    water::LockFreeCircularQueueSPSC<int64_t> queue(queueSizePow); //容量远小于数据量的queue
    auto producerDo = [&]()
    {
        int64_t i = 0;
        while(i < N)
        {
            while(!queue.push(i))
            {
            }
            ++i;
        }
    };

    auto consumerDo = [&]()
    {
        int64_t sum = 0;
        int64_t count = 0;
        while (count < N)
        {
            int64_t item = 0;
            if(!queue.pop(&item))
            {
                // std::this_thread::yield();
                continue;
            }
            ++count;
            sum += item;
        }

        cout << sum << endl;
    };

    std::thread consumer(consumerDo);
    std::thread procuder(producerDo);

    procuder.join();
    consumer.join();
}

void multiProducerAndMultiConsumer()
{
    water::LockFreeCircularQueueMPMC<int64_t> queue(queueSizePow); //容量远小于数据量的queue
    if(!queue.isLockFree())
        cout << "Not lock free!" << endl;

    const int64_t consumerNum = 2;
    const int64_t producerNum = 5;

    std::vector<int64_t> threadSum(consumerNum, 0);
    auto producerDo = [&](int threadCode)
    {
        const int64_t size = N / producerNum;
        int64_t i = size * threadCode;
        int64_t end = i + size;
        while(i < end)
        {
            while(!queue.push(i))
            {
            }
            ++i;
        }
    };

    auto consumerDo = [&](int threadCode)
    {
        const int64_t size = N / consumerNum;
        int64_t& sum = threadSum[threadCode];
        int64_t count = 0;
        while (count < size)
        {
            int64_t item = 0;
            if(!queue.pop(&item))
            {
                // std::this_thread::yield();
                continue;
            }
            ++count;
            sum += item;
        }
    };

    std::vector<std::thread> consumers(consumerNum);
    std::vector<std::thread> producers(producerNum);
    for(int i = 0; i < std::max(consumerNum, producerNum); ++i)
    {
        if(i < consumerNum)
            consumers[i] = std::thread(consumerDo, i);

        if(i < producerNum)
            producers[i] = std::thread(producerDo, i);
    }

    for(int i = 0; i < consumerNum; ++i)
        consumers[i].join();

    for(int i = 0; i < producerNum; ++i)
        producers[i].join();

#ifdef DEBUG
    cout << std::accumulate(threadSum.begin(), threadSum.end(), int64_t(0)) << " : " << threadSum << endl;
#else
    cout << std::accumulate(threadSum.begin(), threadSum.end(), int64_t(0)) << endl;
#endif
}


void multiProducerAndMultiConsumerMutexVersion()
{
    CircularQueue<int64_t> queue(1 << queueSizePow); //容量远小于数据量的queue

    const int64_t consumerNum = 2;
    const int64_t producerNum = 5;

    std::vector<int64_t> threadSum(consumerNum, 0);

    std::mutex mutex;

    auto producerDo = [&](int threadCode)
    {
        const int64_t size = N / producerNum;
        int64_t i = size * threadCode;
        int64_t end = i + size;
        while(i < end)
        {
            {
                std::unique_lock<std::mutex> lock(mutex);
                if(queue.full())
                    continue;

                queue.push(i);
            }
            ++i;
        }
    };

    auto consumerDo = [&](int threadCode)
    {
        const int64_t size = N / consumerNum;
        int64_t& sum = threadSum[threadCode];
        int64_t count = 0;
        while (count < size)
        {
            int64_t item = 0;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if(queue.empty())
                    continue;
                item = queue.get();
                queue.pop();
            }
            ++count;
            sum += item;
        }
    };

    std::vector<std::thread> consumers(consumerNum);
    std::vector<std::thread> producers(producerNum);
    for(int i = 0; i < std::max(consumerNum, producerNum); ++i)
    {
        if(i < consumerNum)
            consumers[i] = std::thread(consumerDo, i);

        if(i < producerNum)
            producers[i] = std::thread(producerDo, i);
    }

    for(int i = 0; i < consumerNum; ++i)
        consumers[i].join();

    for(int i = 0; i < producerNum; ++i)
        producers[i].join();

#ifdef DEBUG
    cout << std::accumulate(threadSum.begin(), threadSum.end(), int64_t(0)) << " : " << threadSum << endl;
#else
    cout << std::accumulate(threadSum.begin(), threadSum.end(), int64_t(0)) << endl;
#endif

}

int main()
{
    int64_t sum = 0;
    for(int64_t i = 0; i < N; ++i)
        sum += i;
    cout << sum << endl;

//    multiProducerAndMultiConsumer();
    const int testTimes = 10;
/*
    PERFORMANCE(singleThread, testTimes);
    PERFORMANCE(singleThreadUnsafe, testTimes);
    PERFORMANCE(singleProducerAndSingleConsumer, testTimes);
    PERFORMANCE(singleProducerAndSingleConsumerMutexVersion, testTimes);
    */
    PERFORMANCE(multiProducerAndMultiConsumer, testTimes);
    PERFORMANCE(multiProducerAndMultiConsumerMutexVersion, testTimes);
    return 0;
}



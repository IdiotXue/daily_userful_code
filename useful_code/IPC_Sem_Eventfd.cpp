#include <iostream>
#include <thread>
#include <sys/eventfd.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include <assert.h>
using std::thread;
using std::vector;

/**
 * 对比eventfd和POSIX信号量semaphore做线程同步的速率，结论是差不多
 * 场景：6个线程的ID分别为1到6,每个线程将自己的ID在屏幕打印n遍，要求顺序为123456123456...
 * time -p ./bin/work > /dev/null 测量运行时间
 * 
 * 当然，这个场景其实也可以用mutex+condition完成，依然是每个线程一个condition
 */

/**
 * eventfd
 */
vector<int> vnEventFd(6);

void PrintID_EventFd(int id)
{
    uint64_t count; //至少要8字节的变量
    // int ret;
    for (int i = 0; i < 400000; ++i)
    {
        read(vnEventFd[id], &count, sizeof(count)); //阻塞读出所有write的和，eventfd清零
        // ret = read(vnEventFd[id], &count, sizeof(count)); //阻塞读出所有write的和，eventfd清零
        // assert(ret == sizeof(count));
        // assert(count == 1);
        printf("%d", id + 1);
        write(vnEventFd[(id + 1) % vnEventFd.size()], &count, sizeof(count));
    }
}
void Test_EventFd()
{
    vector<thread> vtPool; //线程池
    for (size_t i = 0; i < vnEventFd.size(); ++i)
    {
        vnEventFd[i] = eventfd(0, 0); //第一个0是计数器初值，第二个0是默认用阻塞方式
        vtPool.push_back(thread(PrintID_EventFd, i));
    }
    uint64_t count = 1;
    write(vnEventFd[0], &count, sizeof(count));

    for (auto &t : vtPool)
        if (t.joinable())
            t.join();
    std::cout << std::endl
              << "thread pool stoping " << std::endl;
}

/**
 * POSIX 信号量
 */
vector<sem_t> vSem(6);
void PrintID_Sem(int id)
{
    for (int i = 0; i < 400000; ++i)
    {
        sem_wait(&vSem[id]);
        printf("%d", id + 1);
        sem_post(&vSem[(id + 1) % vSem.size()]);
    }
}
void Test_Sem()
{
    vector<thread> vtPool; //线程池
    for (size_t i = 0; i < vSem.size(); ++i)
    {
        sem_init(&vSem[i], 0, 0); //第一个0表示不在多个进程共享，第二个0为初始值
        vtPool.push_back(thread(PrintID_Sem, i));
    }
    sem_post(&vSem[0]);

    for (auto &t : vtPool)
        if (t.joinable())
            t.join();
    for (size_t i = 0; i < vSem.size(); ++i)
        sem_destroy(&vSem[i]);
    std::cout << std::endl
              << "thread pool stoping " << std::endl;
}

int main()
{
    //100000 real:2.54, user:0.32, sys 2.56
    //400000 real:10.4, user:1.35, sys 10.16
     Test_EventFd(); 

    //100000 real:2.59, user:0.39, sys 2.50
    //400000 real:10.49, user:1.75, sys 10.0375
    // Test_Sem();
}

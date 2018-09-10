/**
 * Posix共享内存
 * #include <sys/mman.h>
 * int shm_open(const char *name, int oflag, mode_t mode); //成功返回非负描述符，失败返回-1
 * int shm_unlink(const char *name); //成功返回0，失败返回-1
 * 显然，Posix 共享内存的接口比 system V的容易使用多了，而且有两个特点：
 * 1. 无需磁盘IO：与mmap相比，Posix共享内存没有映射到磁盘文件，只是将内核缓冲区映射到多个进程地址空间
 * 2. 随内核持续性：除非 手动shm_unlink至引用计数为0 或 系统关闭，否则映射的空间将一直存在于虚拟内存，因此支持【热更新】的应用场景
 */

/**
 * 程序1：创建共享内存，写数据，信号量+1
 */
#include <iostream>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>  // O_RDWR
#include <stdlib.h> // exit
#include <cerrno>   //errno
#include <cstring>  //strerror
#include <unistd.h> //ftruncate, access
#include <string>

// POSIX共享内存和信号量生成的文件都存放在/dev/shm目录下
#define POSIX_SHM_NAME "TEST_POSIX_MMAP.shm"
#define POSIX_SEM_NAME "TEST_POSIX_SEM.sem"
const size_t SHM_FILE_SIZE = 100; //byte

int main1(int argc, char const *argv[])
{
    int fd_shm;
    std::string shm_path = "/dev/shm" + std::string(POSIX_SHM_NAME);
    if (access(shm_path.c_str(), F_OK) == 0) //测试文件是否存在, 0为存在
        fd_shm = shm_open(POSIX_SHM_NAME, O_RDWR, 0);
    else
        fd_shm = shm_open(POSIX_SHM_NAME, O_RDWR | O_CREAT, 0666);
    if (fd_shm == -1)
    {
        std::cout << "shm_open fail:" << strerror(errno) << std::endl;
        exit(-1);
    }

    sem_t *sem;
    shm_path = "/dev/shm/sem." + std::string(POSIX_SEM_NAME);
    if (access(shm_path.c_str(), F_OK) == 0) //测试文件是否存在, 0为存在
        sem = sem_open(POSIX_SEM_NAME, 0);
    else
        sem = sem_open(POSIX_SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
    if (sem == SEM_FAILED)
    {
        std::cout << "sem_open fail:" << strerror(errno) << std::endl;
        exit(-1);
    }

    ftruncate(fd_shm, SHM_FILE_SIZE); //设置文件长度

    char *ptr_shm = (char *)mmap(NULL, SHM_FILE_SIZE, PROT_READ | PROT_WRITE,
                                 MAP_SHARED, fd_shm, 0);
    close(fd_shm);

    const char msg[] = "gaoxiao";
    memcpy(ptr_shm, msg, sizeof(msg));
    std::cout << getpid() << " send: " << ptr_shm << std::endl;

    sem_post(sem);
    sem_close(sem);

    return 0;
}

/**************************************************************** */

/**
 * 程序2：读共享内存中的数据
 */
#include <iostream>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>  // O_RDWR
#include <stdlib.h> // exit
#include <cerrno>   //errno
#include <cstring>  //strerror
#include <sys/stat.h>
#include <unistd.h>

#define POSIX_SHM_NAME "TEST_POSIX_MMAP.shm"
#define POSIX_SEM_NAME "TEST_POSIX_SEM.sem"
// const size_t SHM_FILE_SIZE = 100; //byte

int main2(int argc, char const *argv[])
{
    int fd_shm = shm_open(POSIX_SHM_NAME, O_RDWR, 0);
    if (fd_shm == -1)
    {
        std::cout << "shm_open fail:" << strerror(errno) << std::endl;
        exit(-1);
    }

    sem_t *sem = sem_open(POSIX_SEM_NAME, 0);
    if (sem == SEM_FAILED)
    {
        std::cout << "sem_open fail:" << strerror(errno) << std::endl;
        exit(-1);
    }

    struct stat fd_shm_stat;
    fstat(fd_shm, &fd_shm_stat);
    std::cout << "shm file size: " << fd_shm_stat.st_size << std::endl;

    char *ptr_shm = (char *)mmap(NULL, fd_shm_stat.st_size, PROT_READ | PROT_WRITE,
                                 MAP_SHARED, fd_shm, 0);
    close(fd_shm);

    sem_wait(sem);
    std::cout << getpid() << " recv: " << ptr_shm << std::endl;
    sem_close(sem);
    return 0;
}
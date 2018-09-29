/**
 * 获取虚拟内存页大小
 */
#include <iostream>
#include <unistd.h>
using namespace std;

int main(int argc, char const *argv[])
{
    size_t page_size = sysconf(_SC_PAGESIZE);
    cout << page_size << endl;
}
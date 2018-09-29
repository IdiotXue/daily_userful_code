# 日常codeing的存档，记录一些有用的小实验
1. 在useful_code目录中复制一个cpp文件到src目录：cp useful_code/xx.cpp src/main.cpp
2. 执行 ./build即可运行，缺bin和obj目录就创建一下

# 摘要
- get_page_size: 获取虚拟内存分页大小sysconf
- IPC_Dinning_Philosophers: 哲学家进餐问题，多线程下的解法，mutex+condition_variable
- IPC_Sem_Eventfd: 线程同步的两种方法：POSIX信号量semaphore 和 eventfd速率的对比
- Posix_shm_sem: Posix共享内存与信号量做进程间同步和通信的例子
- istringstream_usage: istringstream和getline简化文件读写
- judge_byte_order: 判断系统字节序
- lseek_write: lseek产生文件空洞的实验
- postfixEpr_to_EprTree: 后缀表达式 转 表达式树
- replace_char: 排列+大数问题的字符串替换
- several_sort: 实现多种排序，包括插入、谢尔、自己实现堆、STL堆、归并、三数中值的快排、间接排序、拓扑排序
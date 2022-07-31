#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main()
{

    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skiplist.load_file函数
    Skiplist<int, std::string> skiplist(6);

    skiplist.insert_element(1, "我想和你一起生活");
    skiplist.insert_element(3, "在某个小镇");
    skiplist.insert_element(7, "共享");
    skiplist.insert_element(8, "无尽的黄昏");
    skiplist.insert_element(9, "和");
    skiplist.insert_element(19, "绵绵不绝的钟声");
    skiplist.insert_element(19, "我想和你一起生活");

    std::cout << "skiplist size:" << skiplist.size() << std::endl;

    skiplist.dump_file();

    // skiplist.load_file();

    skiplist.search_element(9);
    skiplist.search_element(18);

    skiplist.display_list();

    skiplist.delete_element(3);
    skiplist.delete_element(7);

    std::cout << "skiplist size:" << skiplist.size() << std::endl;

    skiplist.display_list();
}
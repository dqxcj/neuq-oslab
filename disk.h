#ifndef DISK_H
#define DISK_H
#include <fstream>
#include <string>
#include <unordered_map>

const int NORMAL = 3;
const int SWAP = 4;

struct Inode {
    int index;  //一级索引所在区块
    int size;   //文件大小，单位为B
};

class Disk {
public:
    Disk(const std::string &filename);

    void createFile(const std::string &path, int size, const std::string &content, int type);   //创建新文件并为其分配区块,size单位是B
    void outPut();  //将磁盘内容格式化输出到outfile文件中查看
    std::string readFile(const std::string &path, int type);    //读取文件
    void deleteFile(const std::string &path, int type);         //删除文件

    void outPutUnused();
    // 使用磁盘的交换区
    void swapToD(const std::string &pathS, const std::string &content); // pathS指文件路径和该内存块的原本在内存的位置组合的字符串
    std::string swapFromD(const std::string &pathS);

    int size(const std::string &path);

private:
    std::fstream disk_;  //模拟磁盘
    int block_ = 40;     //模拟磁盘区块大小为40个字节
    int file_ = 900;     //模拟文件区大小为900个区块
    int swap_ = 124;     //模拟交换区大小为124个区块
    bool unused_[1024];  //空闲块位示图
    std::unordered_map<std::string, Inode> inodes_;  //用于指示某个文件的大小和在磁盘中的存储位置
    std::unordered_map<std::string, Inode> inodes_swap_;
};

#endif
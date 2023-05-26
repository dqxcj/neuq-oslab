#include "disk.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

Disk::Disk(const string &filename) {
    disk_.open(filename, fstream::trunc | fstream::in | fstream::out);
    if(!disk_) {
        cerr << "文件" << filename <<"打开失败" << endl;
    } else {
        string s(40960, '-');
        disk_ << s;
    }
    for(int i = 0; i < 1024; i++) {
        unused_[i] = true;
    }
}

void Disk::createFile(const string &path, int size, const string &content, int type) {
    int begin, end;
    if(type == NORMAL) {
        begin = 0;
        end = file_;
    } else if(type == SWAP) {
        begin = file_;
        end = 1024;
    }
    if(size > 400) {
        cerr << "文件太大, 无法存储" << endl;
        return ;
    }

    // 计算存储文件需要的区块数量
    int blockAmount = size / block_;    
    if(blockAmount * block_ != size) {
        blockAmount += 1;
    }

    // 寻找足够的空闲快
    vector<int> toUse;
    for (int i = begin; i < end; i++) {
        if(unused_[i]) {
            toUse.push_back(i);
            if(toUse.size() == blockAmount + 1) {
                break;
            }
        }
    }
    if(toUse.size() < blockAmount + 1) {
        cerr << "磁盘存储空间不足, 无法存储该文件" << endl;
        return ;
    }

    // 写入数据
    string index1;
    string blockContent;
    for(int i = 0; i < blockAmount; i++) {
        // 构造写入的文件的内容
        if(i != blockAmount - 1) {
            blockContent = string(content.begin() + i * block_, content.begin() + (i + 1) * block_);
        } else if(i == blockAmount - 1) {
            blockContent = string(content.begin() + i * block_, content.end());
        }

        // 将构造的文件内容写入
        disk_.seekp(toUse.back() * block_, fstream::beg);
        disk_ << blockContent;

        // 将该区块位置写入索引中
        string temp1 = to_string(toUse.back());
        if(temp1.size() < 4) {
            string temp2(4 - temp1.size(), '0');
            temp1 = temp2 + temp1;
        }
        index1 += temp1;

        // 将该区块改为已用
        unused_[toUse.back()] = false;
        toUse.pop_back();
    }

    // 将一级索引写入
    disk_.seekp(toUse.back() * block_, fstream::beg);
    disk_ << index1;
    // 制作该文件对应的inode
    Inode inode;
    inode.index = toUse.back();
    inode.size = size;
    // cout << "line1121" << inode.size << endl;
    if(type == NORMAL) {
        inodes_[path] = inode;
    } else if(type == SWAP) {
        inodes_swap_[path] = inode;
    }

    unused_[toUse.back()] = false;
    toUse.pop_back();
}

void Disk::outPut() {
    fstream out("outfile", fstream::trunc | fstream::in | fstream::out);
    if(!out) {
        cerr << "outPut中文件打开失败" << endl;
    }
    char c;
    int i = 0;
    disk_.seekp(0, fstream::beg);
    disk_ >> noskipws;
    while(disk_ >> c) {
        out << c;
        i++;
        if(i % 40 == 0) {
            out << endl;
        }
        if(i / 40 == 1024) {
            break;
        }
    }
    disk_ >> skipws;
    disk_.seekp(0, fstream::end);
}

string Disk::readFile(const string &path, int type) {
    string index1, content;
    // 获取一级索引的内容
    Inode inode;
    if(type == NORMAL) {
        inode = inodes_[path];
    } else if(type == SWAP) {
        inode = inodes_swap_[path];
    }

    disk_.seekp(inode.index * block_, fstream::beg);
    char c;
    while(disk_ >> c) {
        if(c == '-' || index1.size() == 40) {
            break;
        }
        index1 += c;
    }
    // cout << "index1: " <<  index1 << endl;

    string tempStr;
    int temp1 = index1.size() / 4;
    for(int i = 0; i < temp1; i++) {
        //获取文件内容
        int content_index = stoi(string(index1.end() - 4, index1.end()));
        for(int m = 0; m < 4; m++) {
            index1.pop_back();
        }
        disk_.seekp(content_index * block_, fstream::beg);
        disk_ >> noskipws;
        while(disk_ >> c) {
            if( c == '-' || content.size() + tempStr.size() == inode.size || tempStr.size() == 40) {
                break;
            }
            tempStr += c;
        }
        disk_ >> skipws;
        //cout << tempStr << endl;
        content = tempStr + content;
        tempStr = "";

    }
    return content;
}

void Disk::deleteFile(const std::string &path, int type) {
    string index1;
    Inode inode;
    if(type == NORMAL) {
        inode = inodes_[path];
        inodes_.erase(path);
    } else if(type == SWAP) {
        inode = inodes_swap_[path];
        inodes_swap_.erase(path);
    }

    disk_.seekp(inode.index * block_, fstream::beg);
    char c;
    while (disk_ >> c) {
        if (c == '-' || index1.size() == 40) {
            break;
        }
        index1 += c;
    }
    // cout << "index1: " << index1 << endl;

    int temp1 = index1.size() / 4;
    for(int i = 0; i < temp1; i++) {
        // 获取存储所在区块
        int index = stoi(string(index1.end() - 4, index1.end()));
        for (int j = 0; j < 4; j++) {
            index1.pop_back();
        }
        // 删除对应区块
        disk_.seekp(index * block_, fstream::beg);
        for(int j = 0; j < block_; j++) {
            disk_ << '-';
        }

        unused_[index] = true;
    }

    // 删除索引所在区块
    disk_.seekp(inode.index * block_, fstream::beg);
    for (int j = 0; j < block_; j++) {
        disk_ << '-';
    }
    unused_[inode.index] = true;
}

void Disk::swapToD(const std::string &pathS, const std::string &content) {
    createFile(pathS, content.size(), content, SWAP);
}
string Disk::swapFromD(const std::string &pathS) {
    string content = readFile(pathS, SWAP);
    deleteFile(pathS, SWAP);
    return content;
}

int Disk::size(const string &path) {
    return inodes_[path].size;
}

void Disk::outPutUnused()
{
    for (int i = 0; i < 1024; i++)
    {
        if (i != 0 && i % 64 == 0)
        {
            cout << endl;
        }
        cout << unused_[i] << " ";
    }
    cout << endl;
}
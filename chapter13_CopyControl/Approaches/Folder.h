#ifndef CODESPACE_FOLDER_H
#define CODESPACE_FOLDER_H

#include <iostream>
#include <set>

class Message;
class Folder {
    friend class Message;
public:
    Folder(const Folder&);
    Folder& operator=(const Folder&);
    Folder() = default;
    ~Folder();

    void save(Message&);
    void remove(Message&);

    // 最基本单元:加inline封装
    inline void addMsg(Message* ms) { data.insert(ms); }
    inline void rmvMsg(Message* ms) { data.erase(ms); }
private:
    std::set<Message*> data;
    void add_to_Messages(const Folder&);
    void remove_from_Messages();
};

class Folder;
class Message {
    friend class Folder;
    friend inline void swap(Message&, Message&);
public:
    explicit Message(const std::string& str = "") : content(str) {}
    Message(const Message&);
    Message(Message &&m) : content(std::move(m.content)) {
        move_Folders(&m);
    }
    Message& operator=(const Message&);
    Message& operator=(Message&&);
    ~Message();

    void save(Folder&);
    void remove(Folder&);

    // 最基本单元:加inline封装
    inline void addFolder(Folder* folder) { folders.insert(folder); }
    inline void rmvFolder(Folder* folder) { folders.erase(folder); }

    void move_Folders(Message *msg);
private:
    std::string content;
    std::set<Folder*> folders;
    // 拷贝构造函数,拷贝赋值运算符,析构函数使用的工具函数
    void add_to_Folders(const Message&);
    void remove_from_Folders();
};

#endif //CODESPACE_FOLDER_H

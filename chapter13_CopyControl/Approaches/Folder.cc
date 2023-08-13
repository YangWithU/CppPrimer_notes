#include <iostream>
#include "Folder.h"

void Message::save(Folder &fd) {
    folders.insert(&fd);
    fd.addMsg(this);
}

void Message::remove(Folder &fd) {
    folders.erase(&fd);
    fd.rmvMsg(this);
}

// 底层添加元素函数
void Message::add_to_Folders(const Message &ms) {
    for(auto x : ms.folders)
        x->addMsg(this);
}

// 底层清空函数
void Message::remove_from_Folders() {
    // set可能抛出异常
    for(auto x : folders)
        x->rmvMsg(this);
    folders.clear();
}

// 不光要给自己成员赋值，同时也需要向Folder中添加成员
Message::Message(const Message &rhs) :
        content(rhs.content), folders(rhs.folders) {
    add_to_Folders(rhs);
}

Message::~Message() {
    remove_from_Folders();
}

// 移动操作的核心
// Message成员函数:移动自身folders
void Message::move_Folders(Message *msg) {
    folders = std::move(msg->folders);

    // 此操作可能抛出异常:bad_alloc
    for(auto x : folders) {
        x->rmvMsg(msg);
        x->addMsg(this);
    }
    msg->folders.clear();
}

Message& Message::operator=(const Message &rhs) {
    remove_from_Folders(); // clear this folder
    content = rhs.content; // copy members
    folders = rhs.folders;
    add_to_Folders(rhs); // add to Folder
    return *this;
}

// 移动赋值运算符
Message& Message::operator=(Message &&rhs) {
    if(this == &rhs) return *this;
    remove_from_Folders();
    content = std::move(rhs.content);
    move_Folders(&rhs);
    return *this;
}

// 两Message对象的拷贝交换
void swap(Message &lhs, Message &rhs) {
    using std::swap;
    // 移除对应Folders中的成员数据
    for(auto x : lhs.folders)
        x->rmvMsg(&lhs);
    for(auto x : rhs.folders)
        x->rmvMsg(&rhs);
    swap(lhs.folders, rhs.folders);
    swap(lhs.content, rhs.content);
    for(auto x : lhs.folders)
        x->addMsg(&lhs);
    for(auto x : rhs.folders)
        x->addMsg(&rhs);
}

// 同时向两类中的两容器data与folders添加对象
void Folder::save(Message &ms) {
    data.insert(&ms);   // data加Message
    ms.addFolder(this); // folders加Folder
}

// 同时向两类中的两容器data与folders移除对象
void Folder::remove(Message &ms) {
    data.erase(&ms); // data删Message
    ms.rmvFolder(this); // folders加Folder
}

void Folder::add_to_Messages(const Folder &fd) {
    for(auto x : fd.data)
        x->addFolder(this);
}

void Folder::remove_from_Messages() {
    while (!data.empty())
        (*data.begin())->remove(*this);
}

Folder::Folder(const Folder &fd) :
    data(fd.data) { add_to_Messages(fd); }

Folder& Folder::operator=(const Folder &fd) {
    remove_from_Messages();
    data = fd.data;
    add_to_Messages(fd);
    return *this;
}

Folder::~Folder() {
    remove_from_Messages();
}
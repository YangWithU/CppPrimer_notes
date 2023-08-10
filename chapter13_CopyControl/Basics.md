# 拷贝控制

## 拷贝构造函数

拷贝构造函数是指**第一个参数是自身类类型的引用，且额外参数都具有默认值**的函数

```c++
class Fo1 {
public:
    Fo1() = default;
    Fo1(const Fo1&) {} // copy ctor
};

class Fo2 {
public:
    Fo2() = default;
    Fo2(const Fo2&) = delete;
};

int main()
{
    Fo1 f1;
    Fo1 f2 = f1;
    Fo2 f3;
    Fo2 f4 = f3; // error: deleted copy ctor
}
```

上述 `Fo1(const Fo1&) {}` 即为拷贝构造函数。一般而言构造函数内部如果没有自定义操作，则编译器会为当前类自动生成一个 `trivial copy ctor`

拷贝构造函数将同类型对象拷贝，经过寄存器将数据从旧拷到新对象。

## 拷贝ctor重载赋值运算符

在对象内部重载运算符本质上是重载一个名为该运算符的函数，例如重载 `<` 即为重载 `operator<` 函数。对于对象赋值运算符，编译器也会同拷贝构造函数一样，为对象内部自动生成**合成拷贝赋值运算符**：

```c++
class Foo {
public:
    Foo(const std::string s) : str(s) {}

    void opt() {
        std::cout << str << '\n';
    }
private:
    int x;
    std::string str;
};
```

对于如上对象而言，编译器为其生成的 `trivial` 拷贝构造函数与赋值运算符重载分别如下：

```c++
Foo(const Foo &other) : x(other.x), str(other.str) {}

Foo& operator=(const Foo &other) {
    this->x = other.x;
    this->str = other.str;
    return *this;
}
```

综上，我们最终可以通过上述代码分析出不同的传递值的方式

## 自定义析构函数类与默认拷贝函数

当我们在自定义构造函数中采用非智能指针形式动态分配内存，就需要在析构函数中进行一个内存的delete,但是当我们未明显指定拷贝运算符的使用方式时

在旧对象调用析构函数delete之前动态内存上的指针后

编译器生成的 `operator=` 会导致新对象的指针指向旧对象已经析构过的指针

这会导致在新对象析构时又对同一块析构过的内存再次delete，属于ub行为

> 从拷贝的控制可以看出，智能指针在拷贝时递增引用计数，不去进行销毁
> 仅仅在赋新值或是销毁的时候才会递减引用计数，以上种种可以解决delete问题

综上，**自定义析构函数的同时必须定义 `拷贝ctor` 与 `operator=`，但反之不然**

```c++
class HasPtr {
public:
    int cnt;
    explicit HasPtr(const std::string &s = std::string()) :
        ps(new std::string(s)), i(0) { cnt = 1; }

    HasPtr(HasPtr& other) :
        ps(new std::string(*other.ps)), i(other.i) {
        other.cnt++;
        this->cnt = other.cnt;
    }


    HasPtr& operator=(const HasPtr& other) {
        // `this->` equivalent to `i`
        // using `this` may help avoid name conflict
        this->i = other.i;
        auto tmpPtr = new std::string(*other.ps);
        // delete old data first
        delete this->ps;
        // then create new
        this->ps = tmpPtr;
        return *this;
    }

    ~HasPtr() { delete ps; }
private:
    std::string *ps;
    int i;
};
```

上述手动定义析构函数的过程很容易忘记，同时在拷贝构造函数中很容易引用未释放的指针
所以更好的解决方案是采用引用计数

## 引用计数

首先分析如何具体实现引用计数：

> + 构造函数负责创建新的引用计数
> + 拷贝构造函数需要递增旧计数器，且要避免分配新的计数器
> + 析构函数递减计数器，计数器为0则释放对象
> + `operator=` 递增右侧对象计数器，递减左侧计数器

所以我们可以有第一种实现：

```c++
class Node {
public:
    int x, cnt;
    explicit Node(int _x) : x(_x) { cnt = 1; };
    Node(Node& other) : x(other.x) {
        other.cnt++;
        this->cnt = other.cnt;
    }
};

int main()
{
    Node p1(114);
    std::cout << p1.cnt << '\n';
    auto p2(p1);
    std::cout << p1.cnt << " " << p2.cnt << '\n';
    auto p3 = p1;
    std::cout << p1.cnt << " " << p3.cnt << '\n';
}
```

但显然当我们遇到const对象后则束手无策：

```c++
Node(const Node& other) // other无法修改
```

所以显然无法将引用计数作为对象的成员变量存储。
我们将存储引用计数的目光放在动态内存上

```c++
class HasPtr {
public:
    explicit HasPtr(const std::string &s = std::string()) :
        ps(new std::string(s)), i(0), use(new std::size_t(1)) {}

    HasPtr(const HasPtr& other) :
        ps(new std::string(*other.ps)), i(other.i), use(other.use)
        { ++*use; }


    HasPtr& operator=(const HasPtr& other) {
        if(*this == &other) return *this;
        ++*other.use;
        if(--*use == 0) {
            delete ps;
            delete use;
        }
        this->ps = other.ps;
        this->i = other.i;
        this->use = other.use;
        return *this;
    }

    ~HasPtr() {
        if(--*use == 0) {
            delete ps;
            delete use;
        }
    }

private:
    std::string *ps;
    int i;
    std::size_t *use;
};
```

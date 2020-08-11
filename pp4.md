

## 环境搭建：



安装spim：

```
sudo apt-get install spim

修改run中地spim路径
```

复制文件指令

```
configure_file(${CMAKE_CURRENT_BINARY_DIR}/y.tab.h ${PROJECT_SOURCE_DIR}/include/y.tab.h  COPYONLY)
```



遇到该错误的时候，如确定子类中已有或实现父类的纯虚函数，则可以清理下cmake缓存，尝试重新编译项目。

```
undefined reference to 'vtable...
```

## 

## 虚函数

子类可以不实现父类的虚函数，但必须实现纯虚函数。

```
#include <iostream>
class Father {
public:
    virtual void Print() {
        std::cout<<"this is father"<<std::endl;
    }
};

class Son: public Father{

};


int main() {
    Father f;
    Son s;
   s.Print();
}
```

多态返回类型也必须相同。

```
#include <iostream>
class Father {
public:
    virtual void Print()=0;
};

class Son: public Father{
public:
    void  Print() {
        std::cout<<"This is in Son"<<std::endl;
    }
    int b;
};


int main() {
   
    Son s;
    s.Print();
}
```



函数重载与重写

```
class A{    // 定义父类 A
public:
    void foo(){
        cout<<"A::foo()"<<endl;
    }
};
class B:public A{   // 子类B从父类A继承而来
public:
    void foo(){    // 函数重载，定义了一个和父类A中同名的函数，但是类型不同
        cout<<"B::foo()"<<endl;
    }
};
int main(){
    A* a = new B;
    a->foo();   // 编译错误，在B类中找不到函数foo(void)，因为编译器不会跨域重载，需要声明 using A::foo
    return 0;
}
A::foo()
```

```
#include <iostream>
using namespace  std;


class A{    // 定义父类 A
public:
    virtual void foo(){
        cout<<"A::foo()"<<endl;
    }
};
class B:public A{   // 子类B从父类A继承而来
public:
    void foo(){    // 函数重载，定义了一个和父类A中同名的函数，但是类型不同
        cout<<"B::foo()"<<endl;
    }
};
int main(){
    A* a = new B;
    a->foo();   // 编译错误，在B类中找不到函数foo(void)，因为编译器不会跨域重载，需要声明 using A::foo
    return 0;
}
B::foo()
```

http://yangyingming.com/article/452/





## 类

#### 声明

类VarDecl的大小，一个空类(无成员变量)的也应该有大小，在该语言中，我们设置一个类声明的最小大小为4B。

```
int ClassDecl::GetMemBytes() {
    int memBytes = 0;
    if(extends!=NULL) {
        Decl *d = Program::gScope->table->Lookup(extends->GetName());
        ClassDecl* classDecl = dynamic_cast<ClassDecl*>(d);
        Assert(classDecl!= nullptr);
        memBytes+=d->GetMemBytes();
    }
    for(int i=0,n=members->NumElements();i<n;++i)
        memBytes+=members->Nth(i)->GetMemBytes();
    if(memBytes==0) return CodeGenerator::VarSize;
    return memBytes;
}
```

样例：

```
class Binky {
   void Method() {}
}

void main() {
  Binky d;
   Binky a;
 
  d = new Binky;
  a = new Binky;
  a = d;
}


SPIM Version 7.4 of January 1, 2009
Copyright 1990-2004 by James R. Larus (larus@cs.wisc.edu).
All Rights Reserved.
See the file README for a full copyright notice.
Loaded: /usr/class/cs143/bin/exceptions.s
```



函数

函数的栈的大小的计算还有一些问题，开小了会导致程序错误，目前还没有找到原因。只能够用一个比较笨的方法来进行修改。

```
Location *FnDecl::Emit(CodeGenerator *cg) {
    int offset = CodeGenerator::OffsetToFirstParam;

    if(isMethod)
        offset +=CodeGenerator::VarSize;

    for (int i=0,n=formals->NumElements();i<n;++i){
        VarDecl *d = formals->Nth(i);
        Location *loc = new Location(fpRelative,offset,d->GetName());
        d->SetMemLoc(loc);
        offset +=d->GetMemBytes();
    }

    if(body!= nullptr) {
        cg->GenLabel(GetLabel());
        cg->GenBeginFunc()->SetFrameSize(body->GetMemBytes()*4);
        body->Emit(cg);
        cg->GenEndFunc();
    }
    return nullptr;
}
```
pp3



## CMAKE

遇到的Cmake的语法需要掌握的

设置编译器为g++以及编译的参数

```
SET(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "  -Wall -g  -Wno-unused -Wno-sign-compare ")
```

自定义的命令，其中的OUTPUT还是不太熟悉

```
ADD_CUSTOM_COMMAND(
        SOURCE ${PROJECT_SOURCE_DIR}/scanner.l
        COMMAND ${FLEX_EXECUTABLE}
        ARGS -d ${PROJECT_SOURCE_DIR}/scanner.l
        TARGET pl
        OUTPUTS lex.yy.cc
     )

ADD_CUSTOM_COMMAND(
        SOURCE ${PROJECT_SOURCE_DIR}/parser.y
        COMMAND ${BISON_EXECUTABLE}
        ARGS -dvty ${PROJECT_SOURCE_DIR}/parser.y
        TARGET pl
        OUTPUTS y.tab.cc  y.tab.h)
```

通过指定属性，用g++来编译.c结尾的代码，以免造成.cc .c编译后链接出现找不到c++标准库的错误。

```
set_source_files_properties(  y.tab.c PROPERTIES LANGUAGE CXX )
set_source_files_properties(  lex.yy.c PROPERTIES LANGUAGE CXX )
```



## 样例检测

例1 检查类型运算。

```
void main() {
  int x;
  double y;

  x = 5 % 3;
  y = x - 3.5;
  y = 3.0;
}


*** Error line 6.
  y = x - 3.5;
        ^
*** Incompatible operands: int - double
```

例2 还是检查运算时候的类型。

```
void main() {
  int x;
  bool b;

  b = true;
  b = (x > 5) && b;
  x = b;
  x = x/2;
  x = 4 + 1.0 / 5 * 3;
}

*** Error line 7.
  x = b;
    ^
*** Incompatible operands: int = bool


*** Error line 9.
  x = 4 + 1.0 / 5 * 3;
              ^
*** Incompatible operands: double / int

```

## 代码解释与分析

运行的时候的总流程，通过递归识别规约产生式，构建一颗语法树。然后通过BuildScope逐渐构建域。





### StmtBlock

```
List<VarDecl*> *decls;
List<Stmt*> *stmts;
```

其含义时代表一个代码块，所以在BuildScope的时候要注意向其中的decls，stmts中添加元素。Check的时候，就检查decls和stmts中的元素是否合理即可。











## 输入重定向

如果需要将输入从文件重定向至应用程序的 stdin，您现在可以执行该操作。 在配置中使用名为 *Redirect input from* 的新字段。 输入:

- 相对路径（CLion 会预置工作目录路径 ）。
- 绝对路径（将针对运程配置重映射）。
- 或 macros（如 *FilePrompt*）。

https://stackoverflow.com/questions/32971803/clion-standard-input-while-debugging



编译报错

undefined reference to `typeinfo for ClassDecl'

 undefined reference to `vtable for Student'

一般未实现基类定义纯虚函数，子类一定要实现基类的虚函数吗？
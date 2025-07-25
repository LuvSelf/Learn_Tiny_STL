MyTinySTL
=====
[![Build Status](https://travis-ci.org/Alinshans/MyTinySTL.svg?branch=master)](https://travis-ci.org/Alinshans/MyTinySTL) [![Build Status](https://ci.appveyor.com/api/projects/status/github/Alinshans/MyTinySTL?branch=master&svg=true)](https://ci.appveyor.com/project/Alinshans/mytinystl) [![Release](https://img.shields.io/github/release/Alinshans/MyTinySTL.svg)](https://github.com/Alinshans/MyTinySTL/releases) [![License](https://img.shields.io/badge/License-MIT%20License-blue.svg)](https://opensource.org/licenses/MIT) [![Chat](https://img.shields.io/badge/chat-on%20gitter-FF6EB4.svg)](https://gitter.im/alinshans/MyTinySTL)

## 简介
   基于 `C++11` 的 `tinySTL`，这是我的第一个项目，使用了中文文档与中文注释，有不规范或不当的地方还请海涵。刚开始是作为新手练习用途，直到现在已经发布了 `2.x.x` 版本。实现了大部分 STL 中的容器与函数，但仍存在许多不足与 bug 。从 `2.x.x` 版本开始，本项目会进入长期维护的阶段，即基本不会增加新的内容，只修复发现的 bug。如发现错误，还请在 [`Issues`](https://github.com/Alinshans/MyTinySTL/issues) 中指出，欢迎 `Fork` 和 [`Pull requests`](https://github.com/Alinshans/MyTinySTL/pulls) 改善代码，谢谢！

## 支持

* 操作系统
  * linux
  * windows
  * osx
* 编译器
  * g++ 5.4 或以上
  * clang++ 3.5 或以上
  * msvc 14.0 或以上

## 需要
  * 使用 cmake 2.8 来构建项目（**可选**）

## 运行

如果你想要运行测试，请先阅读 [这个](https://github.com/Alinshans/MyTinySTL/blob/master/Test/README.md) 。

  * gcc/clang on linux/osx
  1. 克隆仓库
```bash
$ git clone git@github.com:Alinshans/MyTinySTL.git
$ cd MyTinySTL
```
  2. 构建并运行
```bash
$ mkdir build && cd build
$ cmake ..
$ make
$ cd ../bin && ./stltest
```

  * msvc on windows
  1. 克隆仓库或 [Download ZIP](https://github.com/Alinshans/MyTinySTL/archive/master.zip)
  2. 使用 `vs2015`（或 `vs2017`）打开 `MSVC/MyTinySTL_VS2015.sln`，配置成 `Release` 模式，（Ctrl + F5）开始执行。
  
## 文档
  见 [Wiki](https://github.com/Alinshans/MyTinySTL/wiki)。

## 测试
  见 [Test](https://github.com/Alinshans/MyTinySTL/tree/master/Test)。
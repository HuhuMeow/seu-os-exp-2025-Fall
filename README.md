![SEU](https://img.shields.io/badge/SEU-Southeast%20University-blue)
![Kernel](https://img.shields.io/badge/Linux-6.18-yellow)
![Arch](https://img.shields.io/badge/Arch-ARM%20aarch64-red)
![License](https://img.shields.io/badge/License-MIT-green)

> [!NOTE]
>
> 实验报告中的全部实验流程于 **2025年1月6日** 确认有效。

## Project Introduction / 项目简介

This repository contains my experimental reports and part of the source code for the "Advanced Operating Systems Laboratory" course at Southeast University (Fall 2025).

本仓库包含本人在东南大学 2025 年秋季学期《操作系统专题实践》课程的实验报告与部分源码。

##  初衷

这门课程的指导手册基于较旧的 Linux 2.6.21.7 内核，很多方法已不再适用。本人在 **Linux 6.18** 内核及 **ARM 架构**下重新进行了实验，并将实验报告撰写的尽可能详细，希望能为同学们提供参考和帮助。

若您发现了报告中的错误，或验证了实验流程的有效性，欢迎通过邮件与我联系，或直接提交 Pull Request。

## 用法

**实验报告可在 Release 中找到。**

本项目结构如下：

```
.
├── code			# 部分源码
│   ├── fs				# 第三个实验
│   ├── sh				# 第二个实验
│   └── utilities	# 第一个实验
├── doc				# 在此目录下执行 make 即可编译，执行 make clean 清除临时文件
│   ├── assets		# 模板需要，不建议删
│   ├── chapters	# 实验报告正文
│   ├── fonts			# 模板需要，不建议删
│   ├── img				# 实验报告插图
│   ├── main.tex	
│   ├── makefile
│   ├── ref				# 模板需要，不建议删
│   └── temp.cls	# 模板
├── LICENSE
└── README.md
```

- `code` 文件夹下是可能会用到的代码文件。虽然所有的源码都可以在文档中找到，但如果您有快速复现实验的需求，可以直接在 `code` 文件夹下找到代码文件。`utilities` 文件夹对应第一个实验，`sh` 和 `fs` 对应第二个和第三个。
- 在修改内核代码时，您可能仍需要参考文档。不建议您直接从文档中复制代码，因为文档中的代码可能包含预期之外的空格和换行，在拷贝文档中代码时请再三检查。
- `doc` 文件夹下是实验报告的 LaTeX 源码，如何编译请见下节。如果您有编辑实验报告的需求，只需要修改 `chapters` 文件夹、`img`文件夹和 `main.tex` 文件中的内容。理论上，`assets` 文件夹、`fonts` 文件夹和 `ref` 文件夹都是非必要的，但除非您清楚自己在做什么，不建议去掉这三个文件夹。删除这三个文件夹可能会导致编译错误。

## 编译 LaTeX 文件

在 `doc` 目录下执行  `make`  即可编译，执行 `make clean` 清除编译时产生的临时文件，此方法在 macOS 上被确认为有效。

如果您是 Windows 用户，我强烈推荐您在 Linux 虚拟机中进行编译。如果您有在 Windows 下直接编译的强烈欲望，可以在 [Teddy-van-Jerry/seuthesis2024b(东南大学 2024 届本科毕设 LaTeX 模板) ](https://github.com/Teddy-van-Jerry/seuthesis2024b) 的 `README` 中查找解决方案。

## 题外话

有同学在座谈会上跟教务说，这门课的官方手册太旧了，要求更新。学校现在大力教改，这个事大概率是能成的。但是，**对于大部分同学而言，这门课最好的解法就是对着陈旧的官方手册花一下午把三个实验全速通了**。如果想速通这门课，老老实实用官方手册就是效率最高的，因为官方手册十分轮椅，照着抄所有实验都能跑通。回来真把手册换成计网那种说不明白话的古神PPT，让大家花好几个星期干这门就一个学分的选修，谁受的了？想用最新的内核，自己研究不完了吗？你都有水平用最新的内核了还在乎学校给的手册干什么呢？这课又没强制要求用什么工具，如果你愿意花时间，就可以用最新的内核。对于第二个实验，用 rust ，甚至用 python 做都无所谓的。古董轮椅也是轮椅，新鲜出炉的毒药照样是毒药。大家要珍惜轮椅啊。

## License

本项目的实验报告使用了 [Teddy-van-Jerry/seuthesis2024b(东南大学 2024 届本科毕设 LaTeX 模板) ](https://github.com/Teddy-van-Jerry/seuthesis2024b) 作为模板。
原项目和本项目均遵循 MIT 协议。

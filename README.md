### 1. 项目概述

#### **项目背景**

##### **项目目的**

- 通过对网络中传输的数据包进行深入分析，提取关键特征信息。这一过程包括数据的收集和预处理，也涉及到数据分析技术，以便从网络流量中提取用于识别并分类各种应用程序的特征数据。
- 通过使用机器学习和人工智能算法，项目旨在训练出能够高效、准确识别网络流量类型的分类器。可以提高网络管理的效率，还能增强网络安全，通过识别和过滤不安全或非法的流量来保护网络环境。

##### **项目目标**

- **高效率数据包处理**：开发和优化数据包的读取和处理方法，以实现对大规模网络流量的快速、实时分析。这涉及到高性能计算技术的应用，包括并行处理和数据流管理，以减少数据处理时间，确保系统能够即时响应网络活动的变化。
- **提高分类准确性**：通过精细化的特征提取技术和高级的分类算法，提高应用分类的准确率。需要持续优化算法模型，增强其对不同网络行为模式的识别能力，以准确区分正常流量与异常流量、合法应用与恶意软件等。
- **用户友好的交互界面**：开发一个直观、易用的界面，使用户能够轻松地配置分析参数、查看分类结果和统计数据。这将使得非专业用户也能够有效地使用系统，进一步扩大其应用范围。
- 通过实现这些目标，旨在为网络管理提供一个工具，以提高网络的性能、安全性和管理效率，对抗日益复杂的网络威胁，同时为研究人员和网络管理员提供深入分析网络流量的能力。

#### **技术栈**：

##### 前端技术

- **HTML/CSS/JavaScript**基础技术栈，用于构建网页的结构、样式和交互逻辑
  - **HTML **：在网络流量分析的应用场景中，HTML可以被用来设计直观的用户界面，展示分析结果和实时流量数据。
  - **CSS **：用于定制网页的视觉样式和美观。CSS允许开发者为网页元素应用各种视觉样式，包括颜色、字体、间距和布局等，使得复杂的数据和分析结果更加易于理解。
  - **JavaScript**：在网络流量分析项目中，JavaScript的使用可以极大地提高交互性，例如实现动态图表展示流量趋势，或者提供用户交互式查询和过滤网络流量数据的能力。

##### **后端技术**

- **编程语言：C++/Python**
  - 使用C++可以高速处理网络流量包，并将计算得到的协议特征、包特征、流特征等写入数据库
  - 使用python从数据库中读取数据，创建API由前端javascript调用，实现数据的实时读取与刷新
- **框架：Flask**
  - Flask可以用来开发用于展示网络流量分析结果的Web界面，利用其提供的模板引擎（如Jinja2）快速开发动态网页，展示实时数据、图表、分析结果等。
  - 可以创建用户友好的Web界面，允许用户自定义查询条件、选择分析的数据集，以及调整分析参数。

- **数据库：MySQL**
  - **数据存储**：MySQL可以存储大规模的网络流量数据，包括请求的源和目的地地址、时间戳、传输协议、数据包大小等数据。
  - **数据分析**：支持一些简单的数据分析操作，如聚合查询（例如，计算某个时间段内的流量总量）、排序和分组。对于更复杂的分析需求，数据通常会被导出到专门的分析工具或平台中进行处理。

- **其他技术**

  - **版本控制（Git）**:在网络流量分析中主要用于版本控制，确保分析脚本和应用代码的追踪、更新与协作。

  - **代码风格和质量工具（Prettier）**: 在网络流量分析项目中可提高代码的可读性和一致性，维持代码风格一致性和质量。

### 2. 开发环境设置

#### **环境要求**

- Ubuntu 20.04.6 LTS 20.04

- C++ 14

  - PcapPlusPlus-22.11

    - ```
      准备
      sudo apt-get install libpcap-dev
      wget https://github.com/seladb/PcapPlusPlus/archive/refs/tags/v22.11.tar.gz
      
      安装
      tar -xzvf PcapPlusPlus-22.11.tar.gz
      cd PcapPlusPlus-22.11
      sudo ./configure-linux.sh
      
      过程：
      ****************************************
      PcapPlusPlus Linux configuration script 
      ****************************************
      
      Number of arguments: 0
      
      Compile PcapPlusPlus with PF_RING? N
      Compile PcapPlusPlus with DPDK? Y
      Enter DPDK source path:  /usr/local/share/dpdk
      PcapPlusPlus configuration is complete. Files created (or modified): mk/platform.mk, mk/PcapPlusPlus.mk, mk/install.sh, mk/uninstall.sh
      
      make
      sudo make install
      
      安装成功：
      Installation complete!
      ```

  - MySQL Connector/C++ 8.0.0

    - ```
      sudo apt-get update
      sudo apt-get install libmysqlcppconn-dev
      ```

- Python 3.8.10

  - blinker                1.7.0
  - Flask                  3.0.2
  - importlib_metadata     7.1.0
  - itsdangerous           2.1.2
  - Jinja2                 3.1.3
  - MarkupSafe             2.1.5
  - mysql-connector-python 8.3.0
  - Werkzeug               3.0.1

### 3.代码结构

#### 前端代码结构

```
.
├── static
│   ├── css
│   │   ├── button.css 选项按钮设计
│   │   ├── comon0.css 主体内容设计
│   │   └── comon1.css 导航栏设计
│   ├── font 使用的字体
│   ├── images 显示的图片
│   └── js
│       ├── area_echarts.js 选中地图上某一块区域
│       ├── china.js 显示中国地图
│       ├── date.js 时间选择
│       ├── echarts.min.js 引入echarts库
│       ├── jquery.countup.min.js
│       ├── jquery.js 引入jQuery库
│       ├── jquery.liMarquee.js
│       ├── js.js 绘制具体的echarts图表
│       ├── moment.js 时间格式设置
│       ├── moment.min.js
│       └── selectdata.js 时间选择按钮
├── templates
│   ├── index.html 监控大屏
│   └── traffic_detail.html 流量概要页面
└── app.py
```

#### 后端代码结构

```markdown
.
├── Bin
│   └── TLSFingerprinting流量指纹识别可执行文件
├── Inc
│   ├── feature.h 包含包特征、包间特征、流特征、流间特征、协议特征等的头文件
│   ├── flow.h 定义了一个Flow类，用于计算各个特征
│   ├── global.h定义了一些全局变量
│   ├── json.hpp 包含json的头文件
│   ├── predict.h定义应用的SVM分类器
│   └── tls_features.h 定义了TLS指纹的相关结构体和函数
├── mk
│   └── PcapPlusPlus.mk 引入PcapPlusplus库
├── Src
│   ├── db.cpp 定义了将特征写入数据库的函数
│   ├── flow.cpp 定义了解析流的函数
│   ├── fun.cpp 定义了计算各个特征的函数
│   ├── main.cpp 主函数，用于读取包、解析包等
│   └── tls_features.cpp 定义提取TLS指纹的相关函数
├── batch.sh 调用TLSFingerprinting批量处理pcap文件生成对应的特征文件
├── dbconfig.ini 存储数据库信息的配置文件
├── makefile 定义了makefile编译文件
└── name2seq.py 生成各个文件对应的label
```

### 4. API和特征文档

#### API

- Doxygen
- 在Doxyfile所在目录执行doxygen Doxyfile

- http://127.0.0.1:3000/Encry/tls_feat_extraction/html/index.html

#### 特征

- https://www.yuque.com/leeezhuo/mw92rz/ogg17qq0vhmyrym8?singleDoc# 

### 5. 快速使用

1. **离线特征提取：tls_feat_extraction**

```
编译后运行sudo bash batch.sh

运行条件：
Makefile：修改Pcapplusplus路径
batch.sh 修改dataset路径
dbconfig.ini：配置mysql数据库
main.cpp 修改数据库名称
```

2. **离线训练：classification**

```
输入：tls_feat_extraction\Output
输出：model

运行条件(单模型训练)：
build_dataset.py 
train_agent.py

运行条件(多模型训练)：
merge_dataset.py 
ovr.py
```
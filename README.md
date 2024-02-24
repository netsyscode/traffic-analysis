# 电信项目：加密流量分类

### 1.离线特征提取：tls_feat_extraction

输入：dataset

输出：tls_feat_extraction\Output

编译后运行

```
sudo bash batch.sh
```

运行条件：

```
Makefile Pcapplusplus地址
batch.sh 修改dataset路径
```


### 2.离线训练：classification

输入：tls_feat_extraction\Output

输出：model

运行条件(单模型训练)：

```
build_dataset.py 
train_agent.py
```

运行条件(多模型训练)：

```
merge_dataset.py 
ovr.py
```



### 3.测试：online

输入：网卡端口

输出：online\Output

编译后运行

```
sudo ./Bin/TLSFingerprinting -r "file-path"
```


具体文件包括Src与Inc中的所有文件

```
Src/main.cpp 主文件
Src/flow.cpp 提取流特征
Src/tls_features.cpp 提取TLS特征
Inc/flow.h, tls_features.h 对应支持
Inc/json.hpp json支持
Inc/predict.h SVM支持
```

Src/main.cpp 具体函数：

```
listInterfaces()
功能：列出所有可用的网络接口。
参数：无。
返回：无。

onApplicationInterrupted(void cookie)*
功能：当应用程序被中断时（例如，通过Ctrl+C）调用的回调函数。
参数：一个指向布尔值的指针，指示是否应停止应用程序。
返回：无。

writeToOutputFile(const HandlePacketData data)*
功能：将指纹数据写入输出文件。
参数：包含输出文件、统计信息和流数据的结构体的指针。
返回：无。

handlePacket(RawPacket rawPacket, const HandlePacketData data)**
功能：处理每个捕获的数据包，检查其是否为IP数据包和SSL/TLS数据包，并提取TLS指纹。
参数：原始数据包和包含输出文件、统计信息和流数据的结构体的指针。
返回：无。

calculateFlowFeature(const HandlePacketData data)*
功能：计算流特性。
参数：包含输出文件、统计信息和流数据的结构体的指针。
返回：无。

doTlsFingerprintingOnPcapFile(...)
功能：从pcap文件中提取TLS指纹。
参数：输入pcap文件名、输出文件名和BPF过滤器。
返回：无。

doTlsFingerprintingOnLiveTraffic(...)
功能：从实时网络接口捕获流量并提取TLS指纹。
参数：接口名称或IP、输出文件名、BPF过滤器和模型路径。
返回：无。

classficationFlows(const HandlePacketData data, SVMPredictor model)**
功能：使用SVM预测器模型对流进行分类。
参数：包含输出文件、统计信息和流数据的结构体的指针以及SVM预测器模型的指针。
返回：无。
```

Inc/predict.h 具体介绍

```
类名：SVMPredictor
私有成员：
support_vectors：三维的向量，存储支持向量。
dual_coef：三维向量，存储对偶系数。
intercept：一维向量，存储截距。
mean：一维向量，存储平均值。
std：一维向量，存储标准差。
gamma：一个双精度浮点数，存储径向基函数（RBF）核的参数。
rbf_kernel：计算RBF核的函数。
公共成员：
构造函数 SVMPredictor(const std::string& jsonFilePath, double gammaValue)：从JSON文件中读取模型参数，并初始化gamma的值。
standardize：标准化函数，对输入的一维向量进行标准化处理。
predict：预测函数，对输入的一维向量进行分类，并返回预测的类别。
predictWithDistances：预测函数，对输入的一维向量进行分类，并返回预测的类别和该类别到决策边界的距离。
predictForPredictedClass：预测函数，对输入的一维向量进行分类，并返回预测的类别和该类别的得分。
displayPredictionWithDistances：显示预测结果及其与决策边界的距离。
```


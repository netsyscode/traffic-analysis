#include <netinet/in.h>
#include <algorithm>
#include <regex>
#include "global.h"


/**
 * @brief 在单条流结束时计算相关特征
 * @param download_flag 该条流是否是下载流
 */
void Flow::terminate(bool download_flag)
{
    // 包间特征
    PacketsFeature packetsFeature;
    // packetsFeature.app_label = app_label;
    
    packetsFeature.max_packet_size = max_packet_size;
    packetsFeature.min_packet_size = min_packet_size;
    packetsFeature.max_interval_between_packets = max_interval_between_packets;
    packetsFeature.min_interval_between_packets = min_interval_between_packets;
	// 包长的中位数、标准差、偏度和峰度
	packetsFeature.median_packet_size = calculateMedian(packets_size);
	packetsFeature.std_packet_size = calculateStandardVariance(packets_size);
	packetsFeature.packet_size_skewness = calculateSkewness(packets_size);
	packetsFeature.packet_size_kurtosis = calculateKurtosis(packets_size);

	packetsFeature.packet_rate = packet_cnt_of_pcap  / (timestamp_of_last_packet - timestamp_of_first_packet) * 1e9;
	packetsFeature.packet_rate = bytes_cnt_of_pcap  / (timestamp_of_last_packet - timestamp_of_first_packet) * 1e9;
	packetsFeature.psh_between_time = calculateAverage(psh_interval_vec);
	packetsFeature.urg_between_time = calculateAverage(urg_interval_vec);
    
	// 包间时延的中位数和标准差
	packetsFeature.median_packet_interval = calculateMedian(interval_vec);
	packetsFeature.std_packet_interval = calculateStandardVariance(interval_vec);
    data.packetsFeatureVector->push_back(packetsFeature);

	double duration = (double(latest_timestamp) - start_timestamp) / 1e9;//秒为单位
	flowFeature.dur = duration;

	if (cnt_len_over_1000 >= 1)
		flowFeature.ave_pkt_size_over_1000 = (double)ave_pkt_size_over_1000 / cnt_len_over_1000;
	else flowFeature.ave_pkt_size_over_1000 = 0;

    // 吞吐率
	if (latest_timestamp == start_timestamp) {
		bandwith = 0;
	}else{
		bandwith = (double)packets_size_sum / duration * 8 / 1e3;//单位是kbps	
	}

	// 计算 len < 300的平均包长
	if (cnt_len_under_300 >= 1)
		flowFeature.ave_pkt_size_under_300 = (double)ave_pkt_size_under_300 / cnt_len_under_300;

	// 计算RTT
	if(!rtts.empty()){
		double sum = std::accumulate(rtts.begin(), rtts.end(), 0);
		flowFeature.ave_rtt = static_cast<double>(sum) / rtts.size();
		flowFeature.pktlen = packets_size_sum;
	}
	if (pkt_count >= 1){
		flowFeature.ret_rate = double(ret) / pkt_count;
		flowFeature.udp_nopayload_rate = double(udp_nopayload_cnt) / pkt_count;
		ave_pkt_size = double(packets_size_sum) / pkt_count;
		flowFeature.bytes_of_payload = payload_size;
	}

	if (pkt_count > 1){
		ave_interval = duration / double(pkt_count-1);
	}
	packet_cnt_of_pcap += pkt_count;
	bytes_cnt_of_pcap += packets_size_sum;

	flowFeature.bytes_of_flow = packets_size_sum;


	flowFeature.header_of_packets = packets_size_sum - payload_size;
    packets_size_sum = 0;//统计的是一条流的字节数
	flowFeature.bytes_of_ret_packets = ret_bytes;
	flowFeature.count_of_TCPpackets = tcp_packets;
	flowFeature.count_of_UDPpackets = udp_packets;
	flowFeature.count_of_ICMPpackets = icmp_packets;

    flowFeature.itvl = ave_interval;
	flowFeature.bw = bandwith;
	flowFeature.pktlen = ave_pkt_size;
	flowFeature.pktcnt = pkt_count;

	flowFeature.end_to_end_latency = flowFeature.dur / pkt_count;// 有问题
	flowFeature.avg_window_size = static_cast<double>(window_size_ls) / pkt_count;
	flowFeature.avg_ttl = static_cast<double>(ttl) / pkt_count;
	flowFeature.avg_payload_size = static_cast<double>(payload_size) / pkt_count;
	flowFeature.count_of_ret_packets = ret;
	flowFeature.entropy_of_payload = calculateEntropy(frequencyMap);

	videoMetrics.bitrate = flowFeature.bytes_of_flow / flowFeature.dur;

	// 如果是下载流
	if(download_flag) {
		downloadMetrics.download_session_count++;
		downloadMetrics.duration += flowFeature.dur;
		downloadMetrics.download_bytes += flowFeature.bytes_of_flow;
		downloadMetrics.packet_loss_count += flowFeature.count_of_ret_packets;
	}

    auto key = this->flowKey;
    std::map<FlowKey, Flow*>::iterator flowIter = data.flows->find(key);
    if (flowIter != data.flows->end()) {
        const FlowFeature* flowFeatureInfo = &(flowIter->second->flowFeature);
        if (flowFeatureInfo->tlsFingerprint != NULL) {
            auto clientDigest = flowFeatureInfo->tlsFingerprint->clientHelloFingerprint.toDigest().digest;
            auto serverDigest = flowFeatureInfo->tlsFingerprint->serverHelloFingerprint.toDigest().digest;

            flowFeature.client_TLS_version = std::to_string((u_int32_t) clientDigest[0]);
            flowFeature.client_cipher_suite = std::to_string((u_int32_t) clientDigest[1]);
            flowFeature.client_extensions = std::to_string((u_int32_t) clientDigest[2]);
            flowFeature.client_supported_groups = std::to_string((u_int32_t) clientDigest[3]);
            flowFeature.client_ecformat = std::to_string((u_int32_t) clientDigest[4]);

            flowFeature.server_TLS_version = std::to_string((u_int32_t) serverDigest[0]);
            flowFeature.server_cipher_suite = std::to_string((u_int32_t) serverDigest[1]);
            flowFeature.server_extensions = std::to_string((u_int32_t) serverDigest[2]);
        }
    }

    flowFeature.app_label = flow2app[{key.srcIP.toString(), key.dstIP.toString(), key.srcPort, key.dstPort}];
    flowFeature.startts = nanosecondsToDatetime(start_timestamp);
    flowFeature.srcIP = flowKey.srcIP.toString();
    flowFeature.dstIP = flowKey.dstIP.toString();
    flowFeature.srcPort = flowKey.srcPort;
    flowFeature.dstPort = flowKey.dstPort;
    flowFeature.bw = bandwith;

    flowFeature.max_size_of_packet = max_packet_size;
    flowFeature.min_size_of_packet = min_packet_size;

    // 所有是txt文件有对应的应用的流才写入数据库
    if(flowFeature.app_label != ""){
        // std::cout << key.srcIP.toString() << " " << key.dstIP.toString() << " " << key.srcPort << " " << key.dstPort << " " << flowFeature.app_label << std::endl;
        data.flowFeatureVector->push_back(flowFeature);
    }
}

Flow::Flow(FlowKey flowKey)//构造函数
{
	this->flowKey = flowKey;
	this->flowFeature = FlowFeature();
    // 初始化时间戳
    latest_timestamp = 0;
	latter_timestamp = 0;
    start_timestamp = 0;

    // 初始化包计数
    pkt_count = 0;
    window_size_ls = 0;
	ttl = 0;

    // 初始化大小相关的变量
    packets_size_sum = 0;
    app_packets_size_sum = 0;
    header_len_ls = 0;

    // 初始化带宽、吞吐量等
    bandwith = 0;
    throughput = 0;
	duration = 0;

    // 初始化平均值和最大最小值
    ave_pkt_size = 0;
    app_ave_pkt_size = 0;
    ave_interval = 0;
    ave_windows = 0;
    ave_rtt = 0;
    interval_ls = 0;
    rtt_ls = 0;

	tcp_packets = 0, udp_packets = 0, icmp_packets = 0;

    // 初始化其它计数和分类变量
    udp_nopayload_cnt = 0;
    cnt_len_over_1000 = 0;
    ave_pkt_size_over_1000 = 0;
    cnt_len_under_300 = 0;
    ave_pkt_size_under_300 = 0;
    payload_size = 0;
	ret_bytes = 0;
    payload_bandwidth = 0;

	ackBuffer = 0;
    packetBuffer = 0;
    ret = 0;
}

/**
 * @brief 生成TCP连接信息
 * @return 形如1.1.1.1:1->2.2.2.2:2的字符串
 */
std::string FlowKey::toString() const
{	
	return srcIP.toString() +":"+ std::to_string(srcPort)+" -> " + dstIP.toString() +':'+ std::to_string(dstPort);
}


bool FlowKey::operator<(const FlowKey& e) const
{	
	if (this->srcIP.toString() < e.srcIP.toString()) return true;
	if (this->srcIP.toString() > e.srcIP.toString()) return false;

	if (this->dstIP.toString() < e.dstIP.toString()) return true;
	if (this->dstIP.toString() > e.dstIP.toString()) return false;

	if (this->srcPort < e.srcPort) return true;
	if (this->srcPort > e.srcPort) return false;

	return dstPort < e.dstPort;
}

bool FlowKey::operator==(const FlowKey& e) const
{		
	return (this->srcIP == e.srcIP) && (this->dstIP == e.dstIP) && 
		(this->srcPort == e.srcPort) && (this->dstPort == e.dstPort);
}

std::pair<IPAddress, IPAddress> getIPs(const Packet* packet)
{
	IPAddress srcIP, dstIP;
	if (packet->isPacketOfType(IPv4))
	{
		const IPLayer* ipLayer = packet->getLayerOfType<IPLayer>();
		srcIP = ipLayer->getSrcIPAddress();
		dstIP = ipLayer->getDstIPAddress();
	}
	return std::pair<IPAddress, IPAddress>(srcIP, dstIP);
}


std::pair<uint16_t, uint16_t> getTcpPorts(const Packet* packet)
{
	uint16_t srcPort = 0, dstPort = 0;
	if (packet->isPacketOfType(TCP))
	{
		TcpLayer* tcpLayer = packet->getLayerOfType<TcpLayer>();
		srcPort = tcpLayer->getSrcPort();
		dstPort = tcpLayer->getDstPort();
	}

	return std::pair<uint16_t, uint16_t>(srcPort, dstPort);
}

/**
 * @brief 生成一个Flowkey
 * @param pkt 指向pcpp::Packet类型变量的指针
 * @return 指向所生成Flowkey的指针
 */
FlowKey* generateFlowKey(const Packet* pkt)
{
	FlowKey* flowKey = new FlowKey();
	IPLayer* ipLayer = pkt->getLayerOfType<IPLayer>();
	TcpLayer* tcpLayer = pkt->getLayerOfType<TcpLayer>();
	flowKey->srcIP = ipLayer->getSrcIPAddress();
	flowKey->dstIP = ipLayer->getDstIPAddress();
	if (tcpLayer != NULL) {
		flowKey->srcPort = tcpLayer->getSrcPort();
		flowKey->dstPort = tcpLayer->getDstPort();
	}
	else {
		UdpLayer* udpLayer = pkt->getLayerOfType<UdpLayer>();
		if (udpLayer != NULL) {	
			flowKey->srcPort = udpLayer->getSrcPort();
			flowKey->dstPort = udpLayer->getDstPort();
		}
		else
		{
			delete flowKey;
			return NULL;
		}
	}
	return flowKey;
}

/**
 * @brief 用Flowkey填充SessionKey
 * @param sessionKey 所要填充的sessionKey
 * @param flowKey 填充所需要的flowKey
 * @param fromClient 是从客户端而来的流还是反向的
 */
void fillSessionKeyWithFlowKey(SessionKey& sessionKey, const FlowKey& flowKey, bool fromClient)
{
	if (fromClient)
	{
		sessionKey.clientIP = flowKey.srcIP;
		sessionKey.serverIP = flowKey.dstIP;
		sessionKey.clientPort = flowKey.srcPort;
		sessionKey.serverPort = flowKey.dstPort;
	}
	else
	{
		sessionKey.clientIP = flowKey.dstIP;
		sessionKey.serverIP = flowKey.srcIP;
		sessionKey.clientPort = flowKey.dstPort;
		sessionKey.serverPort = flowKey.srcPort;
	}
}

/**
 * @brief 时间戳形式转换
 * @param nanoseconds 以纳秒为单位待转化的时间戳
 * @return 形如2000-00-00:000000的时间
 */
std::string nanosecondsToDatetime(LL nanoseconds) {
    auto seconds = nanoseconds / 1000000000;
    auto nanosec = nanoseconds % 1000000000;
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(seconds);
    tp += std::chrono::nanoseconds(nanosec);
    // 将time_point转换为tm结构
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm* tm_ptr = std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(6) << nanosec / 1000;
    return ss.str();
}

/**
 * @brief 计算分位数近似拟合数据分布
 * @param a double类型的vector
 * @return 一个vector，包含10、25、50、75和90分位数
 */

std::vector<double> calculatePercentiles(const std::vector<double>& a) {
    if (a.empty()) return {};
    std::vector<double> sorted_a = a; // 复制原向量
    std::sort(sorted_a.begin(), sorted_a.end()); // 排序
    size_t n = sorted_a.size();
    std::vector<double> percentiles;

    auto percentileFunc = [&](double p) -> double {
        double pos = (n - 1) * p;
        size_t left = std::floor(pos);
        size_t right = std::ceil(pos);
        double dpos = pos - left;
        // 线性插值
        if (left == right) 
            return sorted_a[left];
        else 
            return sorted_a[left] * (1 - dpos) + sorted_a[right] * dpos;
    };
    // 计算各个分位数
	percentiles.push_back(percentileFunc(0.10));
    percentiles.push_back(percentileFunc(0.25));
    percentiles.push_back(percentileFunc(0.50));
    percentiles.push_back(percentileFunc(0.75));
	percentiles.push_back(percentileFunc(0.90));
    return percentiles;
}

/**
 * @brief 根据fielName提取data中的数据
 * @param data 存储了所有流的信息
 * @param fieldName[] 头部的一个域的名称
 * @return 一个vector，包含从data中提取所指定域的数据
 */
std::vector<double> getFlowsValues(const HandlePacketData* data, const char fieldName[]) {
    std::vector<double> values;
	// 提取流的持续时间
    if (strcmp(fieldName, "dur")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.dur);
        }
	// 提取流的总包数
    } else if (strcmp(fieldName, "pktcnt")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.pktcnt);
        }
	// 提取流的总字节数	
    } else if (strcmp(fieldName, "bytes_of_flow")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.bytes_of_flow);
        }
	// 提取流的包平均字节数
    } else if (strcmp(fieldName, "avg_bytes_of_flow")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.bytes_of_flow / flowPair.second->flowFeature.pktcnt);
        }
	// 提取流的平均TTL
    } else if (strcmp(fieldName, "avg_ttl")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.avg_ttl);
        }
	// 提取流的平均窗口大小
    } else if (strcmp(fieldName, "avg_window_size")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.avg_window_size);
        }
	// 提取流的平均端到端时延
    } else if (strcmp(fieldName, "end_to_end_latency")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.end_to_end_latency);
        }
	// 提取流的负载熵
    } else if (strcmp(fieldName, "entropy_of_payload")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.entropy_of_payload);
        }
	}
    return values;
}

/**
 * @brief 统计一条流的源端口和目的端口数
 * @param data 存储了所有流的信息
 * @return 一个pair,pair里面是port->count的字典
 */
std::pair<std::map<u_int16_t, int>, std::map<u_int16_t, int>> countFlowsByPorts(const HandlePacketData* data) {
    std::map<u_int16_t, int> srcPortCounts, dstPortCounts;
    for (const auto& flowPair : *data->flows) {
        const Flow* flow = flowPair.second;
        
        // 统计源端口
        if (srcPortCounts.find(flow->flowFeature.srcPort) == srcPortCounts.end()) {
            srcPortCounts[flow->flowFeature.srcPort] = 1;
        } else {
            srcPortCounts[flow->flowFeature.srcPort]++;
        }
        // 统计目的端口
        if (dstPortCounts.find(flow->flowFeature.dstPort) == dstPortCounts.end()) {
            dstPortCounts[flow->flowFeature.dstPort] = 1;
        } else {
            dstPortCounts[flow->flowFeature.dstPort]++;
        }
    }
    return {srcPortCounts, dstPortCounts};
}

/**
 * @brief 统计流的源IP和目的IP数
 * @param data 存储了所有流的信息
 * @return 一个pair,pair里面是IP->count的字典
 */
std::pair<std::map<std::string, int>, std::map<std::string, int>> countFlowsByIP(const HandlePacketData* data) {
    std::map<std::string, int> srcIPCounts, dstIPCounts;
    for (const auto& pair : *data->flows) {
        const Flow* flow = pair.second;
        std::string srcIP = flow->flowKey.srcIP.toString(); 
        std::string dstIP = flow->flowKey.dstIP.toString(); 
        srcIPCounts[srcIP]++;
        dstIPCounts[dstIP]++;
    }
    return {srcIPCounts, dstIPCounts};
}

/**
 * @brief 判断是否是公网IP
 * @param ipAddress 一个点分十进制的IP字符串
 * @return 是否是公网IP
 */
bool isPrivateIP(const std::string& ipAddress) {
    std::vector<int> nums;
    std::stringstream ss(ipAddress);
    std::string item;
    while (std::getline(ss, item, '.')) {
        nums.push_back(std::stoi(item));
    }

    if (nums.size() != 4) {
        return false;
    }

    int first = nums[0];
    int second = nums[1];

    // A类私网IP
    if (first == 10) {
        return true;
    }
    // B类私网IP
    if (first == 172 && second >= 16 && second <= 31) {
        return true;
    }
    // C类私网IP
    if (first == 192 && second == 168) {
        return true;
    }

    return false;
}

// 计算平均值
double calculateAverage(std::vector<double>& vec) {
    if(!vec.empty())
        return std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
    else
        return 0;
}

// 计算中位数
double calculateMedian(std::vector<double>& vec) {
    size_t size = vec.size();
    std::sort(vec.begin(), vec.end());
    if (size % 2 == 0) {
        return (vec[size / 2 - 1] + vec[size / 2]) / 2.0;
    } else {
        return vec[size / 2];
    }
}

// 计算标准差
double calculateStandardVariance(std::vector<double>& vec) {
    double mean = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
    double variance = 0.0;
    for (int value : vec) {
        variance += (value - mean) * (value - mean);
    }
    return sqrt(variance / vec.size());
}

/**
 * @brief 计算偏度
 * @param packets_size 存储包长的vector
 * @return 包长的偏度
 */
double calculateSkewness(const std::vector<double>& packets_size) {
    if (packets_size.size() < 3 ) {
        // 数据量太少，无法计算偏度
        return 0.0;
    }
    double mean = std::accumulate(packets_size.begin(), packets_size.end(), 0.0) / packets_size.size();

    double variance = 0.0;
    for (double value : packets_size) {
        variance += (value - mean) * (value - mean);
    }
    variance /= packets_size.size();
    double stdDev = std::sqrt(variance);

    double skewness = 0.0;
    for (double value : packets_size) {
        skewness += std::pow((value - mean) / stdDev, 3);
    }
    skewness = skewness * packets_size.size() / ((packets_size.size() - 1) * (packets_size.size() - 2));

    if (std::isnan(skewness)) {
        skewness = 0.0;
    }
    return skewness;
}

/**
 * @brief 计算峰度
 * @param packets_size 存储包长的vector
 * @return 包长的峰度
 */
double calculateKurtosis(const std::vector<double>& packets_size) {
    size_t n = packets_size.size();
    if (n < 4) {
        // 数据量太少，无法计算峰度
        return 0.0;
    }
    double mean = std::accumulate(packets_size.begin(), packets_size.end(), 0.0) / n;
    double variance = 0.0;
    for (double value : packets_size) {
        variance += (value - mean) * (value - mean);
    }
    variance /= n;
    double stdDev = std::sqrt(variance);
    double kurtosis = 0.0;
    for (double value : packets_size) {
        kurtosis += std::pow((value - mean) / stdDev, 4);
    }
    kurtosis = (kurtosis * n * (n + 1) / ((n - 1) * (n - 2) * (n - 3))) - (3 * std::pow(n - 1, 2) / ((n - 2) * (n - 3)));
    if (std::isnan(kurtosis)) {
        kurtosis = 0.0;
    }
    return kurtosis;
}

/**
 * @brief 提取SPS信息
 * @param rtp_payload RTP负载的起始字节
 * @param  payload_length 负载长度
 * @param sps_length SPS的长度
 * @return SPS数据起始字节的位置
 */
uint8_t* extract_sps_from_rtp(uint8_t* rtp_payload, size_t payload_length, size_t* sps_length) {
    // 确保提供了有效的参数
    if (!rtp_payload || payload_length == 0 || !sps_length) {
        return NULL;
    }

    // 遍历RTP负载以查找SPS NALU
    for (size_t i = 0; i < payload_length - 1; ++i) {
        // NALU起始码通常为0x000001或0x00000001（H.264）
        if (rtp_payload[i] == 0x00 && rtp_payload[i + 1] == 0x00 && rtp_payload[i + 2] == 0x01) {
            // 找到NALU起始码，检查NALU类型
            uint8_t nalu_type = rtp_payload[i + 3] & 0x1F;
            if (nalu_type == 7) { // SPS NALU类型为7
                // 计算SPS数据的长度
                size_t start = i + 4; // SPS数据开始位置
                size_t end = payload_length; // 假设SPS是最后一个NALU
                // 寻找下一个NALU起始码作为结束标志（如果有）
                for (size_t j = start; j < payload_length - 1; ++j) {
                    if (rtp_payload[j] == 0x00 && rtp_payload[j + 1] == 0x00 && rtp_payload[j + 2] == 0x01) {
                        end = j; // 找到下一个NALU起始码，更新结束位置
                        break;
                    }
                }
                *sps_length = end - start;
                // 分配内存并复制SPS数据
                uint8_t* sps_data = (uint8_t*)malloc(*sps_length);
                if (!sps_data) {
                    // 内存分配失败
                    *sps_length = 0;
                    return NULL;
                }
                memcpy(sps_data, rtp_payload + start, *sps_length);
                return sps_data;
            }
        }
    }
    // 未找到SPS NALU
    return NULL;
}

/**
 * @brief 检查HTTP头部字段
 * @param packet 指向pcpp::Packet类型变量的指针
 * @return 是否含有Range字段
 */
bool checkForRangeHeader(const pcpp::Packet& packet) {
    auto* httpLayer = packet.getLayerOfType<pcpp::HttpRequestLayer>();
    if (httpLayer != nullptr) {
        // 检查请求是否包含Range头
        if (httpLayer->getFieldByName("Range") != nullptr) {
            return true;
        }
    }

    auto* httpResponseLayer = packet.getLayerOfType<pcpp::HttpResponseLayer>();
    if (httpResponseLayer != nullptr) {
        // 检查响应是否包含Content-Range头
        if (httpResponseLayer->getFieldByName("Content-Range") != nullptr) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 从ini配置文件中读取数据库配置
 * @param configFile 数据库配置文件
 * @return 各项配置的字典
 */
std::map<std::string, std::string> readConfig(const std::string& configFile) {
    std::map<std::string, std::string> config;
    std::ifstream file(configFile);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (key[0] == '[') { // skip sections
                continue;
            } else if (std::getline(is_line, value)) {
                config[key] = value;
            }
        }
    }
    return config;
}
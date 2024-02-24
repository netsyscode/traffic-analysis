#include "flow.h"
#include "handledata.h"
#include <netinet/in.h>
#include <algorithm>
#include "global.h"
#include <regex>

Flow::Flow(FlowKey flowKey)//构造函数
{
	this->flowKey = flowKey;
	this->flowFeature = FlowFeature();
    // 初始化时间戳
    latest_timestamp = 0;
	latter_timestamp = 0;
    interarrival_time_n = 0;
    interarrival_time_ls = 0;
    flow_duration = 0;
    start_timestamp = 0;
    used_ts = 0;

    // 初始化包计数
    pkt_count = 0;
    rtt_count = 0;
    window_size_ls = 0;
    retrans = 0;
    ack_no_payload = 0;
	ttl = 0;

    // 初始化大小相关的变量
    packets_size_sum = 0;
    app_packets_size_sum = 0;
    header_len_ls = 0;

    // 初始化带宽、吞吐量等
    bandwith = 0;
    appbandwith = 0;
    throughput = 0;
	duration = 0;

    // 初始化平均值和最大最小值
    ave_pkt_size = 0;
    app_ave_pkt_size = 0;
    ave_interval = 0;
    ave_windows = 0;
    ave_rtt = 0;
    rtt_min = 0;
    rtt_max = 0;
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

void Flow::addPacket(RawPacket* pkt)
{
	packets.push_back(pkt);
	updateFeature(pkt);
}

double calculateStandardDeviation(const std::vector<RawPacket*> packets) {
    double sum = 0.0;
    double mean;
    double standardDeviation = 0.0;

    int size = packets.size();
    for(int i = 0; i < size; ++i) {
        sum += packets[i]->getFrameLength();
    }
    mean = sum / size;
    for(int i = 0; i < size; ++i) {
        standardDeviation += pow(packets[i]->getFrameLength() - mean, 2);
    }
    return sqrt(standardDeviation / size);
}

double calculateEntropy(const std::map<uint8_t, int>& frequencyMap) {
    int totalBytes = 0;
    for (const auto& pair : frequencyMap) {
        totalBytes += pair.second; // 累加每个字节的出现次数
    }

    double entropy = 0.0;
    for (const auto& pair : frequencyMap) {
        double probability = static_cast<double>(pair.second) / totalBytes; // 计算概率
        entropy -= probability * log2(probability); // 累加到熵
    }
    return entropy;
}

void Flow::updateFeature(RawPacket* pkt) 
{
	ProtocolInfo protocolInfo;
	PacketInfo packetInfo;
	packetInfo.packet_length = pkt->getFrameLength();
	//统计包长大于1000的包数量
	int pktLen = pkt->getFrameLength();
	if (pktLen > 1000) {
		cnt_len_over_1000++;
		ave_pkt_size_over_1000 += pktLen;
	}else if (pktLen < 300) {
		cnt_len_under_300++;
		ave_pkt_size_under_300 += pktLen;
	}
	
	// 长流截断
	if(pkt_count >= 1e8){
		return;
	}else{
		pkt_count += 1;
	}
	//累加包长
	packets_size_sum += pktLen;//单位
	flowFeature.max_size_of_packet = std::max(flowFeature.max_size_of_packet, pktLen);
	flowFeature.min_size_of_packet = std::min(flowFeature.min_size_of_packet, pktLen);

	//更新最后时间戳(以ns为单位)
	long sec = pkt->getPacketTimeStamp().tv_sec;
	long nsec = pkt->getPacketTimeStamp().tv_nsec;
	latest_timestamp = sec * 1000000000LL + nsec;
	packetInfo.arrival_timestamp = nanosecondsToDatetime(latest_timestamp);

	if(latest_timestamp != latter_timestamp){
		auto diff = (latest_timestamp - latter_timestamp) / 1e9;
		flowFeature.max_interval_between_packets = std::max(flowFeature.max_interval_between_packets, diff);
		flowFeature.min_interval_between_packets = std::min(flowFeature.min_interval_between_packets, diff);
	}

	//payload相关
	pcpp::Packet parsedPacket(pkt);
	pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
    if (tcpLayer != nullptr) {
		protocolInfo.tcp_header_length = tcpLayer->getHeaderLen();
		pcpp::tcphdr* tcpHeader = tcpLayer->getTcpHeader();
		protocolInfo.tcp_window_size = tcpHeader->windowSize;
		window_size_ls += protocolInfo.tcp_window_size;
		
        // The payload length is the TCP payload size
		tcp_packets ++;
        size_t tcpPayloadLength = tcpLayer->getLayerPayloadSize();
		packetInfo.payload_size = tcpPayloadLength;
		if(tcpPayloadLength > 0){
            const uint8_t* payload = tcpLayer->getLayerPayload();
			std::map<uint8_t, int> frequencyPacketMap;
            for (size_t i = 0; i < tcpPayloadLength; ++i) {
                frequencyMap[payload[i]]++;
				frequencyPacketMap[payload[i]]++;
            }
			packetInfo.payload_entropy = calculateEntropy(frequencyPacketMap);
		}
        payload_size += tcpPayloadLength;

		protocolInfo.syn_flag = (tcpHeader->synFlag == 1);
		protocolInfo.fin_flag = (tcpHeader->finFlag == 1);
		protocolInfo.rst_flag = (tcpHeader->rstFlag == 1);
		protocolInfo.psh_flag = (tcpHeader->pshFlag == 1);
		protocolInfo.urg_flag = (tcpHeader->urgFlag == 1);
		flowFeature.count_of_syn_packets += protocolInfo.syn_flag;
		flowFeature.count_of_fin_packets += protocolInfo.fin_flag;
		flowFeature.count_of_rst_packets += protocolInfo.rst_flag;
		flowFeature.count_of_psh_packets += protocolInfo.psh_flag;
		flowFeature.count_of_urg_packets += protocolInfo.urg_flag;
		// RTT近似计算？
        u_int32_t seqNumber = ntohl(tcpHeader->sequenceNumber);//这个包的seq
		protocolInfo.tcp_sequence_number = seqNumber;
		u_int32_t acknoNumber = ntohl(tcpHeader->ackNumber);//这个包的ackno
		protocolInfo.tcp_acknowledgement_number = acknoNumber;
		if(latest_timestamp == start_timestamp) {//如果是第一个包
			ackBuffer = acknoNumber;
			packetBuffer = seqNumber;
		}
		
		if(seqNumber <= packetBuffer){
			// 循环
			if(packetBuffer - seqNumber> 0x7FFFFFFF){
				;
			}else if((seqNumber == packetBuffer) && (ackBuffer == acknoNumber)) { 
					ret += 1;
					ret_bytes += tcpPayloadLength;
			}
		}
		packetBuffer = seqNumber;
		ackBuffer = acknoNumber;
		
        if (tcpHeader->synFlag && !tcpHeader->ackFlag) {//只有syn包会执行
			long sec = pkt->getPacketTimeStamp().tv_sec;
			long nsec = pkt->getPacketTimeStamp().tv_nsec;
			seqToTime[seqNumber] = sec * 1000000000LL + nsec;
			//std::cout << "insert to seq2time!" << std::endl;
        }

		if (tcpHeader->ackFlag) {
            if (seqToTime.count(seqNumber - 1) > 0) {
					uint64_t synTime = seqToTime[seqNumber - 1];
					uint64_t synAckTime = latest_timestamp;
					rtts.push_back(double(synAckTime - synTime) / 1e9);

					auto key = this->flowKey;
					FlowKey keyPal = {key.dstIP, key.srcIP, key.dstPort, key.srcPort};
					std::map<FlowKey, Flow*>::iterator flowIter = data.flows->find(keyPal);
					if (flowIter != data.flows->end())
						flowIter->second->rtts.push_back(double(synAckTime - synTime) / 1e9);
					//seqToTime.erase(seqNumber - 1);
            }
			// 	if (tcpLayer->getTcpHeader()->synFlag && tcpLayer->getTcpHeader()->ackFlag) {
            // //if (seqToTime.count(seqNumber - 1) > 0) {
			// 	auto key = this->flowKey;
			// 	FlowKey keyPal = {key.dstIP, key.srcIP, key.dstPort, key.srcPort};
			// 	std::map<FlowKey, Flow*>::iterator flowIter = data.flows->find(keyPal);
			// 	if (flowIter != data.flows->end()){
			// 		uint64_t synTime = flowIter->second->start_timestamp;
			// 		//synTime = seqToTime[seqNumber - 1];
			// 		long sec = pkt->getPacketTimeStamp().tv_sec;
			// 		long nsec = pkt->getPacketTimeStamp().tv_nsec;
			// 		uint64_t synAckTime = sec * 1000000000LL + nsec;
			// 		flowIter->second->rtts.push_back(double(synAckTime - synTime) / 1e9);//算出来的应该是发起会话的RRT
			// 		rtts.push_back(double(synAckTime - synTime) / 1e9);
			// 		seqToTime.erase(seqNumber - 1);
			// 	//}
            // }
        }
		pcpp::HttpRequestLayer* httpRequestLayer = parsedPacket.getLayerOfType<pcpp::HttpRequestLayer>();
        if (httpRequestLayer != nullptr) {

            // const pcpp::HeaderField* hostField = httpRequestLayer->getFieldByName(PCPP_HTTP_HOST_FIELD);
            // if (hostField != nullptr) {
            //     std::cout << "Host: " << hostField->getFieldValue() << std::endl;
            // }

            // std::cout << "HTTP Request Headers:" << std::endl;
            // for (pcpp::HeaderField* hdr = httpRequestLayer->getFirstField(); hdr != nullptr; hdr = httpRequestLayer->getNextField(hdr)) {
            //     std::cout << hdr->getFieldName() << ": " << hdr->getFieldValue() << std::endl;
            // }
			HttpRequest webrequest;
			webrequest.url = httpRequestLayer->getFirstLine()->getUri();
			webrequest.method = httpRequestLayer->getFirstLine()->getMethod();
			webrequest.httpversion = httpRequestLayer->getFirstLine()->getVersion();

			auto field = httpRequestLayer->getFieldByName("PCPP_HTTP_HOST_FIELD");//encrypted?
			webrequest.host = (field != nullptr) ? field->getFieldValue() : "";
			if(field != nullptr) std::cout << "host : " << webrequest.host << std::endl;

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_CONNECTION_FIELD");
			webrequest.connection = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_USER_AGENT_FIELD");
			webrequest.user_agent = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_REFERER_FIELD");
			webrequest.referer = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_ACCEPT_FIELD");
			webrequest.accept = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_ACCEPT_ENCODING_FIELD");
			webrequest.accept_encoding = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_ACCEPT_LANGUAGE_FIELD");
			webrequest.accept_language = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_COOKIE_FIELD");
			webrequest.cookies = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_CONTENT_LENGTH_FIELD");
			webrequest.content_length = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_CONTENT_ENCODING_FIELD");
			webrequest.content_encoding = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_CONTENT_TYPE_FIELD");
			webrequest.content_type = (field != nullptr) ? field->getFieldValue() : "";

			field = httpRequestLayer->getFieldByName("PCPP_HTTP_SERVER_FIELD");
			webrequest.server = (field != nullptr) ? field->getFieldValue() : "";

			data.WebRequest->push_back(webrequest);
        }

        pcpp::HttpResponseLayer* httpResponseLayer = parsedPacket.getLayerOfType<pcpp::HttpResponseLayer>();
        if (httpResponseLayer != nullptr) {
			HttpResponse webresponse;
			webresponse.status_code = httpResponseLayer->getFirstLine()->getStatusCode();
			webresponse.httpversion = httpResponseLayer->getFirstLine()->getVersion();
            for (pcpp::HeaderField* hdr = httpResponseLayer->getFirstField(); hdr != nullptr; hdr = httpResponseLayer->getNextField(hdr)) {
                auto field = hdr->getFieldName();
				if(field == "Date")
					webresponse.date = hdr->getFieldValue();
				else if(field == "Content-Type")
					webresponse.content_type = hdr->getFieldValue();
				else if(field == "Content-Length")
					webresponse.content_length = hdr->getFieldValue();
				else if(field == "Connection")
					webresponse.connection = hdr->getFieldValue();
				else if(field == "Server")
					webresponse.server = hdr->getFieldValue();
            }
			data.WebResponse->push_back(webresponse);
        }
    }

    // Get the UDP layer
    pcpp::UdpLayer* udpLayer = parsedPacket.getLayerOfType<pcpp::UdpLayer>();
    if (udpLayer != nullptr) {
        // The payload length is the UDP payload size
		udp_packets ++;
		auto udpHeadr = udpLayer->getUdpHeader();
		protocolInfo.udp_header_length = udpHeadr->length;
		protocolInfo.udp_checksum = udpHeadr->headerChecksum;
        size_t udpPayloadLength = udpLayer->getLayerPayloadSize();
		packetInfo.payload_size = udpPayloadLength;
		if(udpPayloadLength > 0){
            const uint8_t* payload = udpLayer->getLayerPayload();
			//entropy
            for (size_t i = 0; i < udpPayloadLength; ++i) {
                frequencyMap[payload[i]]++;
            }
			// 寻找SPS NAL单元起始码
			for (size_t i = 0; i < udpPayloadLength - 4; i++) {
				// 检查NAL单元起始码
				if (payload[i] == 0x00 && payload[i+1] == 0x00 && payload[i+2] == 0x01) {
					uint8_t nalType = payload[i+3] & 0x1F;
					if (nalType == 7) { // SPS类型
						std::cout << "Found SPS packet" << std::endl;
						// 根据需要处理SPS数据
						break;
					}
				}
			}
		}
        payload_size += udpPayloadLength;
		if(udpPayloadLength == 0){
			udp_nopayload_cnt ++;
		}
    }

	pcpp::IcmpLayer* icmpLayer = parsedPacket.getLayerOfType<pcpp::IcmpLayer>();
    if (icmpLayer != nullptr) {
        icmp_packets ++;
		protocolInfo.icmp_code = icmpLayer->getIcmpHeader()->code;
		protocolInfo.icmp_type = icmpLayer->getMessageType();

    }
	
	pcpp::IPv4Layer* ipLayer = parsedPacket.getLayerOfType<pcpp::IPv4Layer>();
    if (ipLayer != nullptr) {
        ttl += ipLayer->getIPv4Header()->timeToLive;
    }

	pcpp::EthLayer* ethLayer = parsedPacket.getLayerOfType<pcpp::EthLayer>();
	if(ethLayer != nullptr){
		protocolInfo.ethernet_type = ethLayer->getEthHeader()->etherType;
	}

	pcpp::EthLayer* ethernetLayer = parsedPacket.getLayerOfType<pcpp::EthLayer>();
	if (ethernetLayer != nullptr) {
		std::string macSrc = ethernetLayer->getSourceMac().toString();
		std::string macDst = ethernetLayer->getDestMac().toString();
		uint16_t ethType = ethernetLayer->getEthHeader()->etherType;
		protocolInfo.mac_source =macSrc;
		protocolInfo.mac_destination = macDst;
		protocolInfo.ethernet_type = ethType;
	}

	pcpp::VlanLayer* vlanLayer = parsedPacket.getLayerOfType<pcpp::VlanLayer>();
	if (vlanLayer != nullptr) {
		uint16_t vlanID = vlanLayer->getVlanID();
		protocolInfo.vlan_id = vlanID;
	}

	pcpp::MplsLayer* mplsLayer = parsedPacket.getLayerOfType<pcpp::MplsLayer>();
	if (mplsLayer != nullptr) {
		uint32_t mplsLabel = mplsLayer->getMplsLabel();
		protocolInfo.mpls_label = mplsLabel;
	}

	pcpp::PPPoELayer* pppoeLayer = parsedPacket.getLayerOfType<pcpp::PPPoELayer>();
	if (pppoeLayer != nullptr) {
		uint16_t pppoeSessionId = pppoeLayer->getPPPoEHeader()->sessionId;
	}

	pcpp::IPv4Layer* ipv4Layer = parsedPacket.getLayerOfType<pcpp::IPv4Layer>();
	if (ipv4Layer != nullptr) {
		uint8_t ipTos = ipv4Layer->getIPv4Header()->typeOfService;
		uint16_t ipChecksum = ipv4Layer->getIPv4Header()->headerChecksum;
		uint8_t ipFragmentationFlag = ipv4Layer->getFragmentFlags();
		uint16_t ipID = ipv4Layer->getIPv4Header()->ipId;
		protocolInfo.ip_tos = ipTos;
		protocolInfo.ip_header_checksum = ipChecksum;
		protocolInfo.ip_fragmentation_flag = ipFragmentationFlag;
		protocolInfo.ip_identifier = ipID;
	}

	pcpp::IPv6Layer* ipv6Layer = parsedPacket.getLayerOfType<pcpp::IPv6Layer>();
	if (ipv6Layer != nullptr) {
		uint8_t* ipv6FlowLabel = ipv6Layer->getIPv6Header()->flowLabel;
		uint8_t ipv6NextHeader = ipv6Layer->getIPv6Header()->nextHeader;
		protocolInfo.ipv6_flow_label = ipv6FlowLabel;
		protocolInfo.ipv6_next_header = ipv6NextHeader;
	}

	pcpp::ArpLayer* arpLayer = parsedPacket.getLayerOfType<pcpp::ArpLayer>();
	if (arpLayer != nullptr) {
		protocolInfo.arp_request = (arpLayer->getArpHeader()->opcode == pcpp::ARP_REQUEST);
		protocolInfo.arp_reply = (arpLayer->getArpHeader()->opcode == pcpp::ARP_REPLY);
	}

	pcpp::DnsLayer* dnsLayer = parsedPacket.getLayerOfType<pcpp::DnsLayer>();
	if (dnsLayer != nullptr) {
		// Iterate over DNS queries if exist
		if (dnsLayer->getFirstQuery() != nullptr) {
			for (pcpp::DnsQuery* query = dnsLayer->getFirstQuery(); query != nullptr; query = dnsLayer->getNextQuery(query)) {
				std::string dnsQueryName = query->getName(); 
				pcpp::DnsType dnsQueryType = query->getDnsType(); 
			}
		}
	}
	// 解析 SMTP 和 FTP 命令/响应
	pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
	if (tcpLayer != nullptr) {
		std::string payload(reinterpret_cast<const char*>(tcpLayer->getLayerPayload()), tcpLayer->getLayerPayloadSize());
		std::regex commandRegex("^(EHLO|HELO|MAIL FROM|RCPT TO|DATA|QUIT|220|250|550|USER|PASS|RETR|STOR).*");
		std::smatch matches;
		if (std::regex_search(payload, matches, commandRegex)) {
			protocolInfo.smtp_command = matches[1];
		}
	}

	// 解析 DHCP 消息类型
	pcpp::DhcpLayer* dhcpLayer = parsedPacket.getLayerOfType<pcpp::DhcpLayer>();
	if (dhcpLayer != nullptr) {
		pcpp::DhcpMessageType dhcpMessageType = dhcpLayer->getMesageType();
		protocolInfo.dhcp_message_type = static_cast<int>(dhcpMessageType);
	}

	// 解析 SIP 请求方法和响应状态码
	pcpp::SipLayer* sipLayer = parsedPacket.getLayerOfType<pcpp::SipLayer>();
	if (sipLayer != nullptr) {
		std::string sipData(reinterpret_cast<const char*>(sipLayer->getData()), sipLayer->getDataLen());
		std::regex sipRegex("^(INVITE|ACK|BYE|CANCEL|OPTIONS|REGISTER|200 OK|404 Not Found).*");
		std::smatch sipMatches;
		if (std::regex_search(sipData, sipMatches, sipRegex)) {
			protocolInfo.sip_data = sipMatches[1];
		}
	}
	data.protocolInfoVector->push_back(protocolInfo);
}

void Flow::terminate()
{
	double duration = (double(latest_timestamp) - start_timestamp) / 1e9;//秒为单位
	flowFeature.dur = duration;
	// 计算吞吐率
	if (latest_timestamp == start_timestamp) {
		appbandwith = 0;
	}else{
		appbandwith = (double)packets_size_sum / duration * 8 / 1e3;//单位是kbps		
	}

	if (cnt_len_over_1000 >= 1)
		flowFeature.ave_pkt_size_over_1000 = (double)ave_pkt_size_over_1000 / cnt_len_over_1000;
	else flowFeature.ave_pkt_size_over_1000 = 0;
	// 计算 len < 300的平均包长
	if (cnt_len_under_300 >= 1)
		flowFeature.ave_pkt_size_under_300 = (double)ave_pkt_size_under_300 / cnt_len_under_300;
	else flowFeature.ave_pkt_size_under_300 = 0;
	
	// 计算payload

	flowFeature.payload_bandwidth = payload_bandwidth;
	// 计算RTT
	if(!rtts.empty()){
		double sum = std::accumulate(rtts.begin(), rtts.end(), 0);
		
		flowFeature.ave_rtt = static_cast<double>(sum) / rtts.size();
		auto min_max = std::minmax_element(rtts.begin(), rtts.end());
		flowFeature.rtt_min = *min_max.first;
		flowFeature.rtt_max = *min_max.second;
		flowFeature.rtt_range = flowFeature.rtt_max - flowFeature.rtt_min;
		flowFeature.pktlen = packets_size_sum;
	}else{
		flowFeature.ave_rtt = 0;
		flowFeature.rtt_min = 0;
		flowFeature.rtt_max = 0;
		flowFeature.rtt_range = 0;
	}
	
	if (pkt_count >= 1){
		flowFeature.ret_rate = double(ret) / pkt_count;
		flowFeature.udp_nopayload_rate = double(udp_nopayload_cnt) / pkt_count;
		ave_pkt_size = double(packets_size_sum) / pkt_count;
		flowFeature.bytes_of_payload = payload_size;
	}
	else{
		flowFeature.ret_rate = 0;
		flowFeature.udp_nopayload_rate = 0;
		ave_pkt_size = 0;
		flowFeature.bytes_of_payload = 0;
	}	
	if (pkt_count > 1){
		ave_interval = duration / double(pkt_count-1);
	}
	else{
		ave_interval = 0;
	}	
	flowFeature.itvl = ave_interval;

	flowFeature.bw = appbandwith;
	flowFeature.pktlen = ave_pkt_size;
	flowFeature.thp = throughput;
	flowFeature.cnt_len_over_1000 = cnt_len_over_1000;
	flowFeature.pktcnt = pkt_count;

	flowFeature.bytes_of_flow = packets_size_sum;
	flowFeature.header_of_packets = packets_size_sum - payload_size;
	flowFeature.bytes_of_ret_packets = ret_bytes;
	flowFeature.count_of_TCPpackets = tcp_packets;
	flowFeature.count_of_UDPpackets = udp_packets;
	flowFeature.count_of_ICMPpackets = icmp_packets;

	flowFeature.std_of_packet_size = calculateStandardDeviation(packets);
	flowFeature.end_to_end_latency = flowFeature.dur / pkt_count;
	flowFeature.avg_window_size = static_cast<double>(window_size_ls) / pkt_count;
	flowFeature.avg_ttl = static_cast<double>(ttl) / pkt_count;
	flowFeature.avg_payload_size = static_cast<double>(payload_size) / pkt_count;

	flowFeature.count_of_ret_packets = ret;
	flowFeature.entropy_of_payload = calculateEntropy(frequencyMap);

	flowFeature.videoMetrics.bitrate = flowFeature.bytes_of_flow / flowFeature.dur;

}



std::string FlowKey::toString()const
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

/**
 * Return a packet source and dest IP addresses
 */
std::pair<IPAddress, IPAddress> getIPs(const Packet* packet)
{
	IPAddress srcIP, dstIP;
	if (packet->isPacketOfType(IP))
	{
		const IPLayer* ipLayer = packet->getLayerOfType<IPLayer>();
		srcIP = ipLayer->getSrcIPAddress();
		dstIP = ipLayer->getDstIPAddress();
	}
	return std::pair<IPAddress, IPAddress>(srcIP, dstIP);
}


/**
 * Return a packet source and dest TCP ports
 */
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
 * Return a packet source and dest IP & ports
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

std::string nanosecondsToDatetime(long long nanoseconds) {
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


std::vector<double> getFlowsValues(const HandlePacketData* data, const char fieldName[]) {
    std::vector<double> values;
    if (strcmp(fieldName, "dur")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.dur);
        }
    } else if (strcmp(fieldName, "pktcnt")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.pktcnt);
        }
    } else if (strcmp(fieldName, "bytes_of_flow")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.bytes_of_flow);
        }
    } else if (strcmp(fieldName, "avg_bytes_of_flow")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.bytes_of_flow / flowPair.second->flowFeature.pktcnt);
        }
    } else if (strcmp(fieldName, "avg_ttl")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.avg_ttl);
        }
    } else if (strcmp(fieldName, "avg_window_size")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.avg_window_size);
        }
    } else if (strcmp(fieldName, "end_to_end_latency")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.end_to_end_latency);
        }
    } else if (strcmp(fieldName, "entropy_of_payload")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.entropy_of_payload);
        }
    } else if (strcmp(fieldName, "flow_peak_traffic")) {
        for (const auto& flowPair : *data->flows) {
            values.push_back(flowPair.second->flowFeature.peak_traffic);
        }
    }
    return values;
}

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

std::pair<std::map<std::string, int>, std::map<std::string, int>> countFlowsByIP(const HandlePacketData* data) {
    std::map<std::string, int> srcIPCounts, dstIPCounts;
    for (const auto& pair : *data->flows) {
        const Flow* flow = pair.second;
        std::string srcIP = flow->flowFeature.srcIP.toString(); 
        std::string dstIP = flow->flowFeature.dstIP.toString(); 
        srcIPCounts[srcIP]++;
        dstIPCounts[dstIP]++;
    }
    return {srcIPCounts, dstIPCounts};
}

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
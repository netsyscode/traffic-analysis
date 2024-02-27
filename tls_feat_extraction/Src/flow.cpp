#include "handledata.h"
#include <netinet/in.h>
#include <algorithm>
#include <regex>

extern HandlePacketData data;
extern long long timestamp_of_first_packet;
extern long long timestamp_of_last_packet;
extern uint64_t packet_cnt_of_pcap;
extern uint64_t bytes_cnt_of_pcap;
extern PacketsFeature packetsFeature;

uint32_t sps_parser_offset;
uint8_t sps_parser_base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
size_t sps_parser_base64_decode(char *buffer) {
	uint8_t dtable[256], block[4], tmp, pad = 0;
	size_t i, count = 0, pos = 0, len = strlen(buffer);

	memset(dtable, 0x80, 256);
	for (i = 0; i < sizeof(sps_parser_base64_table) - 1; i++) {
		dtable[sps_parser_base64_table[i]] = (unsigned char) i;
	}
	dtable['='] = 0;

	for (i = 0; i < len; i++) {
		if (dtable[static_cast<unsigned char>(buffer[i])] != 0x80) {
			count++;
		}
	}
	if (count == 0 || count % 4) return 0;

	count = 0;
	for (i = 0; i < len; i++) {
		tmp = dtable[static_cast<unsigned char>(buffer[i])];
		if (tmp == 0x80) continue;

		if (buffer[i] == '=') pad++;
		block[count] = tmp;
		count++;
		if (count == 4) {
			buffer[pos++] = (block[0] << 2) | (block[1] >> 4);
			buffer[pos++] = (block[1] << 4) | (block[2] >> 2);
			buffer[pos++] = (block[2] << 6) | block[3];

			count = 0;
			if (pad) {
				if (pad == 1) pos--;
				else if (pad == 2) pos -= 2;
				else return 0;
				break;
			}
		}
	}
	return pos;
}

uint32_t sps_parser_read_bits(char *buffer, uint32_t count) {
	uint32_t result = 0;
	uint8_t index = (sps_parser_offset / 8);
	uint8_t bitNumber = (sps_parser_offset - (index * 8));
	uint8_t outBitNumber = count - 1;
	for (uint8_t c = 0; c < count; c++) {
		if (buffer[index] << bitNumber & 0x80) {
			result |= (1 << outBitNumber);
		}
		if (++bitNumber > 7) { bitNumber = 0; index++; }
		outBitNumber--;
	}
	sps_parser_offset += count;
	return result;
}

uint32_t sps_parser_read_ueg(char* buffer) {
    uint32_t bitcount = 0;

    for (;;) {
    	if (sps_parser_read_bits(buffer, 1) == 0) {
	        bitcount++;
    	} else {
    		// bitOffset--;
    		break;
    	}
    }

    	// bitOffset --;
    uint32_t result = 0;
    if (bitcount) {
    	uint32_t val = sps_parser_read_bits(buffer, bitcount);
        result = (uint32_t) ((1 << bitcount) - 1 + val);
    }

    return result;
}

int32_t sps_parser_read_eg(char* buffer) {
	uint32_t value = sps_parser_read_ueg(buffer);
	if (value & 0x01) {
		return (value + 1) / 2;
	} else {
		return -(value / 2);
	}
}

void sps_parser_skipScalingList(char* buffer, uint8_t count) {
	uint32_t deltaScale, lastScale = 8, nextScale = 8;
	for (uint8_t j = 0; j < count; j++) {
		if (nextScale != 0) {
			deltaScale = sps_parser_read_eg(buffer);
			nextScale = (lastScale + deltaScale + 256) % 256;
		}
		lastScale = (nextScale == 0 ? lastScale : nextScale);
	}
}

uint32_t sps_parser(char *buffer) {

	uint8_t profileIdc = 0;
	uint32_t pict_order_cnt_type = 0;
	uint32_t picWidthInMbsMinus1 = 0;
	uint32_t picHeightInMapUnitsMinus1 = 0;
	uint8_t frameMbsOnlyFlag = 0;
	uint32_t frameCropLeftOffset = 0;
	uint32_t frameCropRightOffset = 0;
	uint32_t frameCropTopOffset = 0;
	uint32_t frameCropBottomOffset = 0;


	sps_parser_offset = 0;
	sps_parser_base64_decode(buffer);
	sps_parser_read_bits(buffer, 8);
	profileIdc = sps_parser_read_bits(buffer, 8);
	sps_parser_read_bits(buffer, 16);
	sps_parser_read_ueg(buffer);

	if (profileIdc == 100 || profileIdc == 110 || profileIdc == 122 ||
		profileIdc == 244 || profileIdc == 44 || profileIdc == 83 ||
		profileIdc == 86 || profileIdc == 118 || profileIdc == 128) {
		uint32_t chromaFormatIdc = sps_parser_read_ueg(buffer);
		if (chromaFormatIdc == 3) sps_parser_read_bits(buffer, 1);
		sps_parser_read_ueg(buffer);
		sps_parser_read_ueg(buffer);
		sps_parser_read_bits(buffer, 1);
		if (sps_parser_read_bits(buffer, 1)) {
			for (uint8_t i = 0; i < (chromaFormatIdc != 3) ? 8 : 12; i++) {
				if (sps_parser_read_bits(buffer, 1)) {
					if (i < 6) {
						sps_parser_skipScalingList(buffer, 16);
					} else {
						sps_parser_skipScalingList(buffer, 64);
					}
				}
			}
		}
	}

	sps_parser_read_ueg(buffer);
	pict_order_cnt_type = sps_parser_read_ueg(buffer);

	if (pict_order_cnt_type == 0) {
		sps_parser_read_ueg(buffer);
	} else if (pict_order_cnt_type == 1) {
		sps_parser_read_bits(buffer, 1);
		sps_parser_read_eg(buffer);
		sps_parser_read_eg(buffer);
		for (uint32_t i = 0; i < sps_parser_read_ueg(buffer); i++) {
			sps_parser_read_eg(buffer);
		}
	}

	sps_parser_read_ueg(buffer);
	sps_parser_read_bits(buffer, 1);
	picWidthInMbsMinus1 = sps_parser_read_ueg(buffer);
	picHeightInMapUnitsMinus1 = sps_parser_read_ueg(buffer);
	frameMbsOnlyFlag = sps_parser_read_bits(buffer, 1);
	if (!frameMbsOnlyFlag) sps_parser_read_bits(buffer, 1);
	sps_parser_read_bits(buffer, 1);
	if (sps_parser_read_bits(buffer, 1)) {
		frameCropLeftOffset = sps_parser_read_ueg(buffer);
		frameCropRightOffset = sps_parser_read_ueg(buffer);
		frameCropTopOffset = sps_parser_read_ueg(buffer);
		frameCropBottomOffset = sps_parser_read_ueg(buffer);
	}
	return (
		(((picWidthInMbsMinus1 + 1) * 16) - frameCropLeftOffset * 2 - frameCropRightOffset * 2) << 16 |
		(((2 - frameMbsOnlyFlag) * (picHeightInMapUnitsMinus1 + 1) * 16) - ((frameMbsOnlyFlag ? 2 : 4) * (frameCropTopOffset + frameCropBottomOffset)))
	);
}

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

// 收到一个包，更新相关的特征
void Flow::addPacket(RawPacket* pkt)
{
	packets.push_back(pkt);
	updateFeature(pkt);
}

// 计算熵
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

//更新相关特征
void Flow::updateFeature(RawPacket* pkt) {
	int pktLen = pkt->getFrameLength();
	packetInfo.packet_length = pktLen;
	packets_size.push_back(pktLen);
	//统计包长大于1000的包数量
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
	packetsFeature.max_packet_size = std::max(packetsFeature.max_packet_size, pktLen);
	packetsFeature.min_packet_size = std::min(packetsFeature.min_packet_size, pktLen);

	//更新最新包的时间戳(以ns为单位)
	long sec = pkt->getPacketTimeStamp().tv_sec;
	long nsec = pkt->getPacketTimeStamp().tv_nsec;
	latest_timestamp = sec * 1000000000LL + nsec;
	timestamp_of_last_packet = std::max(timestamp_of_last_packet, latest_timestamp);
	
	packetInfo.arrival_timestamp = nanosecondsToDatetime(latest_timestamp);

	// 计算相邻两个包间时延
	if(latest_timestamp != latter_timestamp){
		auto diff = (latest_timestamp - latter_timestamp) / 1e9;
		interval_vec.push_back(diff);
		packetsFeature.max_interval_between_packets = std::max(packetsFeature.max_interval_between_packets, diff);
		packetsFeature.min_interval_between_packets = std::min(packetsFeature.min_interval_between_packets, diff);
	}

	pcpp::Packet parsedPacket(pkt);
	pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();

	// 如果是TCP包
    if (tcpLayer != nullptr) {
		protocolInfo.tcp_header_length = tcpLayer->getHeaderLen();
		pcpp::tcphdr* tcpHeader = tcpLayer->getTcpHeader();
		protocolInfo.tcp_window_size = tcpHeader->windowSize;
		window_size_ls += protocolInfo.tcp_window_size;
		
		tcp_packets ++;
        size_t tcpPayloadLength = tcpLayer->getLayerPayloadSize();
		packetInfo.payload_size = tcpPayloadLength;
		
		// 如果有TCP负载则计算负载的熵
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
		
		// 统计TCP协议头的标志位
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
		// RTT近似计算
        u_int32_t seqNumber = ntohl(tcpHeader->sequenceNumber);//包的seq
		protocolInfo.tcp_sequence_number = seqNumber;
		u_int32_t acknoNumber = ntohl(tcpHeader->ackNumber);//包的ackno
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

		//只有syn包会执行
        if (tcpHeader->synFlag && !tcpHeader->ackFlag) {
			long sec = pkt->getPacketTimeStamp().tv_sec;
			long nsec = pkt->getPacketTimeStamp().tv_nsec;
			seqToTime[seqNumber] = sec * 1000000000LL + nsec;
        }

		if (tcpHeader->ackFlag) {
			// 判断这个包是否是建立tcp连接第三步握手的ack包
            if (seqToTime.count(seqNumber - 1) > 0) {
					uint64_t synTime = seqToTime[seqNumber - 1];
					uint64_t synAckTime = latest_timestamp;
					rtts.push_back(double(synAckTime - synTime) / 1e9);

					// 认为a->b的RTT与b->a的RTT相同
					auto key = this->flowKey;
					FlowKey keyPal = {key.dstIP, key.srcIP, key.dstPort, key.srcPort};
					std::map<FlowKey, Flow*>::iterator flowIter = data.flows->find(keyPal);
					if (flowIter != data.flows->end())
						flowIter->second->rtts.push_back(double(synAckTime - synTime) / 1e9);
					//seqToTime.erase(seqNumber - 1);
            }
			
			// 统计第一步握手和第二步握手的时间差
			if (tcpLayer->getTcpHeader()->synFlag && tcpLayer->getTcpHeader()->ackFlag) {
				if (seqToTime.count(seqNumber - 1) > 0) {
					auto key = this->flowKey;
					FlowKey keyPal = {key.dstIP, key.srcIP, key.dstPort, key.srcPort};
					std::map<FlowKey, Flow*>::iterator flowIter = data.flows->find(keyPal);
					if (flowIter != data.flows->end()){
						uint64_t synTime = flowIter->second->start_timestamp;
						//synTime = seqToTime[seqNumber - 1];
						long sec = pkt->getPacketTimeStamp().tv_sec;
						long nsec = pkt->getPacketTimeStamp().tv_nsec;
						uint64_t synAckTime = sec * 1000000000LL + nsec;
						packetsFeature.syn_ack_time = double(synAckTime - synTime) / 1e9;//算出来的应该是发起会话的RRT
					}
				}
			}
		}

		// 统计与HTTP请求相关的特征
		pcpp::HttpRequestLayer* httpRequestLayer = parsedPacket.getLayerOfType<pcpp::HttpRequestLayer>();
        if (httpRequestLayer != nullptr) {
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

		// 统计与HTTP相应相关的特征
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
		// 解析UDP协议
		pcpp::UdpLayer* udpLayer = parsedPacket.getLayerOfType<pcpp::UdpLayer>();
		if (udpLayer != nullptr) {
			udp_packets ++;
			auto udpHeadr = udpLayer->getUdpHeader();
			protocolInfo.udp_header_length = udpHeadr->length;
			protocolInfo.udp_checksum = udpHeadr->headerChecksum;
			size_t udpPayloadLength = udpLayer->getLayerPayloadSize();
			packetInfo.payload_size = udpPayloadLength;

			// 如果UDP有负载
			if(udpPayloadLength > 0){
				const uint8_t* payload = udpLayer->getLayerPayload();

				// 计算负载熵
				for (size_t i = 0; i < udpPayloadLength; ++i) {
					frequencyMap[payload[i]]++;
				}

				//解析SPS
				uint8_t* rtp_payload = udpLayer->getLayerPayload();
				size_t* sps_length = 0;
				uint8_t* sps_data = extract_sps_from_rtp(rtp_payload, udpPayloadLength, sps_length);
				if (sps_data != NULL && sps_length != 0){
					char* sps_buffer = (char*)sps_data;
					uint32_t dimensions = sps_parser(sps_buffer);
					
					int width = dimensions >> 16;
					int height = dimensions & 0xFFFF;
					std::cout << "width = " << width << "\nheight = " << height << std::endl;
				}
			}
			payload_size += udpPayloadLength;
			if(udpPayloadLength == 0){
				udp_nopayload_cnt ++;
			}
		}

		// 解析ICMP协议
		pcpp::IcmpLayer* icmpLayer = parsedPacket.getLayerOfType<pcpp::IcmpLayer>();
		if (icmpLayer != nullptr) {
			icmp_packets ++;
			protocolInfo.icmp_code = icmpLayer->getIcmpHeader()->code;
			protocolInfo.icmp_type = icmpLayer->getMessageType();

		}

		// 解析IPv4协议
		pcpp::IPv4Layer* ipv4Layer = parsedPacket.getLayerOfType<pcpp::IPv4Layer>();
		if (ipv4Layer != nullptr) {
			ttl += ipv4Layer->getIPv4Header()->timeToLive;
			uint8_t ipTos = ipv4Layer->getIPv4Header()->typeOfService;
			uint16_t ipChecksum = ipv4Layer->getIPv4Header()->headerChecksum;
			uint8_t ipFragmentationFlag = ipv4Layer->getFragmentFlags();
			uint16_t ipID = ipv4Layer->getIPv4Header()->ipId;
			protocolInfo.ip_tos = ipTos;
			protocolInfo.ip_header_checksum = ipChecksum;
			protocolInfo.ip_fragmentation_flag = ipFragmentationFlag;
			protocolInfo.ip_identifier = ipID;
		}

		// 解析IPv6协议
		pcpp::IPv6Layer* ipv6Layer = parsedPacket.getLayerOfType<pcpp::IPv6Layer>();
		if (ipv6Layer != nullptr) {
			uint8_t* ipv6FlowLabel = ipv6Layer->getIPv6Header()->flowLabel;
			uint8_t ipv6NextHeader = ipv6Layer->getIPv6Header()->nextHeader;
			protocolInfo.ipv6_flow_label = ipv6FlowLabel;
			protocolInfo.ipv6_next_header = ipv6NextHeader;
		}

		// 解析以太网协议
		pcpp::EthLayer* ethernetLayer = parsedPacket.getLayerOfType<pcpp::EthLayer>();
		if (ethernetLayer != nullptr) {
			std::string macSrc = ethernetLayer->getSourceMac().toString();
			std::string macDst = ethernetLayer->getDestMac().toString();
			uint16_t ethType = ethernetLayer->getEthHeader()->etherType;
			protocolInfo.mac_source =macSrc;
			protocolInfo.mac_destination = macDst;
			protocolInfo.ethernet_type = ethType;
		}

		// 解析VLAN协议
		pcpp::VlanLayer* vlanLayer = parsedPacket.getLayerOfType<pcpp::VlanLayer>();
		if (vlanLayer != nullptr) {
			uint16_t vlanID = vlanLayer->getVlanID();
			protocolInfo.vlan_id = vlanID;
		}

		// 解析MPLS协议
		pcpp::MplsLayer* mplsLayer = parsedPacket.getLayerOfType<pcpp::MplsLayer>();
		if (mplsLayer != nullptr) {
			uint32_t mplsLabel = mplsLayer->getMplsLabel();
			protocolInfo.mpls_label = mplsLabel;
		}

		// 解析PPPoE协议
		pcpp::PPPoELayer* pppoeLayer = parsedPacket.getLayerOfType<pcpp::PPPoELayer>();
		if (pppoeLayer != nullptr) {
			uint16_t pppoeSessionId = pppoeLayer->getPPPoEHeader()->sessionId;
			protocolInfo.pppoe_session_id = pppoeSessionId;
		}

		// 解析ARP协议
		pcpp::ArpLayer* arpLayer = parsedPacket.getLayerOfType<pcpp::ArpLayer>();
		if (arpLayer != nullptr) {
			protocolInfo.arp_request = (arpLayer->getArpHeader()->opcode == pcpp::ARP_REQUEST);
			protocolInfo.arp_reply = (arpLayer->getArpHeader()->opcode == pcpp::ARP_REPLY);
		}

		// 解析DNS协议
		pcpp::DnsLayer* dnsLayer = parsedPacket.getLayerOfType<pcpp::DnsLayer>();
		if (dnsLayer != nullptr) {
			// Iterate over DNS queries if exist
			if (dnsLayer->getFirstQuery() != nullptr) {
				std::vector<pcpp::DnsType> DnsQueryType;
				for (pcpp::DnsQuery* query = dnsLayer->getFirstQuery(); query != nullptr; query = dnsLayer->getNextQuery(query)) {
					pcpp::DnsType dnsQueryType = query->getDnsType(); 
					DnsQueryType.push_back(dnsQueryType);
				}
				protocolInfo.dns_query_type = DnsQueryType;
			}
		}
		// 解析 SMTP 和 FTP 命令/响应
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
			auto messageType = dhcpLayer->getMesageType();
			protocolInfo.dhcp_message_type = messageType;

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

// 流结束时统计其他的流特征
void Flow::terminate()
{
	// 包长的中位数、标准差、偏度和峰度
	packetsFeature.median_packet_size = calculateMedian(packets_size);
	packetsFeature.std_packet_size = calculateStandardVariance(packets_size);
	packetsFeature.packet_size_skewness = calculateSkewness(packets_size);
	packetsFeature.packet_size_kurtosis = calculateKurtosis(packets_size);

	// 包间时延的中位数和标准差
	packetsFeature.median_packet_interval = calculateMedian(interval_vec);
	packetsFeature.std_packet_interval = calculateStandardVariance(interval_vec);

	double duration = (double(latest_timestamp) - start_timestamp) / 1e9;//秒为单位
	flowFeature.dur = duration;

	// 吞吐率
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
	packet_cnt_of_pcap += pkt_count;
	bytes_cnt_of_pcap += packets_size_sum;

	flowFeature.bytes_of_flow = packets_size_sum;
	packets_size_sum = 0;//统计的是一条流的字节数

	flowFeature.header_of_packets = packets_size_sum - payload_size;
	flowFeature.bytes_of_ret_packets = ret_bytes;
	flowFeature.count_of_TCPpackets = tcp_packets;
	flowFeature.count_of_UDPpackets = udp_packets;
	flowFeature.count_of_ICMPpackets = icmp_packets;

	flowFeature.end_to_end_latency = flowFeature.dur / pkt_count;
	flowFeature.avg_window_size = static_cast<double>(window_size_ls) / pkt_count;
	flowFeature.avg_ttl = static_cast<double>(ttl) / pkt_count;
	flowFeature.avg_payload_size = static_cast<double>(payload_size) / pkt_count;
	flowFeature.count_of_ret_packets = ret;
	flowFeature.entropy_of_payload = calculateEntropy(frequencyMap);
	flowFeature.videoMetrics.bitrate = flowFeature.bytes_of_flow / flowFeature.dur;

}

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

// 返回包的源IP和目的IP
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


// 返回包的源端口和目的端口
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

// 生成flowKey
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

// 填充会话key
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

// 将时间戳转换为形如2000-00-00:000000的时间
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

// 计算分位数近似拟合数据分布
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

// 根据fielName提取data中的数据
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

// 统计一条流的源端口和目的端口数
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

// 统计一条流的源IP和目的IP数
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

// 判断是否是公网IP
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

// 计算偏度
double calculateSkewness(const std::vector<double>& packets_size) {
    if (packets_size.size() < 3) {
        // 数据量太少，无法计算偏度
        return std::nan("");
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
    skewness *= packets_size.size() / ((packets_size.size() - 1) * (packets_size.size() - 2));
    return skewness;
}

// 计算峰度
double calculateKurtosis(const std::vector<double>& packets_size) {
    size_t n = packets_size.size();
    if (n < 4) {
        // 数据量太少，无法计算峰度
        return std::nan("");
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
    return kurtosis;
}

// 提取SPS信息
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
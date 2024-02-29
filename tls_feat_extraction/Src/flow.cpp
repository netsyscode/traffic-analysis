#include <netinet/in.h>
#include <algorithm>
#include <regex>
#include "global.h"

LL timestamp_of_first_packet = std::numeric_limits<LL>::max();
LL timestamp_of_last_packet = std::numeric_limits<LL>::min();
std::vector<double> psh_interval_vec;
std::vector<double> urg_interval_vec;
uint64_t packet_cnt_of_pcap;
uint64_t bytes_cnt_of_pcap;
bool download_flag = false;
PacketsFeature packetsFeature;
DownloadMetrics downloadMetrics;
VideoStreamMetrics videoMetrics;

uint32_t sps_parser_offset;
uint8_t sps_parser_base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
// Base64解码函数
size_t sps_parser_base64_decode(char *buffer) {
	uint8_t dtable[256], block[4], tmp, pad = 0;
	size_t i, count = 0, pos = 0, len = strlen(buffer);

	// 初始化解码表
	memset(dtable, 0x80, 256);
	for (i = 0; i < sizeof(sps_parser_base64_table) - 1; i++) {
		dtable[sps_parser_base64_table[i]] = (unsigned char) i;
	}
	dtable['='] = 0;

	// 计算有效的编码字符数量
	for (i = 0; i < len; i++) {
		if (dtable[static_cast<unsigned char>(buffer[i])] != 0x80) {
			count++;
		}
	}
	if (count == 0 || count % 4) return 0;// 数据不完整或格式错误

	count = 0;
	for (i = 0; i < len; i++) {
		tmp = dtable[static_cast<unsigned char>(buffer[i])];
		if (tmp == 0x80) continue;// 跳过非Base64字符

		if (buffer[i] == '=') pad++;
		block[count] = tmp;
		count++;
		if (count == 4) {// 每4个字符解码为3个字节
			buffer[pos++] = (block[0] << 2) | (block[1] >> 4);
			buffer[pos++] = (block[1] << 4) | (block[2] >> 2);
			buffer[pos++] = (block[2] << 6) | block[3];

			count = 0;
			if (pad) {// 根据填充字符数量调整解码后的数据长度
				if (pad == 1) pos--;
				else if (pad == 2) pos -= 2;
				else return 0;// 错误的填充
				break;
			}
		}
	}
	return pos;// 返回解码后的数据长度
}

// 从buffer中读取指定位数的bits，并更新offset
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

// 读取指数哥伦布编码（无符号）
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

// 读取指数哥伦布编码（有符号）
int32_t sps_parser_read_eg(char* buffer) {
	uint32_t value = sps_parser_read_ueg(buffer);
	if (value & 0x01) {
		return (value + 1) / 2;
	} else {
		return -(value / 2);
	}
}

// 跳过缩放列表的函数
void sps_parser_skipScalingList(char* buffer, uint8_t count) {
	uint32_t deltaScale, lastScale = 8, nextScale = 8;
	for (uint8_t j = 0; j < count; j++) {
		if (nextScale != 0) {
			deltaScale = sps_parser_read_eg(buffer);// 读取指数哥伦布编码
			nextScale = (lastScale + deltaScale + 256) % 256;
		}
		lastScale = (nextScale == 0 ? lastScale : nextScale);
	}
}

// SPS解析函数
// H.264是一种视频编码格式，而不是网络传输协议
uint32_t sps_parser(char *buffer) {

	uint8_t profileIdc = 0;// Profile标识
	uint32_t pict_order_cnt_type = 0;// 图片顺序计数类型
	uint32_t picWidthInMbsMinus1 = 0;// 宽度（宏块数减一）
	uint32_t picHeightInMapUnitsMinus1 = 0;// 高度（映射单位数减一）
	uint8_t frameMbsOnlyFlag = 0;// 仅帧宏块标志
	uint32_t frameCropLeftOffset = 0;// 左裁剪偏移
	uint32_t frameCropRightOffset = 0;// 右裁剪偏移
	uint32_t frameCropTopOffset = 0;// 上裁剪偏移
	uint32_t frameCropBottomOffset = 0;// 下裁剪偏移


	sps_parser_offset = 0;// 解析偏移量初始化
	sps_parser_base64_decode(buffer);// 对buffer进行Base64解码
	sps_parser_read_bits(buffer, 8);// 读取8位
	profileIdc = sps_parser_read_bits(buffer, 8);// 读取profile标识
	sps_parser_read_bits(buffer, 16);// 跳过一些字段
	sps_parser_read_ueg(buffer);// 读取无符号指数哥伦布编码

	// 根据profileIdc的值，决定是否需要解析后续的字段
	if (profileIdc == 100 || profileIdc == 110 || profileIdc == 122 ||
		profileIdc == 244 || profileIdc == 44 || profileIdc == 83 ||
		profileIdc == 86 || profileIdc == 118 || profileIdc == 128) {
		uint32_t chromaFormatIdc = sps_parser_read_ueg(buffer);// 色度格式
		if (chromaFormatIdc == 3) sps_parser_read_bits(buffer, 1);// 读取额外的位
		sps_parser_read_ueg(buffer);// 读取和跳过一些配置
		sps_parser_read_ueg(buffer);
		sps_parser_read_bits(buffer, 1);
		if (sps_parser_read_bits(buffer, 1)) {// 判断是否存在缩放列表
			for (uint8_t i = 0; i < (chromaFormatIdc != 3) ? 8 : 12; i++) {
				if (sps_parser_read_bits(buffer, 1)) {// 对每个缩放列表进行处理
					if (i < 6) {
						sps_parser_skipScalingList(buffer, 16);
					} else {
						sps_parser_skipScalingList(buffer, 64);
					}
				}
			}
		}
	}

	sps_parser_read_ueg(buffer);// 跳过一些配置
	pict_order_cnt_type = sps_parser_read_ueg(buffer);// 图片顺序计数类型

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

	sps_parser_read_ueg(buffer);// 跳过一些配置
	sps_parser_read_bits(buffer, 1);// 读取一些标志位
	picWidthInMbsMinus1 = sps_parser_read_ueg(buffer);// 宽度（宏块数减一）
	picHeightInMapUnitsMinus1 = sps_parser_read_ueg(buffer);// 高度（映射单位数减一）
	frameMbsOnlyFlag = sps_parser_read_bits(buffer, 1);// 仅帧宏块标志
	if (!frameMbsOnlyFlag) sps_parser_read_bits(buffer, 1);// 如果不是仅帧宏块，读取额外的位
	sps_parser_read_bits(buffer, 1);// 读取一些标志位
	if (sps_parser_read_bits(buffer, 1)) {// 判断是否存在帧裁剪
		frameCropLeftOffset = sps_parser_read_ueg(buffer);// 左裁剪偏移
		frameCropRightOffset = sps_parser_read_ueg(buffer);// 右裁剪偏移
		frameCropTopOffset = sps_parser_read_ueg(buffer);// 上裁剪偏移
		frameCropBottomOffset = sps_parser_read_ueg(buffer);// 下裁剪偏移
	}
	// 计算并返回最终的视频分辨率
	return (
		(((picWidthInMbsMinus1 + 1) * 16) - frameCropLeftOffset * 2 - frameCropRightOffset * 2) << 16 |
		(((2 - frameMbsOnlyFlag) * (picHeightInMapUnitsMinus1 + 1) * 16) - ((frameMbsOnlyFlag ? 2 : 4) * (frameCropTopOffset + frameCropBottomOffset)))
	);
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

	SinglePacketInfo singlePacketInfo;
	singlePacketInfo.packet_length = pktLen;
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
	
	singlePacketInfo.arrival_timestamp = nanosecondsToDatetime(latest_timestamp);

	// 计算相邻两个包间时延
	if(latest_timestamp != latter_timestamp){
		auto diff = (latest_timestamp - latter_timestamp) / 1e9;
		interval_vec.push_back(diff);
		packetsFeature.max_interval_between_packets = std::max(packetsFeature.max_interval_between_packets, diff);
		packetsFeature.min_interval_between_packets = std::min(packetsFeature.min_interval_between_packets, diff);
	}

	ProtocolInfo protocolInfo;
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
		singlePacketInfo.payload_size = tcpPayloadLength;
		
		// 如果有TCP负载则计算负载的熵
		if(tcpPayloadLength > 0){
            const uint8_t* payload = tcpLayer->getLayerPayload();
			std::map<uint8_t, int> frequencyPacketMap;
            for (size_t i = 0; i < tcpPayloadLength; ++i) {
                frequencyMap[payload[i]]++;
				frequencyPacketMap[payload[i]]++;
            }
			singlePacketInfo.payload_entropy = calculateEntropy(frequencyPacketMap);
		}
        payload_size += tcpPayloadLength;
		
		// 统计TCP协议头的标志位
		protocolInfo.syn_flag = (tcpHeader->synFlag == 1);
		protocolInfo.fin_flag = (tcpHeader->finFlag == 1);
		protocolInfo.rst_flag = (tcpHeader->rstFlag == 1);
		protocolInfo.psh_flag = (tcpHeader->pshFlag == 1);
		if(tcpHeader->pshFlag){
			if(!first_psh){
				psh_interval_vec.push_back((latest_timestamp - last_psh) / 1e9);
			}
			last_psh = latest_timestamp;
		}

		protocolInfo.urg_flag = (tcpHeader->urgFlag == 1);
		if(tcpHeader->urgFlag){	
			if(!first_urg){
				urg_interval_vec.push_back((latest_timestamp - last_urg) / 1e9);
			}
			last_urg = latest_timestamp;
		}

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

		// 第一步握手syn包会执行
        if (tcpHeader->synFlag && !tcpHeader->ackFlag) {
			long sec = pkt->getPacketTimeStamp().tv_sec;
			long nsec = pkt->getPacketTimeStamp().tv_nsec;
			synSeqToTime[seqNumber] = sec * 1000000000LL + nsec;
        }

		//fin包会执行
        if (tcpHeader->finFlag) {
			long sec = pkt->getPacketTimeStamp().tv_sec;
			long nsec = pkt->getPacketTimeStamp().tv_nsec;
			finSeqToTime[seqNumber] = sec * 1000000000LL + nsec;
        }

		// 收到了确认包
		if (tcpHeader->ackFlag) {
			// 判断这个包是否是建立tcp连接第三步握手的ack包
            if (synSeqToTime.count(seqNumber - 1) > 0) {
				uint64_t synTime = synSeqToTime[seqNumber - 1];
				uint64_t synAckTime = latest_timestamp;
				rtts.push_back(double(synAckTime - synTime) / 1e9);

				// 认为a->b的RTT与b->a的RTT相同
				auto key = this->flowKey;
				FlowKey keyPal = {key.dstIP, key.srcIP, key.dstPort, key.srcPort};
				std::map<FlowKey, Flow*>::iterator flowIter = data.flows->find(keyPal);
				if (flowIter != data.flows->end())
					flowIter->second->rtts.push_back(double(synAckTime - synTime) / 1e9);
			}

			// 如果是第二步挥手
			if(finSeqToTime.count(acknoNumber - 1) > 0) {
				uint64_t finTime = finSeqToTime[acknoNumber - 1];
				uint64_t finAckTime = latest_timestamp;
				fin_ack_vec.push_back((finAckTime - finTime) / 1e9);
			}
			
			// 统计第一步握手和第二步握手的时间差
			if (tcpHeader->synFlag) {
				if (synSeqToTime.count(acknoNumber - 1) > 0) {
					uint64_t synTime =  finSeqToTime[acknoNumber - 1];
					uint64_t synAckTime = latest_timestamp;
					syn_ack_vec.push_back((synAckTime - synTime) / 1e9);
				}
			}
		}

		// 统计与HTTP请求相关的特征
		pcpp::HttpRequestLayer* httpRequestLayer = parsedPacket.getLayerOfType<pcpp::HttpRequestLayer>();
        if (httpRequestLayer != nullptr) {

			// 检测是否是具有断点续传能力
			if(downloadMetrics.resume_downloading)
				if(checkForRangeHeader(pkt))
					downloadMetrics.resume_downloading = true;

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
			singlePacketInfo.payload_size = udpPayloadLength;

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
		data.singlePacketInfoVector->push_back(singlePacketInfo);
}



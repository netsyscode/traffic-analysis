#ifndef FLOW_H
#define FLOW_H

#pragma once
#define WIN32
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <set>
#include <numeric>
#include <pcapplusplus/Packet.h>
#include <pcapplusplus/IpAddress.h>
#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/IPv6Layer.h>
#include <pcapplusplus/TcpLayer.h>
#include <pcapplusplus/UdpLayer.h>
#include <pcapplusplus/HttpLayer.h>
#include <pcapplusplus/DnsLayer.h>
#include <pcapplusplus/IcmpLayer.h>
#include <pcapplusplus/EthLayer.h>
#include <pcapplusplus/PPPoELayer.h>
#include <pcapplusplus/VlanLayer.h>
#include <pcapplusplus/MplsLayer.h>
#include <pcapplusplus/DhcpLayer.h>
#include <pcapplusplus/SipLayer.h>
#include "feature.h"

#define EXPIRED_UPDATE 40
#define SEGMENT_WINDOWS_SIZE 3
//定义连续的时间范围内
#define CLUMP_TIMEOUT 1
//连续发送多个包的最小值
#define BULK_BOUND 4
#define DOWNLOAD_DATA_THRESHOLD 1024 * 1024
#define ACTIVE_TIMEOUT 0.00

enum FlowDirection
{
	forward, backward
};

struct FlowKey
{
	IPAddress srcIP, dstIP; 			/* 源/目的IP */
	uint16_t srcPort, dstPort;			/* 源/目的端口号 */

	std::string toString() const;
	bool operator< (const struct FlowKey& e) const;
	bool operator== (const struct FlowKey& e) const;
};

extern VideoStreamMetrics videoMetrics;
extern bool download_flag;
extern DownloadMetrics downloadMetrics;

class Flow
{
public:
	FlowKey flowKey;
	FlowFeature flowFeature;
	std::vector<RawPacket*> packets;//记录一条流的所有包
	//用于计算流特征
	LL latest_timestamp = 0, latter_timestamp = 0, start_timestamp = 0, window_size_ls = 0;
	int pkt_count = 0;
	uint16_t ttl = 0;
	LL packets_size_sum = 0, app_packets_size_sum = 0, header_len_ls = 0;
	double bandwith = 0.0, throughput = 0.0, duration = 0.0;
	double ave_pkt_size = 0.0, app_ave_pkt_size = 0.0,ave_interval = 0.0, ave_windows = 0.0, ave_rtt = 0.0, rtt_min = 0.0, rtt_max = 0.0, interval_ls = 0.0, rtt_ls = 0.0;
	int tcp_packets = 0, udp_packets = 0, icmp_packets = 0;
	
	Flow(FlowKey flowKey);
	void updateFeature(RawPacket* pkt);
	void addPacket(RawPacket* pkt);
	void terminate(bool flag);

	std::map<uint8_t, int> frequencyMap;
	int udp_nopayload_cnt = 0;
	int cnt_len_over_1000 = 0, ave_pkt_size_over_1000 = 0;
	int cnt_len_under_300 = 0,ave_pkt_size_under_300 = 0;
	LL payload_size = 0, ret_bytes = 0;
	double payload_bandwidth;
	std::map<uint32_t, uint64_t> synSeqToTime; //每条流都含有一个
	std::map<uint32_t, uint64_t> finSeqToTime; //每条流都含有一个
	uint64_t packetBuffer = 0;
	uint64_t ackBuffer = 0;
	std::vector<double> rtts;
	RawPacket* pkt;
	int ret = 0;
	std::vector<double> packets_size; // 每条流一个
	std::vector<double> interval_vec;
	std::vector<double> syn_ack_vec;
	std::vector<double> fin_ack_vec;
	bool first_psh = false;
	bool first_urg = false;
	LL last_psh = 0;
	LL last_urg = 0;
	double diff = 0.0;
	int max_packet_size = 0, min_packet_size = 1<<16;
	double max_interval_between_packets = 0.0, min_interval_between_packets = 0.0;
};

extern std::vector<double> psh_interval_vec;
extern std::vector<double> urg_interval_vec;
extern uint64_t packet_cnt_of_pcap;
extern uint64_t bytes_cnt_of_pcap;
extern LL timestamp_of_first_packet;
extern LL timestamp_of_last_packet;
extern std::vector<std::string> toString;

std::string nanosecondsToDatetime(LL nanoseconds);
double calculateAverage(std::vector<double>& vec);
double calculateMedian(std::vector<double>& vec);
double calculateStandardVariance(std::vector<double>& vec);
FlowKey* generateFlowKey(const Packet* packet);
void fillSessionKeyWithFlowKey(SessionKey& SessionKey, const FlowKey& flowKey,  bool fromClient);
double calculateSkewness(const std::vector<double>& packets_size);
double calculateKurtosis(const std::vector<double>& packets_size);
uint8_t* extract_sps_from_rtp(uint8_t* rtp_payload, size_t payload_length, size_t* sps_length);
bool checkForRangeHeader(const pcpp::Packet& packet);
bool isResumedDownload(const std::vector<pcpp::Packet>& packets);
double calculateEntropy(const std::map<uint8_t, int>& frequencyMap);
#endif //FLOW_H


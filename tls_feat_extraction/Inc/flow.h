#pragma once
#define WIN32
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
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
#include "tls_features.h"

typedef long long LL; 
#define EXPIRED_UPDATE 40
#define SEGMENT_WINDOWS_SIZE 3
//定义连续的时间范围内
#define CLUMP_TIMEOUT 1
//连续发送多个包的最小值
#define BULK_BOUND 4
#define DOWNLOAD_DATA_THRESHOLD 1024 * 1024

#define ACTIVE_TIMEOUT 0.005
using namespace pcpp;

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

//流特征
struct FlowFeature
{
	TLSFingerprint* tlsFingerprint = nullptr;
	IPAddress srcIP, dstIP;
	uint16_t srcPort, dstPort;
	double startts, endts;			// 开始时间，结束时间
	LL pktcnt;				// 包数
	double pktlen, applen;			// 包长，应用层长度
	double itvl, bw, appbw, thp, dur;	// 包间隔，带宽，应用层带宽，吞吐率
	// 只比较吞吐量
	bool operator<(const struct FlowFeature &other)const {
		return thp < other.thp;
	}
	float score;
	// 腾讯分类用的，这里其实用不太上
	double ave_pkt_size_over_1000;//包长大于1000的包 的平均包长
	double ave_pkt_size_under_300;//包长小于300的包 的平均包长
	double cnt_len_over_1000;
	double payload_size, payload_bandwidth; //payload长度，带宽
	double udp_nopayload_rate;//没有payload的udp包比例
	double ret_rate; //重传比例
	double ave_rtt, rtt_min, rtt_max, rtt_range; //rtt均值，极值, 极差
 
	//新增协议
	//流特征
	LL bytes_of_flow, bytes_of_payload, header_of_packets, bytes_of_ret_packets;
	int count_of_TCPpackets, count_of_UDPpackets, count_of_ICMPpackets;
	int max_size_of_packet, min_size_of_packet;
	double end_to_end_latency, avg_window_size, avg_ttl, avg_payload_size;
	int count_of_ret_packets, count_of_syn_packets, count_of_fin_packets, count_of_rst_packets, count_of_ack_packets, count_of_psh_packets, count_of_urg_packets;
	double entropy_of_payload;
	int count_of_forward_packets, count_of_backward_packets;

	FlowFeature() : 
        tlsFingerprint(nullptr),
        srcPort(0), dstPort(0),
        startts(0.0), endts(0.0),
        pktcnt(0),
        pktlen(0.0), applen(0.0),
        itvl(0.0), bw(0.0), appbw(0.0), thp(0.0),dur(0.0),
        score(0.0),
        ave_pkt_size_over_1000(0.0),
        ave_pkt_size_under_300(0.0),
        cnt_len_over_1000(0.0),
        payload_size(0.0), payload_bandwidth(0.0),
        udp_nopayload_rate(0.0),
        ret_rate(0.0),
        ave_rtt(0), rtt_min(0), rtt_max(0), rtt_range(0),

		bytes_of_flow(0), bytes_of_payload(0), header_of_packets(0), bytes_of_ret_packets(0),
		count_of_TCPpackets(0), count_of_UDPpackets(0), count_of_ICMPpackets(0),
		end_to_end_latency(0.0), avg_window_size(0.0), avg_ttl(0.0), avg_payload_size(0.0),
		count_of_ret_packets(0), count_of_syn_packets(0), count_of_fin_packets(0), count_of_rst_packets(0), count_of_ack_packets(0), count_of_psh_packets(0), count_of_urg_packets(0),
		entropy_of_payload(0.0),
		count_of_forward_packets(0), count_of_backward_packets(0)
		// videoMetrics{0.0, "Unknown", 0.0, "Unknown", "Unknown", "Unknown", false, false, 0.0, false, false}
		//downloadMetrics{0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0.0, 0, 0, 0.0}
    {}
};

struct WholeFlowsFeature {
    std::vector<int> active_flow_count;
    std::vector<double> duration_distribution;
    std::vector<double> packet_count_distribution;
    std::vector<double> byte_size_distribution;
	std::pair<std::map<std::string, int>, std::map<std::string, int>> flow_of_same_ip;
    std::vector<double> average_packet_size_distribution;
    std::vector<double> ttl_distribution;
    std::vector<double> window_size_distribution;
    std::pair<std::map<u_int16_t, int>, std::map<u_int16_t, int>> flow_of_same_port;
	std::vector<double> end_to_end_lanteny_distribution;
    std::vector<double> payload_entropy_distribution;
    std::vector<double> flow_peak_traffic_distribution;
};
WholeFlowsFeature flowsFeature;

struct HttpRequestHeader {
    std::string name;
    std::string value;
};

struct HttpResponseHeader {
    std::string name;
    std::string value;
};

struct Cookie {
    std::string name;
    std::string value;
};

struct HttpRequest {
	std::string method;
	std::string url;
	std::string httpversion;
	std::string host;
    std::string connection;
    std::string user_agent;
    std::string cookies;
    std::string referer;
	std::string content_length;
	std::string accept;
	std::string accept_encoding;
	std::string content_encoding;
	std::string content_type;
	std::string server;
	std::string accept_language;
};

struct HttpResponse
{
	std::string httpversion;
	std::string status_code;
	std::string response_header;
	std::string date;
	std::string content_type;
	std::string content_length;
	std::string connection;
	std::string server;
};

struct ProtocolInfo {
    std::string mac_source = "00:00:00:00:00:00";
	std::string mac_destination = "00:00:00:00:00:00";
    uint16_t ethernet_type = 0; 
    int vlan_id = 1; 
    int mpls_label = 0; 
    uint16_t pppoe_session_id = 0; 

	std::string protocol_type = "TCP";
    std::string ip_version = "IPv4";
    uint8_t ip_tos = 0;
    uint16_t ip_header_checksum = 0;
    uint8_t ip_fragmentation_flag = 0;
    uint16_t ip_identifier = 0;
    uint8_t* ipv6_flow_label = nullptr;
    uint8_t ipv6_next_header = 0;
    std::string wireless_network_ssid = "default_ssid";

    size_t tcp_header_length = 0;
    uint16_t tcp_window_size = 0;
    bool syn_flag = false;
	bool fin_flag = false;
	bool rst_flag = false;
	bool psh_flag = false;
	bool urg_flag = false;
    uint32_t tcp_sequence_number = 0;
    uint32_t tcp_acknowledgement_number = 0;
    uint16_t udp_header_length = 0;
    uint16_t udp_checksum = 0;
    uint8_t icmp_type = 0;
    int icmp_code = 0;
    bool arp_request; 
    bool arp_reply; 
    std::vector<pcpp::DnsType> dns_query_type;

    std::string smtp_command;
    std::string dhcp_message_type;
    std::string sip_data;
};

struct SinglePacketInfo
{
	size_t payload_size = 0;
	double payload_entropy = 0.0;
	int packet_length = 0;
	std::string arrival_timestamp = "";
};

struct PacketsFeature
{
	double avg_packet_size = 0.0;
	uint32_t median_packet_size = 0;
	int max_packet_size = 0;
	int min_packet_size = 0;
	double std_packet_size = 0.0;

	double avg_packet_interval = 0.0;
	double median_packet_interval = 0.0;
	double max_interval_between_packets = 0.0;
	double min_interval_between_packets = 0.0;
	double std_packet_interval = 0.0;
	
	double packet_size_kurtosis = 0.0;
	double packet_size_skewness = 0.0;
	double packet_rate = 0.0;
	double byte_rate = 0.0;

	double syn_ack_time = 0.0;
	double fin_ack_time = 0.0;
	double psh_between_time = 0.0;
	double urg_between_time = 0.0;
};
PacketsFeature packetsFeature;// 包间特征

struct VideoStreamMetrics {
	double bitrate;
	std::string resolution;
	double frame_rate;
	std::string video_codec;
	std::string audio_codec;
	std::string packet_pattern;
	bool is_bursty;
	bool is_multiplexed;
	double bandwidth_usage;
	bool long_duration_connection;
	bool buffering_behavior;
};
VideoStreamMetrics videoMetrics;

struct DownloadMetrics {
	LL download_bytes = 0;
	double duration = 0.0;
	double traffic_ratio= 0.0;
	double average_download_rate= 0.0;
	int packet_loss_count = 0;
	double retransmitted_packets_ratio= 0.0;
	int download_session_count = 0;
	int parallel_threads = 0;
	double request_response_delay= 0.0;
	bool segmented_download = false;
	bool resume_downloading = false;
	double peak_trough_traffic = 0.0;
};
bool download_flag = false;
DownloadMetrics downloadMetrics;

class Flow
{
public:
	FlowKey flowKey;
	FlowFeature flowFeature;
	std::vector<RawPacket*> packets;//记录一条流的所有包
	//用于计算流特征
	LL latest_timestamp, latter_timestamp, interarrival_time_n, interarrival_time_ls, flow_duration, start_timestamp, used_ts;
	int pkt_count,rtt_count, window_size_ls, retrans, ack_no_payload, ttl;
	LL packets_size_sum, app_packets_size_sum, header_len_ls;
	double bandwith, appbandwith ,throughput, duration;
	double ave_pkt_size, app_ave_pkt_size,ave_interval, ave_windows, ave_rtt, rtt_min, rtt_max, interval_ls, rtt_ls;
	int tcp_packets, udp_packets, icmp_packets;
	
	Flow(FlowKey flowKey);
	void updateFeature(RawPacket* pkt);
	void addPacket(RawPacket* pkt);
	void terminate(bool flag);

	std::map<uint8_t, int> frequencyMap;
	int udp_nopayload_cnt;
	int cnt_len_over_1000, ave_pkt_size_over_1000;
	int cnt_len_under_300,ave_pkt_size_under_300;
	LL payload_size, ret_bytes;
	double payload_bandwidth;
	std::map<uint32_t, uint64_t> synSeqToTime; //每条流都含有一个
	std::map<uint32_t, uint64_t> finSeqToTime; //每条流都含有一个
	uint64_t packetBuffer;
	uint64_t ackBuffer;
	std::vector<double> rtts;
	RawPacket* pkt;
	int ret;
	std::vector<double> packets_size;
	std::vector<double> interval_vec;
	std::vector<double> syn_ack_vec;
	std::vector<double> fin_ack_vec;
};


uint64_t packet_cnt_of_pcap;
uint64_t bytes_cnt_of_pcap;
LL timestamp_of_first_packet = std::numeric_limits<LL>::max();
LL timestamp_of_last_packet = std::numeric_limits<LL>::min();
std::vector<std::string> toString;

uint32_t sps_parser_offset;
uint8_t sps_parser_base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string nanosecondsToDatetime(LL nanoseconds);
double calculateMedian(std::vector<double>& vec);
double calculateStandardVariance(std::vector<double>& vec);
FlowKey* generateFlowKey(const Packet* packet);
void fillSessionKeyWithFlowKey(SessionKey& SessionKey, const FlowKey& flowKey,  bool fromClient);
double calculateSkewness(const std::vector<double>& packets_size);
double calculateKurtosis(const std::vector<double>& packets_size);
uint8_t* extract_sps_from_rtp(uint8_t* rtp_payload, size_t payload_length, size_t* sps_length);
bool checkForRangeHeader(const pcpp::Packet& packet);
bool isResumedDownload(const std::vector<pcpp::Packet>& packets);
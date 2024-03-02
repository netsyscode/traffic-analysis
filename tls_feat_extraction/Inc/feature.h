
#include "tls_features.h"
#include <map>
#include <pcapplusplus/DnsLayer.h>

using namespace pcpp;
typedef long long LL; 
//流特征
struct FlowFeature
{
	TLSFingerprint* tlsFingerprint = nullptr;
	IPAddress srcIP, dstIP;
	uint16_t srcPort = 0, dstPort = 0;
	double startts = 0.0, endts = 0.0;			// 开始时间，结束时间
	LL pktcnt = 0;				// 包数
	double pktlen = 0.0, applen = 0.0;			// 包长，应用层长度
	double itvl = 0.0, bw = 0.0, appbw = 0.0, thp = 0.0, dur = 0.0;	// 包间隔，带宽，应用层带宽，吞吐率
	// 只比较吞吐量
	bool operator<(const struct FlowFeature &other)const {
		return thp < other.thp;
	}

	double ave_pkt_size_over_1000 = 0.0;//包长大于1000的包 的平均包长
	double ave_pkt_size_under_300 = 0.0;//包长小于300的包 的平均包长
	double cnt_len_over_1000 = 0.0;
	double payload_size = 0.0, payload_bandwidth = 0.0; //payload长度，带宽
	double udp_nopayload_rate = 0.0;//没有payload的udp包比例
	double ret_rate = 0.0; //重传比例
	double ave_rtt = 0.0, rtt_min = 0.0, rtt_max = 0.0, rtt_range = 0.0; //rtt均值，极值, 极差
 
	LL bytes_of_flow = 0, bytes_of_payload = 0, header_of_packets = 0, bytes_of_ret_packets = 0;
	int count_of_TCPpackets = 0, count_of_UDPpackets = 0, count_of_ICMPpackets = 0;
	int max_size_of_packet = 0, min_size_of_packet = 0;
	double end_to_end_latency = 0.0, avg_window_size, avg_ttl = 0.0, avg_payload_size = 0.0;
	int count_of_ret_packets = 0, count_of_syn_packets = 0, count_of_fin_packets = 0, count_of_rst_packets = 0, count_of_ack_packets = 0, count_of_psh_packets = 0, count_of_urg_packets = 0;
	double entropy_of_payload = 0.0;
	int count_of_forward_packets = 0, count_of_backward_packets = 0;
};

// 流间特征
struct FlowsFeature {
	std::pair<std::map<std::string, int>, std::map<std::string, int>> flow_of_same_ip;
	std::pair<std::map<u_int16_t, int>, std::map<u_int16_t, int>> flow_of_same_port;
    int max_active_flow_count;
    std::vector<double> flow_duration_distribution;
    std::vector<double> packet_count_distribution;
    std::vector<double> byte_size_distribution;
    std::vector<double> average_packet_size_distribution;
    std::vector<double> avg_ttl_distribution;
    std::vector<double> window_size_distribution;
	std::vector<double> end_to_end_lanteny_distribution;
    std::vector<double> payload_entropy_distribution;
};

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
    bool arp_request = false; 
    bool arp_reply = false; 
    std::vector<pcpp::DnsType> dns_query_type;

    std::string smtp_command = "";
    std::string dhcp_message_type = "";
    std::string sip_data = "";
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
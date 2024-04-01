#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <getopt.h>
#include <chrono>
#include <thread>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <ctime>
#include <iomanip>
#include "predict.h"
#include "tls_features.h"
#include "global.h"
#include <climits>

HandlePacketData data;
FlowsFeature flowsFeature;
std::string app_label;

bool isDownloadSessions(const FlowKey key, const LL &payload_size); 
void calculateFlowsFeature(const HandlePacketData* data);
void insertFlowFeatureIntoMySQL(const std::vector<FlowFeature>& features, std::unique_ptr<sql::Connection>& con);
void insertProtocolFeatureIntoMySQL(const std::vector<ProtocolInfo>& data, std::unique_ptr<sql::Connection>& con);
void insertPacketFeatureIntoMySQL(const std::vector<SinglePacketInfo>& data, std::unique_ptr<sql::Connection>& con);
void insertPacketsFeatureIntoMySQL(const std::vector<PacketsFeature>& features, std::unique_ptr<sql::Connection>& con);
void insertFlowsFeatureIntoMySQL(const FlowsFeature& flowsFeature, std::unique_ptr<sql::Connection>& con);
void insertHttpRequestIntoMySQL(const std::vector<HttpRequest>& requests, std::unique_ptr<sql::Connection>& con);
std::pair<std::map<u_int16_t, int>, std::map<u_int16_t, int>> countFlowsByPorts(const HandlePacketData* data);
std::pair<std::map<std::string, int>, std::map<std::string, int>> countFlowsByIP(const HandlePacketData* data);
std::vector<double> calculatePercentiles(const std::vector<double>& a);
std::vector<double> getFlowsValues(const HandlePacketData* data, const char fieldName[]);
std::map<std::string, std::string> readConfig(const std::string& configFile); 

static struct option TLSFingerprintingOptions[] =
{
	{"interface",  required_argument, 0, 'i'},
	{"input-file",  required_argument, 0, 'r'},
	{"output-file", required_argument, 0, 'o'},
	{"filter", required_argument, 0, 'f'},
	{"app_label", required_argument, 0, 'l'},
	{0, 0, 0, 0}
};

#define EXIT_WITH_ERROR(reason) do { \
	std::cout << "ERROR: " << reason << std::endl << std::endl; \
	exit(1); \
	} while(0)

bool isNotAlphanumeric(char c)
{
	return std::isalnum(c) == 0;
}

/**
 * An auxiliary method for sorting the TLS fingerprint count map. Used in printCommonTLSFingerprints()
 */
bool stringCountComparer(std::pair<std::string, uint64_t> first, std::pair<std::string, uint64_t> second)
{
	if (first.second == second.second)
	{
		return first.first > second.first;
	}
	return first.second > second.second;
}


// store the finger print of sessions
std::map<SessionKey, TLSFingerprint> tlsFingerprintMap;

/**
 * Print cipher-suite map in a table sorted by number of occurrences (most common cipher-suite will be first)
 */
void printCommonTLSFingerprints(const std::map<std::string, uint64_t>& tlsFingerprintMap, int printCountItems)
{
	// create the table
	std::vector<std::string> columnNames;
	columnNames.push_back("TLS Fingerprint");
	columnNames.push_back("Count");
	std::vector<int> columnsWidths;
	columnsWidths.push_back(32);
	columnsWidths.push_back(7);
	TablePrinter printer(columnNames, columnsWidths);

	// sort the TLS fingerprint map so the most popular will be first
	// since it's not possible to sort a std::map you must copy it to a std::vector and sort it then
	std::vector<std::pair<std::string, int> > map2vec(tlsFingerprintMap.begin(), tlsFingerprintMap.end());
	std::sort(map2vec.begin(),map2vec.end(), &stringCountComparer);

	// go over all items (fingerprints + count) in the sorted vector and print them
	for(std::vector<std::pair<std::string, int> >::iterator iter = map2vec.begin();
			iter != map2vec.end();
			iter++)
	{
		if (iter - map2vec.begin() >= printCountItems)
			break;

		std::stringstream values;
		values << iter->first << "|" << iter->second;
		printer.printRow(values.str(), '|');
	}
}

// 处理每一个包
/**
 * @brief 处理每一个数据包
 * @param rawPacket 原始数据包
 * @param data 包含所有流的信息
 */
void handlePacket(RawPacket* rawPacket, const HandlePacketData* data)
{
	Packet* parsedPacket;
	// 丢掉坏包
	try {
		parsedPacket = new Packet(rawPacket);
		// check if the packet is IP packet
		if (parsedPacket->isPacketOfType(IPv4))//IPv4
		{
			// generate flow key for the packet
			FlowKey* flowKey = generateFlowKey(parsedPacket);

			if (flowKey == NULL) return;
			std::map<FlowKey, Flow*>::iterator flowIter = data->flows->find(*flowKey);
			if (flowIter == data->flows->end())
			{
				//starting a new flow
				Flow* flow = new Flow(*flowKey);
				flow->addPacket(rawPacket);
				long sec = rawPacket->getPacketTimeStamp().tv_sec;
				long nsec = rawPacket->getPacketTimeStamp().tv_nsec;
				flow->start_timestamp = sec * 1000000000LL + nsec;
				if(data->flows->empty()) timestamp_of_first_packet = flow->start_timestamp;
				flow->latter_timestamp = flow->start_timestamp;
				data->flows->insert(std::pair<FlowKey, Flow*>(*flowKey, flow));
			}
			else
			{
				flowIter->second->addPacket(rawPacket);
			}

			// check if the packet contains SSL/TLS layer
			if (parsedPacket->isPacketOfType(SSL))
			{
				// extract the SSL/TLS handhsake layer
				SSLHandshakeLayer* sslHandshakeLayer = parsedPacket->getLayerOfType<SSLHandshakeLayer>();
				if (sslHandshakeLayer != NULL)
				{
					// check if the SSL/TLS handhsake layer contains a ClientHello message
					SSLClientHelloMessage* clientHelloMessage = sslHandshakeLayer->getHandshakeMessageOfType<SSLClientHelloMessage>();
					if (clientHelloMessage != NULL)
					{
						// extract the TLS session
						SessionKey sessionKey = generateSessionKey(parsedPacket, true);
						// extract the TLS fingerprint
						ClientHelloFingerprint tlsFingerprint = generateClientHelloFingerprint(clientHelloMessage);
						std::string tlsFingerprintString = tlsFingerprint.toString();
						// find the TLS fingerprint in the map, update the related part
						if (data->stats->find(sessionKey) != data->stats->end())
						{
							data->stats->at(sessionKey)->clientHelloFingerprint = tlsFingerprint;
						}
						else
						{
							TLSFingerprint* tlsFingerprints = new TLSFingerprint();
							tlsFingerprints->clientHelloFingerprint = tlsFingerprint;
							data->stats->insert(std::pair<SessionKey, TLSFingerprint*>(sessionKey, tlsFingerprints));//insert stats
						}
						return;
					}
					// check if the SSL/TLS handhsake layer contains a ServerHello message
					SSLServerHelloMessage* servertHelloMessage = sslHandshakeLayer->getHandshakeMessageOfType<SSLServerHelloMessage>();
					if (servertHelloMessage != NULL)
					{
						// extract the TLS session
						SessionKey sessionKey = generateSessionKey(parsedPacket, false);
						// extract the TLS fingerprint
						ServerHelloFingerprint tlsFingerprint = generateServerHelloFingerprint(servertHelloMessage);
						std::string tlsFingerprintString = tlsFingerprint.toString();
						// find the TLS fingerprint in the map, update the related part
						if (data->stats->find(sessionKey) != data->stats->end())
						{
							data->stats->at(sessionKey)->serverHelloFingerprint = tlsFingerprint;
						}
						else
						{
							TLSFingerprint* tlsFingerprints = new TLSFingerprint();
							tlsFingerprints->serverHelloFingerprint = tlsFingerprint;
							data->stats->insert(std::pair<SessionKey, TLSFingerprint*>(sessionKey, tlsFingerprints));
						}
						return;
					}
				}
			}
		}
	} 
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
		return;
	}
}

// 所有数据已存于流中
/**
 * @brief 计算流的特征
 * @param data 包含所有流的信息
 */
void calculateFlowFeature(const HandlePacketData* data)
{
	if (data->flows->size() >= 1) {
		// 处理每一条流
		for (auto it = data->flows->begin(); it != data->flows->end(); ++it) {
			// build SeesionKey from FlowKey, add TLS fingerprint to flow feature
			SessionKey sessionKey;
			fillSessionKeyWithFlowKey(sessionKey, it->first, true);//from client
			if (data->stats->find(sessionKey) != data->stats->end()) {
				it->second->flowFeature.tlsFingerprint = data->stats->at(sessionKey);
			}
			else {
				fillSessionKeyWithFlowKey(sessionKey, it->first, false);
				if (data->stats->find(sessionKey) != data->stats->end())
					it->second->flowFeature.tlsFingerprint = data->stats->at(sessionKey);
			}

			if(isDownloadSessions(it->second->flowKey, it->second->payload_size))
				download_flag = true;
			it->second->terminate(download_flag);
			download_flag = false;
		}
	}
}

/**
 * @brief 将流进行分类
 * @param data 包含所有流的信息
 * @param model SVM分类器
 */
void classficationFlows(const HandlePacketData* data, SVMPredictor* model)
{
	auto flows = *(data->flows);
	if (flows.size() > 0)
	{
		// iterate over all items in the map and print them
		for(auto iter = flows.begin(); iter != flows.end(); iter++)
		{
			FlowKey flowKey = iter->first;
			Flow* flow = iter->second;
			const FlowFeature* flowFeature = &(flow->flowFeature);
			std::vector<double> x(24, 0.0);
			x[0] = flowKey.srcPort;
			x[1] = flowKey.dstPort;
			if (flowFeature->tlsFingerprint != NULL) {
				auto clientDigest = flowFeature->tlsFingerprint->clientHelloFingerprint.toDigest().digest;
				auto serverDigest = flowFeature->tlsFingerprint->serverHelloFingerprint.toDigest().digest;
				for (size_t i = 0; i < 5; i++)
					x[2+i] =  (u_int32_t) clientDigest[i] ;
				for (size_t i = 0; i < 3; i++)
					x[7+i] =  (u_int32_t) serverDigest[i] ;
			} else {
				for (size_t i = 0; i < 8; i++)
					x[2+i] = 0;
			}
			x[10] = flowFeature->bw;
			x[11] = flowFeature->itvl;
			x[12] = flowFeature->pktlen;
			x[14] = flowFeature->ave_pkt_size_under_300;
			x[15] = flowFeature->ave_pkt_size_over_1000;
			x[18] = flowFeature->udp_nopayload_rate;
			x[19] = flowFeature->ave_rtt;
			x[23] = flowFeature->ret_rate; // ???
			std::pair<int, double> temp = model->predictForPredictedClass(x);
			//flow->flowFeature.res = temp.first;
			//flow->flowFeature.dis = temp.second;
			std::cout << flowKey.srcIP << " "<< flowKey.dstIP << "   "<< temp.first  << "   "<< temp.second << std::endl;
		}
	}
}

/**
 * @brief 从pcap文件中提取TLS指纹
 * @param inputPcapFileName 输入pcap文件的文件名
 * @param outputFileName 输出文件的文件名
 * @param bpfFilter BPF过滤器
 * @param modelPath 分类模型的路径
 */
void doTlsFingerprintingOnPcapFile(const std::string& inputPcapFileName, std::string& outputFileName, const std::string& bpfFilter, std::string& modelPath)
{
	IFileReaderDevice* reader = IFileReaderDevice::getReader(inputPcapFileName.c_str());
	if (!reader->open())
		EXIT_WITH_ERROR("Cannot open pcap/pcapng file");

	std::string outputPath = "./Output/test/";
	std::string pengdingPath;
	if (outputFileName.empty())
		pengdingPath = inputPcapFileName;	
	else
		pengdingPath = outputFileName;
	size_t fileNameOffset = pengdingPath.find_last_of("\\/") + 1;
	size_t extensionOffset = pengdingPath.find_last_of(".");
	std::string fileNameWithoutExtension = pengdingPath.substr(fileNameOffset, extensionOffset - fileNameOffset);
	outputFileName = outputPath + fileNameWithoutExtension + ".csv";

	std::ofstream outputFile(outputFileName.c_str());
	if (!outputFile){
		EXIT_WITH_ERROR("Cannot open output file '" << outputFileName << "'");
	}

	// write the column headers to the output file
	//writeFlowFeatureHeaderToOutputFile(outputFile);

	// set BPF filter if provided by the user
	if (!bpfFilter.empty()){
		if (!reader->setFilter(bpfFilter))
			EXIT_WITH_ERROR("Error in setting BPF filter to the pcap file");
	}

	std::cout << "Start reading '" << inputPcapFileName << "'..." << std::endl;

	std::map<SessionKey, TLSFingerprint*> stats;
	std::map<FlowKey, Flow*> flows;
	std::vector<HttpRequest> webRequestVector;
	std::vector<HttpResponse> webresponse;
	std::vector<SinglePacketInfo> singlePacketInfoVector;
	std::vector<ProtocolInfo> protocolInfoVector;
	std::vector<PacketsFeature> packetsFeatureVector;
    std::vector<FlowFeature> flowFeatureVector;	

	//data.outputFile = &outputFile;
	data.stats = &stats;
	data.flows = &flows;
	data.WebRequestVector = &webRequestVector;
	data.WebResponseVector = &webresponse;
	data.singlePacketInfoVector = &singlePacketInfoVector;
	data.protocolInfoVector = &protocolInfoVector;
	data.packetsFeatureVector= &packetsFeatureVector;
	data.flowFeatureVector = &flowFeatureVector;

	data.videoMetrics = &videoMetrics;
	data.downloadMetrics = &downloadMetrics;
	data.flowsFeature = &flowsFeature;

	// 读取pcap中的各个包
	try {
		RawPacket rawPacket;
		while (reader->getNextPacket(rawPacket)){
			handlePacket(&rawPacket, &data);
		}
	}
	catch(const std::exception& e) {
		std::cerr << "Flow capture has been shut due to capture problems." << '\n';
	}

	reader->close();
	delete reader;

	calculateFlowFeature(&data);
	calculateFlowsFeature(&data);

	// 向数据库中写入数据
	try {
		sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

		//连接至数据库
		auto config = readConfig("dbconfig.ini");
		std::unique_ptr<sql::Connection> con(driver->connect(config["host"], config["user"], config["password"]));
		con->setSchema("TrafficData");
		// insertFlowFeatureIntoMySQL(flowFeatureVector, con);
		// insertProtocolFeatureIntoMySQL(protocolInfoVector, con);
		// insertPacketFeatureIntoMySQL(singlePacketInfoVector, con);
		// insertPacketsFeatureIntoMySQL(packetsFeatureVector, con);
		// insertFlowsFeatureIntoMySQL(flowsFeature, con);
		insertHttpRequestIntoMySQL(webRequestVector, con);
	}catch(std::exception &e){
		std::cerr << e.what() << std::endl;
	}

	// SVMPredictor predictor(modelPath, 1.0/24.0);
 	// std::cout << "--------------------------------------------------------" << std::endl;
 	// std::cout << "Successfully load model from '" << modelPath << "'" << std::endl;
 	// std::cout << "--------------------------------------------------------" << std::endl;
	
	//classficationFlows(&data, &predictor);
	//std::cout << "Output file was written to: '" << outputFileName << "'" << std::endl;
}


/**
 * main method of this utility
 */
int main(int argc, char* argv[])
{
	AppName::init(argc, argv);

	std::string interfaceNameOrIP;
	std::string inputPcapFileName;
	std::string outputFileName;
	std::string bpfFilter;
	std::string modelPath="../model/ovr_params.json";
	int optionIndex = 0;
	int opt = 0;

	while ((opt = getopt_long(argc, argv, "i:r:o:f:l:", TLSFingerprintingOptions, &optionIndex)) != -1) {
		switch (opt)
		{
			case 0:
				break;
			case 'i':
				interfaceNameOrIP = optarg;
				break;
			case 'r':
				inputPcapFileName = optarg;
				break;
			case 'o':
				outputFileName = optarg;
				break;
			case 'f':
				bpfFilter = optarg;
				break;
			case 'l':
				app_label = optarg;
				break;
			default:
				exit(-1);
		}
	}


	// if no interface or input pcap file provided or both are provided- exit with error
	if (inputPcapFileName.empty() == interfaceNameOrIP.empty())
	{
		EXIT_WITH_ERROR("Please provide an interface or an input pcap file");
	}
	if (!inputPcapFileName.empty())
	{
		doTlsFingerprintingOnPcapFile(inputPcapFileName, outputFileName, bpfFilter, modelPath);
	}
}

int countActiveFlowsEveryFiveSeconds(const HandlePacketData* data) {
    if (data == nullptr || data->flows == nullptr) return {};

    // 找出所有流中最早和最晚的时间戳
    LL minStart = LONG_LONG_MAX, maxEnd = LONG_LONG_MIN;
    for (const auto& pair : *data->flows) {
        const Flow* flow = pair.second;
        minStart = std::min(minStart, flow->start_timestamp);
        maxEnd = std::max(maxEnd, flow->latest_timestamp);
    }
    // 初始化结果向量
    int res = 0;

    // 遍历每个流
    for (const auto& pair : *data->flows) {
		int active_flow_count = 0;
        const Flow* flow = pair.second;
        int startInterval = (flow->start_timestamp - minStart) /1e9 / 5;
        int endInterval = (flow->latest_timestamp - minStart) / 1e9 / 5;
        // 在流活跃的每个时间窗口内增加计数
        for (int i = startInterval; i <= endInterval; ++i) {
            active_flow_count++;
        }
		res = std::max(res, active_flow_count);
    }
    return res;
}

// 统计下载会话
bool isDownloadSessions(const FlowKey key, const LL &payload_size) {
        size_t downloadDataSize = 0;
        // 假设下载流量主要通过HTTP/HTTPS，即端口80或443
        if (key.dstPort == 80 || key.dstPort == 443) {
            downloadDataSize += payload_size;
            // 如果下行数据量超过某个阈值，认为是下载会话
            if (downloadDataSize > DOWNLOAD_DATA_THRESHOLD) {
                return true;
            }
    }
    return false;
}

void calculateFlowsFeature(const HandlePacketData* data){
	flowsFeature.app_label = stoi(app_label);
	flowsFeature.max_active_flow_count = countActiveFlowsEveryFiveSeconds(data);
	flowsFeature.flow_of_same_port = countFlowsByPorts(data);
	flowsFeature.flow_of_same_ip = countFlowsByIP(data);

	flowsFeature.flow_duration_distribution = calculatePercentiles(getFlowsValues(data, "dur"));
	flowsFeature.packet_count_distribution = calculatePercentiles(getFlowsValues(data, "pktcnt"));
	flowsFeature.byte_size_distribution = calculatePercentiles(getFlowsValues(data, "bytes_of_flow"));
	flowsFeature.average_packet_size_distribution = calculatePercentiles(getFlowsValues(data, "avg_bytes_of_flow"));
	flowsFeature.avg_ttl_distribution = calculatePercentiles(getFlowsValues(data, "avg_ttl"));
	flowsFeature.window_size_distribution = calculatePercentiles(getFlowsValues(data, "avg_window_size"));
	flowsFeature.end_to_end_lanteny_distribution = calculatePercentiles(getFlowsValues(data, "end_to_end_latency"));
	flowsFeature.payload_entropy_distribution = calculatePercentiles(getFlowsValues(data, "entropy_of_payload"));

	if(downloadMetrics.download_session_count)
		downloadMetrics.segmented_download = true;
}
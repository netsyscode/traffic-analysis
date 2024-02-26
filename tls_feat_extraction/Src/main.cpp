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
#include "tls_features.h"
#include "handledata.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <ctime>
#include <iomanip>
#include "predict.h"
#include <climits>

HandlePacketData data;
PacketsFeature packetsFeature;
uint64_t packet_cnt_of_pcap;
uint64_t bytes_cnt_of_pcap;
long long timestamp_of_first_packet = std::numeric_limits<long long>::max();
long long timestamp_of_last_packet = std::numeric_limits<long long>::min();

std::string nanosecondsToDatetime(long long nanoseconds);
std::vector<double> calculatePercentiles(const std::vector<double>& a);
std::vector<double> getFlowsValues(const HandlePacketData* data, const char fieldName[]);
std::pair<std::map<u_int16_t, int>, std::map<u_int16_t, int>> countFlowsByPorts(const HandlePacketData* data);
std::pair<std::map<std::string, int>, std::map<std::string, int>> countFlowsByIP(const HandlePacketData* data);
std::vector<int> countActiveFlowsEveryFiveSeconds(const HandlePacketData* data);
static struct option TLSFingerprintingOptions[] =
{
	{"interface",  required_argument, 0, 'i'},
	{"input-file",  required_argument, 0, 'r'},
	{"output-file", required_argument, 0, 'o'},
	{"filter", required_argument, 0, 'f'},
	{"list-interfaces", no_argument, 0, 'l'},
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

/**
 * Go over all interfaces and output their names
 */
void listInterfaces()
{
	const std::vector<PcapLiveDevice*>& devList = PcapLiveDeviceList::getInstance().getPcapLiveDevicesList();

	std::cout << std::endl << "Network interfaces:" << std::endl;
	for (std::vector<PcapLiveDevice*>::const_iterator iter = devList.begin(); iter != devList.end(); iter++)
	{
		std::cout << "    -> Name: '" << (*iter)->getName() << "'   IP address: " << (*iter)->getIPv4Address().toString() << std::endl;
	}
	exit(0);
}

/**
 * The callback to be called when application is terminated by ctrl-c
 */
static void onApplicationInterrupted(void* cookie)
{
	bool* shouldStop = (bool*)cookie;
	*shouldStop = true;
}

// /**
//  * Write data about a single ClientHello/ServerHello packet to the output file.
//  * This method takes the parsed packets and the TLS fingerprint as inputs, extracts the rest of the data such as IP addresses and TCP ports,
//  * and writes a single row to the output file
//  */
// void writeToOutputFile(std::ofstream* outputFile, const Packet& parsedPacket, const std::string &tlsFPString, const std::string &tlsFPType)
// {
//     const char separator = '\t';
// 	std::pair<IPAddress, IPAddress> ipSrcDest = getIPs(parsedPacket);
// 	std::pair<uint16_t, uint16_t> tcpPorts = getTcpPorts(parsedPacket);

// 	*outputFile <<
// 		tlsFPString << separator <<
// 		tlsFPType << separator <<
// 		ipSrcDest.first.toString() << separator <<
// 		tcpPorts.first << separator <<
// 		ipSrcDest.second.toString() << separator <<
// 		tcpPorts.second << std::endl;
// }

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

/**
 * Write the column headers to the output file
 */
void writeHeaderToOutputFile(std::ofstream& outputFile)
{
    const char separator = '\t';
	outputFile <<						
		"Src IP" << separator <<
		"Dst IP" << separator <<
		"Src Port" << separator <<
		"Dst Port" << separator <<
		"Client TlsVersion" << separator <<
		"Client Cipher Suits" << separator <<
		"Client Extensions" << separator <<
		"Client SupportedGroups" << separator <<
		"Client EcPointFormats" << separator <<
		"Server TlsVersion" << separator <<
		"Server cipherSuite" << separator <<
		"Server Extensions" << separator <<
		"AppBandwidth" << separator <<
		"Flow Duration" << separator <<
		"Avg Packet Interval" << separator <<
		"Packets count" << separator << 
		"AvgPacket Length" << separator <<
		//"Throughoutput" << separator <<
		"AvgPacket Length under 300" << separator <<
		"AvgPacket Length over 1000" << separator <<
		//"Payload Length" << separator <<
		//"Payload Bandwidth" << separator <<
		"UDP without Payload Rate" << separator <<
		"RTT" << separator <<
		// "RTT Min" << separator <<
		// "RTT Max" << separator <<
		// "RTT Range" << separator <<
		"Retransfer Rate" <<
		std::endl;
}

// store the finger print of sessions
std::map<SessionKey, TLSFingerprint> tlsFingerprintMap;

// struct HandlePacketData
// {
// 	std::ofstream* outputFile;
// 	std::map<SessionKey, TLSFingerprint*>* stats;
// 	std::map<FlowKey, Flow*>* flows;
// };

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

/**
 * Print TLS fingerprinting stats
 */
void writeToOutputData(const HandlePacketData* data, std::vector<std::string> &toString, const std::string& tableName)
{
	//const char separator = '\t';
	auto flows = *(data->flows);
	//auto outputFile = data->outputFile;
	// if there is ClientHello data to show
	if (flows.size() > 0)
	{
		// iterate over all items in the map and print them
		for(auto iter = flows.begin(); iter != flows.end(); iter++)
		{
			FlowKey flowKey = iter->first;
			Flow* flow = iter->second;
			const FlowFeature* flowFeature = &(flow->flowFeature);
			toString.push_back(tableName);
			toString.push_back(nanosecondsToDatetime(flow->start_timestamp));//流起始时间
			toString.push_back(flowKey.srcIP.toString());//源IP
			toString.push_back(flowKey.dstIP.toString());//目的IP
			toString.push_back(std::to_string(flowKey.srcPort));//源端口号
			toString.push_back(std::to_string(flowKey.dstPort));//目的端口号

			// *outputFile << flowKey.srcIP.toString() << separator;
			if (flowFeature->tlsFingerprint != NULL) {
				auto clientDigest = flowFeature->tlsFingerprint->clientHelloFingerprint.toDigest().digest;
				auto serverDigest = flowFeature->tlsFingerprint->serverHelloFingerprint.toDigest().digest;
				for (size_t i = 0; i < 5; i++){
					// *outputFile << (u_int32_t) clientDigest[i] << separator;
					toString.push_back(std::to_string((u_int32_t) clientDigest[i]));//client加密套件
				}
				for (size_t i = 0; i < 3; i++){
					// *outputFile << (u_int32_t) serverDigest[i] << separator;
					toString.push_back(std::to_string((u_int32_t) serverDigest[i]));//server加密套件
				}
			} else {
				for (size_t i = 0; i < 8; i++){
					// *outputFile << 0 << separator;
					toString.push_back("0");
				}
			}

			toString.push_back(std::to_string(flowFeature->bw));//flowBandwidth
			toString.push_back(std::to_string(flowFeature->itvl));//flowInterval
			toString.push_back(std::to_string(flowFeature->pktcnt));//packetCount
			toString.push_back(std::to_string(flowFeature->pktlen));//avgPacketLength
			toString.push_back(std::to_string(flowFeature->ave_pkt_size_under_300));//packetLengthUnder300
			toString.push_back(std::to_string(flowFeature->ave_pkt_size_over_1000));//packetLengthOver1000
			toString.push_back(std::to_string(flowFeature->udp_nopayload_rate));//UDPwithoutPayloadRate
			toString.push_back(std::to_string(flowFeature->ave_rtt));//RTT
			toString.push_back(std::to_string(flowFeature->ret_rate));//RetransferRate
			// *outputFile << flowFeature->bw << separator	
			// 	<< flowFeature->dur << separator
			// 	<< flowFeature->itvl << separator	
			// 	<< flowFeature->pktcnt << separator
			// 	<< flowFeature->pktlen << separator	;
				//<< flowFeature->thp<< separator	;
			// *outputFile << flowFeature->ave_pkt_size_under_300 << separator	 
			// << flowFeature->ave_pkt_size_over_1000 << separator	
				//<< flowFeature->payload_size << separator	
				//<< flowFeature->payload_bandwidth << separator	
				//<< flowFeature->udp_nopayload_rate<< separator	
				//<< flowFeature->ave_rtt << separator	
				//<< flowFeature->rtt_min << separator	
				//<< flowFeature->rtt_max << separator	
				// << flowFeature->rtt_range<< separator	
				//<< flowFeature->ret_rate << separator	
				//<< std::endl;
		}
	}
}

void insertDataIntoMySQL(const std::vector<std::string>& data) {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));

        con->setSchema("Dataset");

        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS feature (appName VARCHAR(100), timeStamp DATETIME(6), srcIP VARCHAR(20), dstIP VARCHAR(20),srcPort INT, dstPort INT,\
			clientTlsVersion VARCHAR(10) ,clientCipherSuits VARCHAR(10), clientExtensions VARCHAR(10), clientSupportedGroups VARCHAR(10), clientEcPointFormats VARCHAR(10),\
			serverTlsVersion VARCHAR(10) ,serverCipherSuits VARCHAR(10), serverExtensions VARCHAR(10), appBandwidth DOUBLE, flowDuration DOUBLE,\
			packetCount BIGINT, packetLengthOfFlow BIGINT, packetLengthUnder300 DOUBLE, packetLengthOver1000 DOUBLE, UDPwithoutPayloadRate DOUBLE, RTT DOUBLE, retransferRate DOUBLE)");

        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO feature (appName, timeStamp, srcIP, dstIP, srcPort, dstPort, clientTlsVersion ,clientCipherSuits, clientExtensions, \
					clientSupportedGroups, clientEcPointFormats,serverTlsVersion ,serverCipherSuits, serverExtensions, appBandwidth , flowDuration ,packetCount, \
					packetLengthOfFlow, packetLengthUnder300, packetLengthOver1000, UDPwithoutPayloadRate, RTT, retransferRate) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        for (size_t i = 0; i < data.size(); i += 23) {
			pstmt->setString(1, data[i]);
            pstmt->setDateTime(2, data[i + 1]);
            pstmt->setString(3, data[i + 2]);
            pstmt->setString(4, data[i + 3]);
			pstmt->setInt(5, std::stoi(data[i + 4]));
			pstmt->setInt(6, std::stoi(data[i + 5]));
			if(std::stoi(data[i + 6]) == 3)
            	pstmt->setString(7, "TLS1.2");
			else
				pstmt->setString(7, "其他");
            pstmt->setString(8, data[i + 7]);
            pstmt->setString(9, data[i + 8]);
			pstmt->setString(10, data[i + 9]);
			pstmt->setString(11, data[i + 10]);
			if(std::stoi(data[i + 11]) == 3)
            	pstmt->setString(12, "TLS1.2");
			else
				pstmt->setString(12, "其他");
            pstmt->setString(13, data[i + 12]);
			pstmt->setString(14, data[i + 13]);
            pstmt->setDouble(15, std::stod(data[i + 14]));
			pstmt->setDouble(16, std::stod(data[i + 15]));
			pstmt->setBigInt(17, data[i + 16]);
			pstmt->setBigInt(18, data[i + 17]);
			pstmt->setDouble(19, std::stod(data[i + 18]));
			pstmt->setDouble(20, std::stod(data[i + 19]));
			pstmt->setDouble(21, std::stod(data[i + 20]));
			pstmt->setDouble(22, std::stod(data[i + 21]));
			pstmt->setDouble(23, std::stod(data[i + 22]));

            pstmt->executeUpdate();
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertDataIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

/**
 * Handle an intercepted packet: identify if it's a ClientHello or ServerHello packets, extract the TLS fingerprint and write it to the output file
 */

void handlePacket(RawPacket* rawPacket, const HandlePacketData* data)
{
	Packet* parsedPacket;
	// throw the bad packets
	try {
		parsedPacket = new Packet(rawPacket);
		// check if the packet is IP packet
		if (parsedPacket->isPacketOfType(IPv4))//IPv4
		{
			// generate flow key for the packet
			FlowKey* flowKey = generateFlowKey(parsedPacket);

			// TODO: fix mem bug
			// find the flow by flowkey, then update flowkey-flow mapping
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
				//flow->start_timestamp = rawPacket->getPacketTimeStamp().tv_nsec;//fixed the duration and bandwidth bugs
				//flow->flowFeature.startts = rawPacket->getPacketTimeStamp().tv_nsec; 
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
						// // extract the TLS session
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

void calculateFlowFeature(const HandlePacketData* data)
{
	if (data->flows->size() >= 1) {
		for (auto it = data->flows->begin(); it != data->flows->end(); ++it) {
			it->second->terminate();
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
		}
	}
}

WholeFlowsFeature flowsFeature;
void calculateWholeFlowFeature(const HandlePacketData* data){
	flowsFeature.active_flow_count = countActiveFlowsEveryFiveSeconds(data);
	flowsFeature.flow_of_same_port = countFlowsByPorts(data);
	flowsFeature.flow_of_same_ip = countFlowsByIP(data);

	flowsFeature.duration_distribution = calculatePercentiles(getFlowsValues(data, "dur"));
	flowsFeature.packet_count_distribution = calculatePercentiles(getFlowsValues(data, "pktcnt"));
	flowsFeature.byte_size_distribution = calculatePercentiles(getFlowsValues(data, "bytes_of_flow"));
	flowsFeature.average_packet_size_distribution = calculatePercentiles(getFlowsValues(data, "avg_bytes_of_flow"));
	flowsFeature.ttl_distribution = calculatePercentiles(getFlowsValues(data, "avg_ttl"));
	flowsFeature.window_size_distribution = calculatePercentiles(getFlowsValues(data, "avg_window_size"));
	flowsFeature.end_to_end_lanteny_distribution = calculatePercentiles(getFlowsValues(data, "end_to_end_latency"));
	flowsFeature.payload_entropy_distribution = calculatePercentiles(getFlowsValues(data, "entropy_of_payload"));
	flowsFeature.flow_peak_traffic_distribution = calculatePercentiles(getFlowsValues(data, "flow_peak_traffic"));

	packetsFeature.packet_rate = packet_cnt_of_pcap  / (timestamp_of_last_packet - timestamp_of_first_packet) * 1e9;
	packetsFeature.packet_rate = bytes_cnt_of_pcap  / (timestamp_of_last_packet - timestamp_of_first_packet) * 1e9;

}


// void calculateFlowFeature(const HandlePacketData* data)
// {
// 	if (data->flows->size() >= 1) {
// 		std::map<FlowKey, Flow>::iterator it;
// 		std::vector<FlowFeature> flowFeatures;
// 		for (auto it = data->flows->begin(); it != data->flows->end(); ++it) {
// 			it->second->terminate();
// 			// build SeesionKey from FlowKey, add TLS fingerprint to flow feature
// 			SessionKey sessionKey;
// 			fillSessionKeyWithFlowKey(sessionKey, it->first, true);
// 			if (data->stats->find(sessionKey) != data->stats->end()) {
// 				it->second->flowFeature.tlsFingerprint = data->stats->at(sessionKey);
// 			} else {
// 				fillSessionKeyWithFlowKey(sessionKey, it->first, false);
// 				if (data->stats->find(sessionKey) != data->stats->end())
// 					it->second->flowFeature.tlsFingerprint = data->stats->at(sessionKey);
// 			}
// 		}
// 	}
// }


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
			x[13] = flowFeature->thp;
			x[14] = flowFeature->ave_pkt_size_under_300;
			x[15] = flowFeature->ave_pkt_size_over_1000;
			x[16] = flowFeature->payload_size;
			x[17] = flowFeature->payload_bandwidth;
			x[18] = flowFeature->udp_nopayload_rate;
			x[19] = flowFeature->ave_rtt;
			x[20] = flowFeature->rtt_min;	
			x[21] = flowFeature->rtt_max;	
			x[22] = flowFeature->rtt_range;	
			x[23] = flowFeature->ret_rate;
			std::pair<int, double> temp = model->predictForPredictedClass(x);
			//flow->flowFeature.res = temp.first;
			//flow->flowFeature.dis = temp.second;
			std::cout << flowKey.srcIP << " "<< flowKey.dstIP << "   "<< temp.first  << "   "<< temp.second << std::endl;
		}
	}
}

/**
 * Extract TLS fingerprints from a pcap/pcapng file
 */
void doTlsFingerprintingOnPcapFile(const std::string& inputPcapFileName, std::string& outputFileName, const std::string& bpfFilter, std::string& modelPath)
{
	// open input file (pcap or pcapng file)
	IFileReaderDevice* reader = IFileReaderDevice::getReader(inputPcapFileName.c_str());
	// try to open the file device
	if (!reader->open())
		EXIT_WITH_ERROR("Cannot open pcap/pcapng file");

	// set output file name to input file name if not provided by the user
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

	// open output file
	std::ofstream outputFile(outputFileName.c_str());
	if (!outputFile)
	{
		EXIT_WITH_ERROR("Cannot open output file '" << outputFileName << "'");
	}

	// write the column headers to the output file
	writeHeaderToOutputFile(outputFile);

	// set BPF filter if provided by the user
	if (!bpfFilter.empty())
	{
		if (!reader->setFilter(bpfFilter))
			EXIT_WITH_ERROR("Error in setting BPF filter to the pcap file");
	}

	std::cout << "Start reading '" << inputPcapFileName << "'..." << std::endl;


	std::map<SessionKey, TLSFingerprint*> stats;
	std::map<FlowKey, Flow*> flows;
	std::vector<HttpRequest> webrequest;
	std::vector<HttpResponse> webresponse;
	std::vector<SinglePacketInfo> packetInfoVector;
	std::vector<PacketsFeature> packetsInfoVector;
	std::vector<ProtocolInfo> protocolInfoVector;

	data.outputFile = &outputFile;
	data.stats = &stats;
	data.flows = &flows;
	data.WebRequest = &webrequest;
	data.WebResponse = &webresponse;
	data.singlePacketInfoVector = &packetInfoVector;
	data.packetsInfoVector = &packetsInfoVector;
	data.protocolInfoVector = &protocolInfoVector;
	std::vector<std::string> toString;
	RawPacket rawPacket;

	// iterate over all packets in the file
	try {
		while (reader->getNextPacket(rawPacket)){
			handlePacket(&rawPacket, &data);
		}
	}
	catch(const std::exception& e) {
		std::cerr << "Flow capture has been shut due to capture problems." << '\n';
	}

	// close the reader and free its memory
	reader->close();
	delete reader;

    //write feature data to database
	std::cout << "here" << std::endl;
	calculateFlowFeature(&data);
	calculateWholeFlowFeature(&data);
	writeToOutputData(&data, toString, fileNameWithoutExtension);
	//insertDataIntoMySQL(toString);
	std::cout << "Having written " << toString.size() / 4 << " rows into the table\n\n";

	SVMPredictor predictor(modelPath, 1.0/24.0);
 	std::cout << "--------------------------------------------------------" << std::endl;
 	std::cout << "Successfully load model from '" << modelPath << "'" << std::endl;
 	std::cout << "--------------------------------------------------------" << std::endl;
	
	//classficationFlows(&data, &predictor);
	//std::cout << "Output file was written to: '" << outputFileName << "'" << std::endl;
}


/**
 * packet capture callback - called whenever a packet arrives on the live interface (in live device capturing mode)
 */
static void onPacketArrives(RawPacket* rawPacket, PcapLiveDevice* dev, void* cookie)
{
	HandlePacketData* data = static_cast<HandlePacketData*>(cookie);
	handlePacket(rawPacket, data);
}


/**
 * Extract TLS fingerprints from a live interface
 */
void doTlsFingerprintingOnLiveTraffic(const std::string& interfaceNameOrIP, std::string& outputFileName, const std::string& bpfFilter, std::string& modelPath)
{
	// extract pcap live device by interface name or IP address
	PcapLiveDevice* dev = PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIpOrName(interfaceNameOrIP);
	if (dev == NULL)
		EXIT_WITH_ERROR("Couldn't find interface by given IP address or name");

	if (!dev->open())
		EXIT_WITH_ERROR("Couldn't open interface");

	// set output file name to interface name if not provided by the user
	std::string outputPath = "./Output_backup/";
	if (outputFileName.empty())
	{
		// take the device name and remove all chars which are not alphanumeric
		outputFileName = std::string(dev->getName());
		outputFileName.erase(remove_if(
			outputFileName.begin(),
			outputFileName.end(),
			isNotAlphanumeric), outputFileName.end());

		outputFileName = outputPath + outputFileName + ".csv";
	}
	else
	{
		size_t fileNameOffset = outputFileName.find_last_of("\\/") + 1;
		size_t extensionOffset = outputFileName.find_last_of(".");
		std::string fileNameWithoutExtension = outputFileName.substr(fileNameOffset, extensionOffset - fileNameOffset);
		outputFileName = outputPath + fileNameWithoutExtension + ".csv";
	}

	// open output file
	std::ofstream outputFile(outputFileName.c_str());
	if (!outputFile)
	{
		EXIT_WITH_ERROR("Cannot open output file '" << outputFileName << "'");
	}

	// write the column headers to the output file
	writeHeaderToOutputFile(outputFile);

	// set BPF filter if provided by the user
	if (!bpfFilter.empty())
	{
		if (!dev->setFilter(bpfFilter))
			EXIT_WITH_ERROR("Error in setting BPF filter to interface");
	}

	std::cout << "Start capturing packets from '" << interfaceNameOrIP << "'..." << std::endl;

	std::map<SessionKey, TLSFingerprint*> stats;
	HandlePacketData data;
	data.outputFile = &outputFile;
	data.stats = &stats;

	// start capturing packets. Each packet arrived will be handled by the onPacketArrives method
	dev->startCapture(onPacketArrives, &data);

	// register the on app close event to print summary stats on app termination
	bool shouldStop = false;
	ApplicationEventHandler::getInstance().onApplicationInterrupted(onApplicationInterrupted, &shouldStop);

	// run in an endless loop until the user press ctrl+c
	while(!shouldStop)
		multiPlatformSleep(1);

	// stop capturing and close the live device
	dev->stopCapture();
	dev->close();

	// printStats(flows);

	std::cout << "Output file was written to: '" << outputFileName << "'" << std::endl;
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


	while((opt = getopt_long(argc, argv, "i:r:o:t:f:s:vhl", TLSFingerprintingOptions, &optionIndex)) != -1)
	{
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
				listInterfaces();
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
	else
	{
		doTlsFingerprintingOnLiveTraffic(interfaceNameOrIP, outputFileName, bpfFilter, modelPath);
	}
}

std::vector<int> countActiveFlowsEveryFiveSeconds(const HandlePacketData* data) {
    if (data == nullptr || data->flows == nullptr) return {};

    // 找出所有流中最早和最晚的时间戳
    long long minStart = LONG_LONG_MAX, maxEnd = LONG_LONG_MIN;
    for (const auto& pair : *data->flows) {
        const Flow* flow = pair.second;
        minStart = std::min(minStart, flow->start_timestamp);
        maxEnd = std::max(maxEnd, flow->latest_timestamp);
    }

    // 计算时间窗口的数量
    int duration = (maxEnd - minStart) / 1e9;
    auto intervals = (duration / 5) + 1;

    // 初始化结果向量
    std::vector<int> activeFlows(intervals, 0);

    // 遍历每个流
    for (const auto& pair : *data->flows) {
        const Flow* flow = pair.second;
        int startInterval = (flow->start_timestamp - minStart) /1e9 / 5;
        int endInterval = (flow->latest_timestamp - minStart) / 1e9 / 5;
        
        // 在流活跃的每个时间窗口内增加计数
        for (int i = startInterval; i <= endInterval; ++i) {
            activeFlows[i]++;
        }
    }
    return activeFlows;
}


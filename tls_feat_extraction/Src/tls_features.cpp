#include "tls_features.h"
#include <string.h>
#include <sstream>
#include <map>
#include <set>
#include<iostream>
using namespace pcpp;

std::set<uint16_t> createGreaseSet() {
	uint16_t greaseExtensions[] = {0x0a0a, 0x1a1a, 0x2a2a, 0x3a3a, 0x4a4a, 0x5a5a, 0x6a6a, 0x7a7a, 0x8a8a, 0x9a9a, 0xaaaa, 0xbaba, 0xcaca, 0xdada, 0xeaea, 0xfafa};
	return std::set<uint16_t>(greaseExtensions, greaseExtensions + 16);
}
static const std::set<uint16_t> GreaseSet = createGreaseSet();

/**
 *	@brief: Hash the feature vector to a digest, with a range of [1, 1024)
 *	@param[in] vec: the feature vector
 *	@return: the digest 
 */
template <typename T>
static uint8_t hashVectorToDigest(const std::vector<T>& vec) {
	uint32_t modFactor = 0x3ff; // 1023
	uint32_t sum = 0;
	for (auto iter = vec.begin(); iter != vec.end(); iter++)
		sum += *iter;
	return (uint8_t)((sum % modFactor) + 1);
}

/* ClientHelloTLSFingerprint */

std::string ClientHelloFingerprint::toString()
{
	std::stringstream tlsFingerprint;

	// add version
	tlsFingerprint << tlsVersion << ",";

	// add cipher suites
	//eg TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
	bool firstCipher = true;
	for (std::vector<uint16_t>::const_iterator iter = cipherSuites.begin(); iter != cipherSuites.end(); iter++)
	{
		tlsFingerprint << (firstCipher ? "" : "-") << *iter;//判断是否第一个
		firstCipher = false;
	}
	tlsFingerprint << ",";

	// add extensions
	//额外交换的信息用于改善性能、增强兼容性和引入新功能
	bool firstExtension = true;
	for (std::vector<uint16_t>::const_iterator iter = extensions.begin(); iter != extensions.end(); iter++)
	{
		tlsFingerprint << (firstExtension ? "" : "-") << *iter;
		firstExtension = false;
	}
	tlsFingerprint << ",";

	// add supported groups
	//特定数学群组
	bool firstGroup = true;
	for (std::vector<uint16_t>::const_iterator iter = supportedGroups.begin(); iter != supportedGroups.end(); iter++)
	{
		tlsFingerprint << (firstGroup ? "" : "-") << (*iter);
		firstGroup = false;
	}
	tlsFingerprint << ",";

	// add EC point formats
	//何表示椭圆曲线上的点，影响着密钥交换数据的大小和表示形式
	bool firstPointFormat = true;
	for (std::vector<uint8_t>::iterator iter = ecPointFormats.begin(); iter != ecPointFormats.end(); iter++)
	{
		tlsFingerprint << (firstPointFormat ? "" : "-") << (int)(*iter);
		firstPointFormat = false;
	}

	return tlsFingerprint.str();
}

ClientDigest ClientHelloFingerprint::toDigest(){
	ClientDigest clientDigest;
	// tlsVersion
	clientDigest.digest[0] = tlsVersion;
	// cipherSuites
	clientDigest.digest[1] = hashVectorToDigest(cipherSuites);
	// extensions
	clientDigest.digest[2] = hashVectorToDigest(extensions);
	// supportedGroups
	clientDigest.digest[3] = hashVectorToDigest(supportedGroups);
	// ecPointFormats
	clientDigest.digest[4] = hashVectorToDigest(ecPointFormats);
	return clientDigest;
}

ClientHelloFingerprint generateClientHelloFingerprint(SSLClientHelloMessage* clientHelloMsg)
{
	ClientHelloFingerprint tlsFingerprint;
	// extract version
	tlsFingerprint.tlsVersion = clientHelloMsg->getHandshakeVersion().asUInt();

	// extract cipher suites
	int cipherSuiteCount = clientHelloMsg->getCipherSuiteCount();

	//std::cout << "cipherSuiteID" << ": ";
	for (int i = 0; i < cipherSuiteCount; i++)
	{
		bool isValid = false;
		uint16_t cipherSuiteID = clientHelloMsg->getCipherSuiteID(i, isValid);//会修改isValid

		if (isValid && GreaseSet.find(cipherSuiteID) == GreaseSet.end()){
			tlsFingerprint.cipherSuites.push_back(cipherSuiteID);
			//std::cout << cipherSuiteID << " ";
		}

	}

	// extract extensions
	int extensionCount = clientHelloMsg->getExtensionCount();
	//std::cout << std::endl << "extension" << ": ";
	for (int i = 0; i < extensionCount; i++)
	{
		uint16_t extensionType = clientHelloMsg->getExtension(i)->getTypeAsInt();
		if (GreaseSet.find(extensionType) != GreaseSet.end())
			continue;

		tlsFingerprint.extensions.push_back(extensionType);
		//std::cout << extensionType << " ";
	}

	// extract supported groups
	TLSSupportedGroupsExtension* supportedGroupsExt = clientHelloMsg->getExtensionOfType<TLSSupportedGroupsExtension>();
	if (supportedGroupsExt != NULL)
	{
		//std::cout << std::endl << "supportedGroups" << ": ";
		std::vector<uint16_t> supportedGroups = supportedGroupsExt->getSupportedGroups();
		for (std::vector<uint16_t>::const_iterator iter = supportedGroups.begin(); iter != supportedGroups.end(); iter++)
			if (GreaseSet.find(*iter) == GreaseSet.end()){
				tlsFingerprint.supportedGroups.push_back(*iter);
				//std::cout << *iter<< " ";
			}
	}
	
	// extract EC point formats
	TLSECPointFormatExtension* ecPointFormatExt = clientHelloMsg->getExtensionOfType<TLSECPointFormatExtension>();
	if (ecPointFormatExt != NULL)
	{
		tlsFingerprint.ecPointFormats = ecPointFormatExt->getECPointFormatList();
	}
	//std::cout << std::endl << "ecPointFormats: ";
	//for(auto t:tlsFingerprint.ecPointFormats)
		//std::cout << int(t) << " ";
	//std::cout << std::endl;

	return tlsFingerprint;
}

/* ServerHelloTLSFingerprint */

std::string ServerHelloFingerprint::toString()
{
	std::stringstream tlsFingerprint;

	// add version and cipher suite
	tlsFingerprint << tlsVersion << "," << cipherSuite << ",";

	// add extensions
	bool firstExtension = true;
	for (std::vector<uint16_t>::const_iterator iter = extensions.begin(); iter != extensions.end(); iter++)
	{
		tlsFingerprint << (firstExtension ? "" : "-") << *iter;
		firstExtension = false;
	}

	return tlsFingerprint.str();
}

ServerDigest ServerHelloFingerprint::toDigest(){
	ServerDigest serverDigest;
	// tlsVersion
	serverDigest.digest[0] = tlsVersion;
	// cipherSuite
	serverDigest.digest[1] = cipherSuite;
	// extensions
	serverDigest.digest[2] = hashVectorToDigest(extensions);
	return serverDigest;
}

ServerHelloFingerprint generateServerHelloFingerprint(SSLServerHelloMessage* serverHelloMsg)
{
	ServerHelloFingerprint tlsFingerprint;

	// extract version
	tlsFingerprint.tlsVersion = serverHelloMsg->getHandshakeVersion().asUInt();

	// extract cipher suite
	bool isValid;
	uint16_t cipherSuite = serverHelloMsg->getCipherSuiteID(isValid);
	tlsFingerprint.cipherSuite = (isValid ? cipherSuite : 0);

	// extract extensions
	int extensionCount = serverHelloMsg->getExtensionCount();
	for (int i = 0; i < extensionCount; i++)
	{
		uint16_t extensionType = serverHelloMsg->getExtension(i)->getTypeAsInt();
		tlsFingerprint.extensions.push_back(extensionType);
	}
	return tlsFingerprint;
}

/* Utils */

SessionKey generateSessionKey(Packet *packet, bool isFromClient) {
	SessionKey sessionKey;
	std::pair<IPAddress, IPAddress> ips = getIPs(packet);
	std::pair<uint16_t, uint16_t> ports = getTcpPorts(packet);
	if (isFromClient) {
		// client to server
		sessionKey.clientIP = ips.first;
		sessionKey.serverIP = ips.second;
		sessionKey.clientPort = ports.first;
		sessionKey.serverPort = ports.second;
	} else {
		// server to client
		sessionKey.clientIP = ips.second;
		sessionKey.serverIP = ips.first;
		sessionKey.clientPort = ports.second;
		sessionKey.serverPort = ports.first;
	}
	return sessionKey;
}
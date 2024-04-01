#pragma once
#include <pcapplusplus/SystemUtils.h>
#include <pcapplusplus/TablePrinter.h>
#include <pcapplusplus/IPLayer.h>
#include <pcapplusplus/TcpLayer.h>
#include <pcapplusplus/SSLLayer.h>
#include <pcapplusplus/SSLHandshake.h>
#include <pcapplusplus/PcapPlusPlusVersion.h>
#include <pcapplusplus/PcapLiveDeviceList.h>
#include <pcapplusplus/PcapFileDevice.h>
#include <set>
// #include "md5.h"

using namespace pcpp;

std::set<uint16_t> createGreaseSet();

struct ClientDigest {
	uint8_t digest[5];
};

/**
 * @struct ClientHelloTLSFingerprint
 * A struct that contains all the elements needed for creating a Client Hello TLS fingerprinting:
 * TLS version, a list of Cipher Suite IDs, a list of Extensions IDs, a list of support groups and a list of
 * EC point formats.
 * This struct contains methods to extract the TLS fingerprint itself: toString() and toMD5()
 */
struct ClientHelloFingerprint
{
	/** TLS version */
	uint16_t tlsVersion;
	/** A list of Cipher Suite IDs */
	std::vector<uint16_t> cipherSuites;
	/** A list of extension IDs */
	std::vector<uint16_t> extensions;
	/** A list of Suppotred Groups taken from the "supported groups" TLS extension (if exist in the message) */
	std::vector<uint16_t> supportedGroups;
	/** A list of EC point formats taken from the "EC point formats" TLS extension (if exist in the message) */
	std::vector<uint8_t> ecPointFormats;

	/**
	 * @return A string representing the TLS fingerprint, for example:
	 * <b>771,4866-4867-4865-255,0-11-10-35-22-23-13-43-45-51,29-23-30-25-24,0-1-2</b>
	 *
	 * This string has the following format: <b>TLSVersion,CipherSuiteIDs,ExtensionIDs,SupportedGroups,ECPointFormats</b>
	 *
	 * The extension IDs, supported groups and EC point formats are each separated by a "-".
	 * If the message doesn't include the "supported groups" or "EC point formats" extensions, they will be replaced
	 * by an empty string for example: <b>771,4866-4867-4865-255,0-11-10-35-22-23-13-43-45-51,,</b>
	 */
	std::string toString();

	/**
	 * @return the hash of the features
	 */
	ClientDigest toDigest();
};



struct ServerDigest {
	uint8_t digest[3];
};

/**
 * @struct ServerHelloTLSFingerprint
 * A struct that contains all the elements needed for creating a Server Hello TLS fingerprinting:
 * TLS version, Cipher Suite ID, and a list of Extensions IDs.
 * You can read more about this in SSLServerHelloMessage#generateTLSFingerprint().
 * This struct contains methods to extract the TLS fingerprint itself: toString() and toMD5()
 */
struct ServerHelloFingerprint
{
	/** TLS version */
	uint16_t tlsVersion;
	/** Cipher Suite ID */
	uint16_t cipherSuite;
	/** A list of extension IDs */
	std::vector<uint16_t> extensions;

	/**
	 * @return A string representing the TLS fingerprint, for example: <b>771,49195,65281-16-11</b>
	 *
	 * This string has the following format: <b>TLSVersion,Cipher,Extensions</b>
	 *
	 * The extension ID are separated with a "-"
	 */
	std::string toString();

	/**
	 * @return the hash of the features
	 */
	ServerDigest toDigest();
};

/**
 * @class TLS session Fingerprints
 */
struct TLSFingerprint {
	ClientHelloFingerprint clientHelloFingerprint;
	ServerHelloFingerprint serverHelloFingerprint;
};

/**
 * @class TLS session key
 */
struct SessionKey {
	// packet's source and dest IP address
	IPAddress clientIP, serverIP;
	uint16_t clientPort, serverPort;

	// override operator < for std::set
	bool operator<(const SessionKey& other) const {
		return clientIP < other.clientIP;
	}
};


/**
 * @brief 生成SessionKey
 * @param packet 数据包
 * @param isFromClient 指示会话的方向
 * @return SessionKey结构体
*/
SessionKey generateSessionKey(Packet *packet, bool isFromClient);

/**
 * @brief 生成客户端的TLSHello指纹
 * @param clientHelloMsg TLS握手消息
 * @param isFromClient 指示会话的方向
 * @return 客户端握手指纹
*/
ClientHelloFingerprint generateClientHelloFingerprint(SSLClientHelloMessage* clientHelloMsg);

/**
 * @brief 生成服务器端的TLSHello指纹
 * @param clientHelloMsg TLS握手消息
 * @param isFromClient 指示会话的方向
 * @return 服务器端握手指纹
*/
ServerHelloFingerprint generateServerHelloFingerprint(SSLServerHelloMessage* serverHelloMsg);

/**
 * @brief 得到一个包的源端口和目的端口
 * @param packet 指向pcpp::Packet类型变量的指针
 * @return 源端口和目的端口对
 */
std::pair<IPAddress, IPAddress> getIPs(const Packet* packet);

/**
 * @brief 获取包的端口号
 * @param packet 指向pcpp::Packet类型变量的指针
 * @return 源端口号和目的端口号的pair
 */
std::pair<uint16_t, uint16_t> getTcpPorts(const Packet* packet);
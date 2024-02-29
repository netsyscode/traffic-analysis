#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <ctime>
#include "feature.h"

// 将流特征写入数据库
void insertFlowFeatureIntoMySQL(const std::vector<std::string>& data, std::unique_ptr<sql::Connection>& con) {
    try {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS feature (app VARCHAR(100), timeStamp DATETIME(6), srcIP VARCHAR(20), dstIP VARCHAR(20),srcPort INT, dstPort INT,\
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
        std::cerr << "SQLException in insertFlowFeatureIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

void insertProtocolInfoIntoMySQL(const std::vector<ProtocolInfo>& data, std::unique_ptr<sql::Connection>& con) {
    try {
        con->setSchema("YourDatabaseName");

        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS ProtocolInfo ("
                      "mac_source VARCHAR(17), "
                      "mac_destination VARCHAR(17), "
                      "ethernet_type SMALLINT, "
                      "vlan_id INT, "
                      "mpls_label INT, "
                      "pppoe_session_id SMALLINT, "
                      "protocol_type VARCHAR(10), "
                      "ip_version VARCHAR(4), "
                      "ip_tos TINYINT, "
                      "ip_header_checksum SMALLINT, "
                      "ip_fragmentation_flag TINYINT, "
                      "ip_identifier SMALLINT, "
                      // Skip ipv6_flow_label as it needs special handling
                      "ipv6_next_header TINYINT, "
                      "wireless_network_ssid VARCHAR(32), "
                      "tcp_header_length SMALLINT, "
                      "tcp_window_size SMALLINT, "
                      "syn_flag BOOLEAN, "
                      "fin_flag BOOLEAN, "
                      "rst_flag BOOLEAN, "
                      "psh_flag BOOLEAN, "
                      "urg_flag BOOLEAN, "
                      "tcp_sequence_number INT, "
                      "tcp_acknowledgement_number INT, "
                      "udp_header_length SMALLINT, "
                      "udp_checksum SMALLINT, "
                      "icmp_type TINYINT, "
                      "icmp_code INT, "
                      "arp_request BOOLEAN, "
                      "arp_reply BOOLEAN, "
                      // Skipping dns_query_type, smtp_command, dhcp_message_type, sip_data for simplicity
                      "PRIMARY KEY(mac_source, mac_destination))");

        // Prepare an SQL statement for inserting data into the table
        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO ProtocolInfo (mac_source, mac_destination, ethernet_type, vlan_id, mpls_label, pppoe_session_id, protocol_type, "
                                          "ip_version, ip_tos, ip_header_checksum, ip_fragmentation_flag, ip_identifier, ipv6_next_header, wireless_network_ssid, tcp_header_length, "
                                          "tcp_window_size, syn_flag, fin_flag, rst_flag, psh_flag, urg_flag, tcp_sequence_number, tcp_acknowledgement_number, udp_header_length, "
                                          "udp_checksum, icmp_type, icmp_code, arp_request, arp_reply) "
                                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        // Bind data to the prepared statement
		for (const auto& info : data) {
			pstmt->setString(1, info.mac_source);
			pstmt->setString(2, info.mac_destination);
			pstmt->setInt(3, info.ethernet_type);
			pstmt->setInt(4, info.vlan_id);
			pstmt->setInt(5, info.mpls_label);
			pstmt->setInt(6, info.pppoe_session_id);
			pstmt->setString(7, info.protocol_type);
			pstmt->setString(8, info.ip_version);
			pstmt->setInt(9, info.ip_tos);
			pstmt->setInt(10, info.ip_header_checksum);
			pstmt->setInt(11, info.ip_fragmentation_flag);
			pstmt->setInt(12, info.ip_identifier);
			// Skipping ipv6_flow_label
			pstmt->setInt(13, info.ipv6_next_header);
			pstmt->setString(14, info.wireless_network_ssid);
			pstmt->setInt(15, info.tcp_header_length);
			pstmt->setInt(16, info.tcp_window_size);
			pstmt->setBoolean(17, info.syn_flag);
			pstmt->setBoolean(18, info.fin_flag);
			pstmt->setBoolean(19, info.rst_flag);
			pstmt->setBoolean(20, info.psh_flag);
			pstmt->setBoolean(21, info.urg_flag);
			pstmt->setInt(22, info.tcp_sequence_number);
			pstmt->setInt(23, info.tcp_acknowledgement_number);
			pstmt->setInt(24, info.udp_header_length);
			pstmt->setInt(25, info.udp_checksum);
			pstmt->setInt(26, info.icmp_type);
			pstmt->setInt(27, info.icmp_code);
			pstmt->setBoolean(28, info.arp_request);
			pstmt->setBoolean(29, info.arp_reply);
			// Execute the prepared statement
			pstmt->executeUpdate();
		}

    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertProtocolInfoIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

void insertSinglePacketInfoIntoMySQL(const std::vector<SinglePacketInfo>& data, std::unique_ptr<sql::Connection>& con) {
    try {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS SinglePacketInfo ("
                      "id INT AUTO_INCREMENT PRIMARY KEY,"
                      "payload_size BIGINT,"
                      "payload_entropy DOUBLE,"
                      "packet_length INT,"
                      "arrival_timestamp VARCHAR(255))");
        
        // Prepare the insert statement
        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO SinglePacketInfo (payload_size, payload_entropy, packet_length, arrival_timestamp) "
                                          "VALUES (?, ?, ?, ?)"));

        // Iterate over each SinglePacketInfo object and insert it into the database
        for (const auto& packet : data) {
            pstmt->setUInt64(1, packet.payload_size); // Using setUInt64 for size_t
            pstmt->setDouble(2, packet.payload_entropy);
            pstmt->setInt(3, packet.packet_length);
            pstmt->setString(4, packet.arrival_timestamp);
            
            pstmt->executeUpdate();
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertSinglePacketInfoIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}
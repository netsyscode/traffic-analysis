#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <ctime>
#include "feature.h"

// 将流特征写入数据库（注意列之间的顺序）
void insertFlowFeatureIntoMySQL(const std::vector<std::string>& data, std::unique_ptr<sql::Connection>& con) {
    try {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());

        //表不存在时会创建新表
        stmt->execute("CREATE TABLE IF NOT EXISTS FlowFeature (app VARCHAR(100), timeStamp DATETIME(6), srcIP VARCHAR(20), dstIP VARCHAR(20),srcPort INT, dstPort INT,\
			clientTlsVersion VARCHAR(10) ,clientCipherSuits VARCHAR(10), clientExtensions VARCHAR(10), clientSupportedGroups VARCHAR(10), clientEcPointFormats VARCHAR(10),\
			serverTlsVersion VARCHAR(10) ,serverCipherSuits VARCHAR(10), serverExtensions VARCHAR(10), appBandwidth DOUBLE, flowDuration DOUBLE,\
			packetCount BIGINT, packetLengthOfFlow BIGINT, packetLengthUnder300 DOUBLE, packetLengthOver1000 DOUBLE, UDPwithoutPayloadRate DOUBLE, RTT DOUBLE, retransferRate DOUBLE)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO FlowFeature (app, timeStamp, srcIP, dstIP, srcPort, dstPort, clientTlsVersion ,clientCipherSuits, clientExtensions, \
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
                      "tcp_window_size SMALLINT UNSIGNED, "
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
                      "arp_reply BOOLEAN "
                      // Skipping dns_query_type, smtp_command, dhcp_message_type, sip_data for simplicity
                      ")ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

        // Prepare an SQL statement for inserting data into the table
        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO ProtocolInfo (mac_source, mac_destination, ethernet_type, vlan_id, mpls_label, pppoe_session_id, protocol_type, "
                                          "ip_version, ip_tos, ip_header_checksum, ip_fragmentation_flag, ip_identifier, ipv6_next_header, wireless_network_ssid, tcp_header_length, "
                                          "tcp_window_size, syn_flag, fin_flag, rst_flag, psh_flag, urg_flag, tcp_sequence_number, tcp_acknowledgement_number, udp_header_length, "
                                          "udp_checksum, icmp_type, icmp_code, arp_request, arp_reply) "
                                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        // Bind data to the prepared statement
		for (const auto& info : data) {
            int i = 1;
			pstmt->setString(i++, info.mac_source);
			pstmt->setString(i++, info.mac_destination);
			pstmt->setUInt(i++, info.ethernet_type);
			pstmt->setInt(i++, info.vlan_id);
			pstmt->setInt(i++, info.mpls_label);
			pstmt->setUInt(i++, info.pppoe_session_id);
			pstmt->setString(i++, info.protocol_type);
			pstmt->setString(i++, info.ip_version);
			pstmt->setInt(i++, info.ip_tos);
			pstmt->setUInt(i++, info.ip_header_checksum);
			pstmt->setUInt(i++, info.ip_fragmentation_flag);
			pstmt->setInt(i++, info.ip_identifier);
			// Skipping ipv6_flow_label
			pstmt->setInt(i++, info.ipv6_next_header);
			pstmt->setString(i++, info.wireless_network_ssid);
			pstmt->setInt(i++, info.tcp_header_length);
			pstmt->setUInt(i++, info.tcp_window_size);
			pstmt->setBoolean(i++, info.syn_flag);
			pstmt->setBoolean(i++, info.fin_flag);
			pstmt->setBoolean(i++, info.rst_flag);
			pstmt->setBoolean(i++, info.psh_flag);
			pstmt->setBoolean(i++, info.urg_flag);
			pstmt->setInt(i++, info.tcp_sequence_number);
			pstmt->setInt(i++, info.tcp_acknowledgement_number);
			pstmt->setInt(i++, info.udp_header_length);
			pstmt->setInt(i++, info.udp_checksum);
			pstmt->setInt(i++, info.icmp_type);
			pstmt->setInt(i++, info.icmp_code);
			pstmt->setBoolean(i++, info.arp_request);
			pstmt->setBoolean(i++, info.arp_reply);
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
            int i = 1;
            pstmt->setUInt64(i++, packet.payload_size); // Using setUInt64 for size_t
            pstmt->setDouble(i++, packet.payload_entropy);
            pstmt->setInt(i++, packet.packet_length);
            pstmt->setString(i++, packet.arrival_timestamp);
            
            pstmt->executeUpdate();
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertSinglePacketInfoIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

void insertPacketsFeatureIntoMySQL(const PacketsFeature& feature, std::unique_ptr<sql::Connection>& con) {
    try {


        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS PacketsFeature ("
                      "avg_packet_size DOUBLE, "
                      "median_packet_size INT, "
                      "max_packet_size INT, "
                      "min_packet_size INT, "
                      "std_packet_size DOUBLE, "
                      "avg_packet_interval DOUBLE, "
                      "median_packet_interval DOUBLE, "
                      "max_interval_between_packets DOUBLE, "
                      "min_interval_between_packets DOUBLE, "
                      "std_packet_interval DOUBLE, "
                      "packet_size_kurtosis DOUBLE, "
                      "packet_size_skewness DOUBLE, "
                      "packet_rate DOUBLE, "
                      "byte_rate DOUBLE, "
                      "syn_ack_time DOUBLE, "
                      "fin_ack_time DOUBLE, "
                      "psh_between_time DOUBLE, "
                      "urg_between_time DOUBLE)"
                      "ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

        // Prepare SQL insert statement
        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO PacketsFeature (avg_packet_size, median_packet_size, max_packet_size, min_packet_size, std_packet_size, "
                                          "avg_packet_interval, median_packet_interval, max_interval_between_packets, min_interval_between_packets, std_packet_interval, "
                                          "packet_size_kurtosis, packet_size_skewness, packet_rate, byte_rate, "
                                          "syn_ack_time, fin_ack_time, psh_between_time, urg_between_time) "
                                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
        int i = 1;
        pstmt->setDouble(i++, feature.avg_packet_size);
        pstmt->setUInt(i++, feature.median_packet_size);
        pstmt->setInt(i++, feature.max_packet_size);
        pstmt->setInt(i++, feature.min_packet_size);
        pstmt->setDouble(i++, feature.std_packet_size);
        pstmt->setDouble(i++, feature.avg_packet_interval);
        pstmt->setDouble(i++, feature.median_packet_interval);
        pstmt->setDouble(i++, feature.max_interval_between_packets);
        pstmt->setDouble(i++, feature.min_interval_between_packets);
        pstmt->setDouble(i++, feature.std_packet_interval);
        pstmt->setDouble(i++, feature.packet_size_kurtosis);
        pstmt->setDouble(i++, feature.packet_size_skewness);
        pstmt->setDouble(i++, feature.packet_rate);
        pstmt->setDouble(i++, feature.byte_rate);
        pstmt->setDouble(i++, feature.syn_ack_time);
        pstmt->setDouble(i++, feature.fin_ack_time);
        pstmt->setDouble(i++, feature.psh_between_time);
        pstmt->setDouble(i++, feature.urg_between_time);

        pstmt->executeUpdate();
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertPacketsFeatureIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

void insertWholeFlowsFeatureIntoMySQL(const WholeFlowsFeature& flowsFeature, std::unique_ptr<sql::Connection>& con) {
    try {
        
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS FlowFeatures ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "max_active_flow_count INT,"
            "flow_duration_10 DOUBLE,"
            "flow_duration_25 DOUBLE,"
            "flow_duration_50 DOUBLE,"
            "flow_duration_75 DOUBLE,"
            "flow_duration_90 DOUBLE,"
            "packet_count_10 DOUBLE,"
            "packet_count_25 DOUBLE,"
            "packet_count_50 DOUBLE,"
            "packet_count_75 DOUBLE,"
            "packet_count_90 DOUBLE,"
            "byte_size_10 DOUBLE,"
            "byte_size_25 DOUBLE,"
            "byte_size_50 DOUBLE,"
            "byte_size_75 DOUBLE,"
            "byte_size_90 DOUBLE,"
            "average_packet_size_10 DOUBLE,"
            "average_packet_size_25 DOUBLE,"
            "average_packet_size_50 DOUBLE,"
            "average_packet_size_75 DOUBLE,"
            "average_packet_size_90 DOUBLE,"
            "avg_ttl_10 DOUBLE,"
            "avg_ttl_25 DOUBLE,"
            "avg_ttl_50 DOUBLE,"
            "avg_ttl_75 DOUBLE,"
            "avg_ttl_90 DOUBLE,"
            "window_size_10 DOUBLE,"
            "window_size_25 DOUBLE,"
            "window_size_50 DOUBLE,"
            "window_size_75 DOUBLE,"
            "window_size_90 DOUBLE,"
            "end_to_end_latency_10 DOUBLE,"
            "end_to_end_latency_25 DOUBLE,"
            "end_to_end_latency_50 DOUBLE,"
            "end_to_end_latency_75 DOUBLE,"
            "end_to_end_latency_90 DOUBLE,"
            "payload_entropy_10 DOUBLE,"
            "payload_entropy_25 DOUBLE,"
            "payload_entropy_50 DOUBLE,"
            "payload_entropy_75 DOUBLE,"
            "payload_entropy_90 DOUBLE"
         ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO FlowFeatures (max_active_flow_count, \
        flow_duration_10, flow_duration_25, flow_duration_50, flow_duration_75, flow_duration_90, \
        packet_count_10, packet_count_25, packet_count_50, packet_count_75, packet_count_90, byte_size_10, \
        byte_size_25, byte_size_50, byte_size_75, byte_size_90, average_packet_size_10, average_packet_size_25, \
        average_packet_size_50, average_packet_size_75, average_packet_size_90, avg_ttl_10, avg_ttl_25, avg_ttl_50, \
        avg_ttl_75, avg_ttl_90, window_size_10, window_size_25, window_size_50, window_size_75, window_size_90, \
        end_to_end_latency_10, end_to_end_latency_25, end_to_end_latency_50, end_to_end_latency_75, end_to_end_latency_90,\
        payload_entropy_10, payload_entropy_25, payload_entropy_50, payload_entropy_75, payload_entropy_90) VALUES (?, ?, ?,\
        ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        pstmt->setInt(1, flowsFeature.max_active_flow_count);
        // 从第二列开始
        int index = 2; 
        for (const auto& distribution : {flowsFeature.flow_duration_distribution, flowsFeature.packet_count_distribution, flowsFeature.byte_size_distribution, 
        flowsFeature.average_packet_size_distribution, flowsFeature.avg_ttl_distribution, flowsFeature.window_size_distribution, flowsFeature.end_to_end_lanteny_distribution, flowsFeature.payload_entropy_distribution}) {
            for (const auto& percentile : distribution) {
                pstmt->setDouble(index++, percentile);
            }
        }
        pstmt->executeUpdate();
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertWholeFlowsFeatureIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}
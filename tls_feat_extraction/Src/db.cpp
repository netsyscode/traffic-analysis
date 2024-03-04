#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <ctime>
#include "feature.h"

void insertProtocolFeatureIntoMySQL(const std::vector<ProtocolInfo>& data, std::unique_ptr<sql::Connection>& con) {
    try {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS ProtocolFeatures ("
                      "id INT AUTO_INCREMENT PRIMARY KEY,"
                      "app_label INT,"
                      "mac_source VARCHAR(17), "
                      "mac_destination VARCHAR(17), "
                      "ethernet_type SMALLINT UNSIGNED, "
                      "vlan_id INT, "
                      "mpls_label INT UNSIGNED, "
                      "pppoe_session_id SMALLINT UNSIGNED, "
                      "protocol_type VARCHAR(10), "
                      "ip_version VARCHAR(4), "
                      "ip_tos TINYINT UNSIGNED, "
                      "ip_header_checksum SMALLINT UNSIGNED, "
                      "ip_fragmentation_flag TINYINT UNSIGNED, "
                      "ip_identifier SMALLINT UNSIGNED, "
                      // Skip ipv6_flow_label as it needs special handling
                      "ipv6_next_header TINYINT UNSIGNED, "
                      "wireless_network_ssid VARCHAR(32), "
                      "tcp_header_length SMALLINT UNSIGNED, "
                      "tcp_window_size SMALLINT UNSIGNED, "
                      "syn_flag BOOLEAN, "
                      "fin_flag BOOLEAN, "
                      "rst_flag BOOLEAN, "
                      "psh_flag BOOLEAN, "
                      "urg_flag BOOLEAN, "
                      "tcp_sequence_number INT UNSIGNED, "
                      "tcp_acknowledgement_number INT UNSIGNED, "
                      "udp_header_length SMALLINT UNSIGNED, "
                      "udp_checksum SMALLINT UNSIGNED, "
                      "icmp_type SMALLINT UNSIGNED, "
                      "icmp_code INT, "
                      "arp_request BOOLEAN, "
                      "arp_reply BOOLEAN "
                      // Skipping dns_query_type, smtp_command, dhcp_message_type, sip_data for simplicity
                      ")ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

        // Prepare an SQL statement for inserting data into the table
        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO ProtocolFeatures (app_label, mac_source, mac_destination, ethernet_type, vlan_id, mpls_label, pppoe_session_id, protocol_type, "
                                          "ip_version, ip_tos, ip_header_checksum, ip_fragmentation_flag, ip_identifier, ipv6_next_header, wireless_network_ssid, tcp_header_length, "
                                          "tcp_window_size, syn_flag, fin_flag, rst_flag, psh_flag, urg_flag, tcp_sequence_number, tcp_acknowledgement_number, udp_header_length, "
                                          "udp_checksum, icmp_type, icmp_code, arp_request, arp_reply) "
                                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        // Bind data to the prepared statement
		for (const auto& info : data) {
            int i = 1;
            pstmt->setInt(i++, info.app_label);
			pstmt->setString(i++, info.mac_source);
			pstmt->setString(i++, info.mac_destination);
			pstmt->setUInt(i++, info.ethernet_type);
			pstmt->setInt(i++, info.vlan_id);
			pstmt->setUInt(i++, info.mpls_label);
			pstmt->setUInt(i++, info.pppoe_session_id);
			pstmt->setString(i++, info.protocol_type);
			pstmt->setString(i++, info.ip_version);
			pstmt->setUInt(i++, info.ip_tos);
			pstmt->setUInt(i++, info.ip_header_checksum);
			pstmt->setUInt(i++, info.ip_fragmentation_flag);
			pstmt->setUInt(i++, info.ip_identifier);
			// Skipping ipv6_flow_label
			pstmt->setUInt(i++, info.ipv6_next_header);
			pstmt->setString(i++, info.wireless_network_ssid);
			pstmt->setUInt(i++, info.tcp_header_length);
			pstmt->setUInt(i++, info.tcp_window_size);
			pstmt->setBoolean(i++, info.syn_flag);
			pstmt->setBoolean(i++, info.fin_flag);
			pstmt->setBoolean(i++, info.rst_flag);
			pstmt->setBoolean(i++, info.psh_flag);
			pstmt->setBoolean(i++, info.urg_flag);
			pstmt->setUInt(i++, info.tcp_sequence_number);
			pstmt->setUInt(i++, info.tcp_acknowledgement_number);
			pstmt->setUInt(i++, info.udp_header_length);
			pstmt->setUInt(i++, info.udp_checksum);
			pstmt->setUInt(i++, info.icmp_type);
			pstmt->setInt(i++, info.icmp_code);
			pstmt->setBoolean(i++, info.arp_request);
			pstmt->setBoolean(i++, info.arp_reply);
			// Execute the prepared statement
			pstmt->executeUpdate();
		}
        std::cout << "Insert ProtocolFeatures Into MySQL Successfully!\n";
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertProtocolFeatureIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

void insertPacketFeatureIntoMySQL(const std::vector<SinglePacketInfo>& data, std::unique_ptr<sql::Connection>& con) {
    try {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS PacketFeatures ("
                      "id INT AUTO_INCREMENT PRIMARY KEY,"
                      "app_label INT,"
                      "payload_size BIGINT,"
                      "payload_entropy DOUBLE,"
                      "packet_length INT,"
                      "arrival_timestamp VARCHAR(255))");
        
        // Prepare the insert statement
        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO PacketFeatures (app_label, payload_size, payload_entropy, packet_length, arrival_timestamp) "
                                          "VALUES (?, ?, ?, ?, ?)"));

        // Iterate over each SinglePacketInfo object and insert it into the database
        for (const auto& packet : data) {
            int i = 1;
            pstmt->setInt(i++, packet.app_label);
            pstmt->setUInt64(i++, packet.payload_size); // Using setUInt64 for size_t
            pstmt->setDouble(i++, packet.payload_entropy);
            pstmt->setInt(i++, packet.packet_length);
            pstmt->setString(i++, packet.arrival_timestamp);
            
            pstmt->executeUpdate();
        }
        std::cout << "Insert PacketFeatures Into MySQL Successfully!\n";
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertPacketFeatureIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

void insertPacketsFeatureIntoMySQL(const std::vector<PacketsFeature>& features, std::unique_ptr<sql::Connection>& con) {
    try {


        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS PacketsFeatures ("
                      "id INT AUTO_INCREMENT PRIMARY KEY,"
                      "app_label INT,"
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
        pstmt.reset(con->prepareStatement("INSERT INTO PacketsFeatures (app_label, avg_packet_size, median_packet_size, max_packet_size, min_packet_size, std_packet_size, "
                                          "avg_packet_interval, median_packet_interval, max_interval_between_packets, min_interval_between_packets, std_packet_interval, "
                                          "packet_size_kurtosis, packet_size_skewness, packet_rate, byte_rate, "
                                          "syn_ack_time, fin_ack_time, psh_between_time, urg_between_time) "
                                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
        for(auto feature: features){                                 
            int i = 1;
            pstmt->setInt(i++, feature.app_label);
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
        }

        std::cout << "Insert PacketsFeatures Into MySQL Successfully!\n";
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertPacketsFeatureIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

void insertFlowsFeatureIntoMySQL(const FlowsFeature& flowsFeature, std::unique_ptr<sql::Connection>& con) {
    try { 
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS FlowsFeatures ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "app_label INT,"
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
        pstmt.reset(con->prepareStatement("INSERT INTO FlowsFeatures (app_label, max_active_flow_count, \
        flow_duration_10, flow_duration_25, flow_duration_50, flow_duration_75, flow_duration_90, \
        packet_count_10, packet_count_25, packet_count_50, packet_count_75, packet_count_90, byte_size_10, \
        byte_size_25, byte_size_50, byte_size_75, byte_size_90, average_packet_size_10, average_packet_size_25, \
        average_packet_size_50, average_packet_size_75, average_packet_size_90, avg_ttl_10, avg_ttl_25, avg_ttl_50, \
        avg_ttl_75, avg_ttl_90, window_size_10, window_size_25, window_size_50, window_size_75, window_size_90, \
        end_to_end_latency_10, end_to_end_latency_25, end_to_end_latency_50, end_to_end_latency_75, end_to_end_latency_90,\
        payload_entropy_10, payload_entropy_25, payload_entropy_50, payload_entropy_75, payload_entropy_90) VALUES (?, ?, ?,\
        ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        //只插入一行
        pstmt->setInt(1, flowsFeature.max_active_flow_count);
        pstmt->setInt(2, flowsFeature.app_label);
        // 从第三列开始
        int index = 3; 
        for (const auto& distribution : {flowsFeature.flow_duration_distribution, flowsFeature.packet_count_distribution, flowsFeature.byte_size_distribution, 
        flowsFeature.average_packet_size_distribution, flowsFeature.avg_ttl_distribution, flowsFeature.window_size_distribution, flowsFeature.end_to_end_lanteny_distribution, flowsFeature.payload_entropy_distribution}) {
            for (const auto& percentile : distribution) {
                pstmt->setDouble(index++, percentile);
            }
        }
        pstmt->executeUpdate();
        std::cout << "Insert FlowsFeatures Into MySQL Successfully!\n";
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertFlowsFeatureIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}

void insertFlowFeatureIntoMySQL(const std::vector<FlowFeature>& features, std::unique_ptr<sql::Connection>& con) {
    try {
        const char* createTableSQL = R"SQL(
            CREATE TABLE IF NOT EXISTS FlowFeatures (
                id INT AUTO_INCREMENT PRIMARY KEY,
                app_label INT,
                srcIP VARCHAR(255),
                dstIP VARCHAR(255),
                srcPort SMALLINT UNSIGNED,
                dstPort SMALLINT UNSIGNED,
                startts VARCHAR(255),
                pktcnt INT,
                client_TLS_version VARCHAR(255),
                client_cipher_suite VARCHAR(255),
                client_extensions VARCHAR(255),
                client_supported_groups VARCHAR(255),
                client_ecformat VARCHAR(255),
                server_TLS_version VARCHAR(255),
                server_cipher_suite VARCHAR(255),
                server_extensions VARCHAR(255),
                pktlen DOUBLE,
                itvl DOUBLE,
                bw DOUBLE,
                dur DOUBLE,
                ave_pkt_size_over_1000 DOUBLE,
                ave_pkt_size_under_300 DOUBLE,
                udp_nopayload_rate DOUBLE,
                ret_rate DOUBLE,
                ave_rtt DOUBLE,
                bytes_of_flow BIGINT,
                bytes_of_payload BIGINT,
                header_of_packets BIGINT,
                bytes_of_ret_packets BIGINT,
                count_of_TCPpackets INT,
                count_of_UDPpackets INT,
                count_of_ICMPpackets INT,
                max_size_of_packet INT,
                min_size_of_packet INT,
                end_to_end_latency DOUBLE,
                avg_window_size DOUBLE,
                avg_ttl DOUBLE,
                avg_payload_size DOUBLE,
                count_of_ret_packets INT,
                count_of_syn_packets INT,
                count_of_fin_packets INT,
                count_of_rst_packets INT,
                count_of_ack_packets INT,
                count_of_psh_packets INT,
                count_of_urg_packets INT,
                entropy_of_payload DOUBLE
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
            )SQL";
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute(createTableSQL);

        std::unique_ptr<sql::PreparedStatement> pstmt;
        pstmt.reset(con->prepareStatement("INSERT INTO FlowFeatures (app_label, srcIP, dstIP, srcPort, dstPort, startts, pktcnt, "
                                          "client_TLS_version, client_cipher_suite, client_extensions, client_supported_groups, client_ecformat, "
                                          "server_TLS_version, server_cipher_suite, server_extensions, pktlen, itvl, bw, dur, "
                                          "ave_pkt_size_over_1000, ave_pkt_size_under_300, udp_nopayload_rate, ret_rate, ave_rtt, "
                                          "bytes_of_flow, bytes_of_payload, header_of_packets, bytes_of_ret_packets, "
                                          "count_of_TCPpackets, count_of_UDPpackets, count_of_ICMPpackets, "
                                          "max_size_of_packet, min_size_of_packet, end_to_end_latency, avg_window_size, avg_ttl, avg_payload_size, "
                                          "count_of_ret_packets, count_of_syn_packets, count_of_fin_packets, count_of_rst_packets, count_of_ack_packets, count_of_psh_packets, count_of_urg_packets, "
                                          "entropy_of_payload) "
                                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        for(const auto& feature: features) {
            int i = 1;
            pstmt->setInt(i++, feature.app_label);
            pstmt->setString(i++, feature.srcIP);
            pstmt->setString(i++, feature.dstIP);
            pstmt->setUInt(i++, feature.srcPort);
            pstmt->setUInt(i++, feature.dstPort);
            pstmt->setString(i++, feature.startts);
            pstmt->setInt(i++, feature.pktcnt);
            pstmt->setString(i++, feature.client_TLS_version);
            pstmt->setString(i++, feature.client_cipher_suite);
            pstmt->setString(i++, feature.client_extensions);
            pstmt->setString(i++, feature.client_supported_groups);
            pstmt->setString(i++, feature.client_ecformat);
            pstmt->setString(i++, feature.server_TLS_version);
            pstmt->setString(i++, feature.server_cipher_suite);
            pstmt->setString(i++, feature.server_extensions);
            pstmt->setDouble(i++, feature.pktlen);
            pstmt->setDouble(i++, feature.itvl);
            pstmt->setDouble(i++, feature.bw);
            pstmt->setDouble(i++, feature.dur);
            pstmt->setDouble(i++, feature.ave_pkt_size_over_1000);
            pstmt->setDouble(i++, feature.ave_pkt_size_under_300);
            pstmt->setDouble(i++, feature.udp_nopayload_rate);
            pstmt->setDouble(i++, feature.ret_rate);
            pstmt->setDouble(i++, feature.ave_rtt);
            pstmt->setBigInt(i++, std::to_string(feature.bytes_of_flow));
            pstmt->setBigInt(i++, std::to_string(feature.bytes_of_payload));
            pstmt->setBigInt(i++, std::to_string(feature.header_of_packets));
            pstmt->setBigInt(i++, std::to_string(feature.bytes_of_ret_packets));
            pstmt->setInt(i++, feature.count_of_TCPpackets);
            pstmt->setInt(i++, feature.count_of_UDPpackets);
            pstmt->setInt(i++, feature.count_of_ICMPpackets);
            pstmt->setInt(i++, feature.max_size_of_packet);
            pstmt->setInt(i++, feature.min_size_of_packet);
            pstmt->setDouble(i++, feature.end_to_end_latency);
            pstmt->setDouble(i++, feature.avg_window_size);
            pstmt->setDouble(i++, feature.avg_ttl);
            pstmt->setDouble(i++, feature.avg_payload_size);
            pstmt->setInt(i++, feature.count_of_ret_packets);
            pstmt->setInt(i++, feature.count_of_syn_packets);
            pstmt->setInt(i++, feature.count_of_fin_packets);
            pstmt->setInt(i++, feature.count_of_rst_packets);
            pstmt->setInt(i++, feature.count_of_ack_packets);
            pstmt->setInt(i++, feature.count_of_psh_packets);
            pstmt->setInt(i++, feature.count_of_urg_packets);
            pstmt->setDouble(i++, feature.entropy_of_payload);

            pstmt->executeUpdate();
        }
        std::cout << "Insert FlowFeatures Into MySQL Successfully!\n";
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException in insertFlowFeatureIntoMySQL():" << std::endl
                  << "SQLState: " << e.getSQLState() << std::endl
                  << "Error Code: " << e.getErrorCode() << std::endl
                  << "Message: " << e.what() << std::endl;
    }
}
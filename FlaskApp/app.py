from flask import Flask, jsonify, render_template
import mysql.connector
from mysql.connector import Error
import datetime
from datetime import datetime as dt
import json

app = Flask(__name__)

def query_database(query, params=()):
    try:
        with open('dbconfig.json', 'r') as file:
            config = json.load(file)
        connection = mysql.connector.connect(**config)
        
        cursor = connection.cursor()
        cursor.execute(query, params) # 执行查询语句 
        result = cursor.fetchall()
        return result
    except Error as e:
        print("Error while connecting to MySQL", e)
        return []
    finally:
        if connection.is_connected():
            cursor.close()
            connection.close()

def get_earliest_timestamp():
    query = "SELECT MIN(startts) AS earliestTimeStamp FROM FlowFeatures;"
    results = query_database(query)
    if results:
        return results[0][0]
    else:
        return None


def get_top10_data(start_time_str):
    # 将传入的字符串转换为datetime对象
    start_time = dt.strptime(start_time_str, '%Y-%m-%d %H:%M:%S.%f')
    end_time = start_time + datetime.timedelta(seconds=108000)
    
    task1_query = """
    SELECT app_label, 
    SUM(bytes_of_flow) AS totalBytes, 
    SUM(pktcnt),
    COUNT(DISTINCT srcIP) AS uniqueSrcIPs, 
    COUNT(DISTINCT dstIP) AS uniqueDstIPs
    FROM FlowFeatures
    WHERE startts BETWEEN %s AND %s 
    GROUP BY app_label
    ORDER BY totalBytes DESC
    LIMIT 10;
    """


    task2_query = """
    SELECT SUM(bytes_of_flow) AS cumulativeLength
    FROM FlowFeatures
    WHERE startts <= %s
    ORDER BY startts ASC;
    """

    task3_query = """
    SELECT client_TLS_version,
    SUM(CASE WHEN client_TLS_version = 3 THEN 1 ELSE 0 END) AS TLS_Count,
    SUM(CASE WHEN client_TLS_version != 3 THEN 1 ELSE 0 END) AS NonTLS_Count
    FROM FlowFeatures
    WHERE startts <= %s
    GROUP BY client_TLS_version;
    """
    
    query1_data = query_database(task1_query, (start_time, end_time))
    query2_data = query_database(task2_query, (end_time,))
    query3_data = query_database(task3_query, (end_time,))
    return jsonify(query1_data, query2_data, query3_data)

@app.route('/get_top10_and_total_traffic')
def get_top10_and_total_traffic():
    earliest_timestamp = get_earliest_timestamp() # 返回一个字符串
    if earliest_timestamp:
        global current_time
        if 'current_time' not in globals():
            # 第一次调用，直接使用earliest_timestamp
            current_time = earliest_timestamp
        else:
            # 将字符串转换为datetime对象，然后加上36000秒
            current_dt = dt.strptime(current_time, '%Y-%m-%d %H:%M:%S.%f')
            current_dt += datetime.timedelta(seconds=6000)
            # 再次将datetime对象转换回字符串，以便传递
            current_time = current_dt.strftime('%Y-%m-%d %H:%M:%S.%f')
            top = get_top10_data(current_time)
            # encry = get_encrypted_ratio(top[0][4:])

            # return jsonify(top, encry)
            return top
    else:
        return jsonify({"error": "No data available"})


# def get_encrypted_ratio(start_time_str, apps):
#     # 将传入的字符串转换为datetime对象
#     start_time = dt.strptime(start_time_str, '%Y-%m-%d %H:%M:%S.%f')
#     end_time = start_time + datetime.timedelta(seconds=108000)
    
#     query = """
#     SELECT 
#     app_label,
#     SUM(CASE WHEN payload_size > 0 AND payload_entropy > 7 THEN payload_size ELSE 0 END) AS encrypted_bytes,
#     SUM(CASE WHEN payload_size > 0 THEN payload_size ELSE 0 END) AS total_bytes,
#     (SUM(CASE WHEN payload_size > 0 AND payload_entropy > 7 THEN payload_size ELSE 0 END) * 1.0 / SUM(CASE WHEN payload_size > 0 THEN payload_size ELSE 0 END)) AS encryption_ratio
#     FROM PacketFeatures AND arrival_timestamp BETWEEN %s AND %s 
#     WHERE app_label = %s -- 使用参数化查询
#     """

#     data = [query_database(query, (app, start_time, end_time))[3] for app in apps]
#     return data

@app.route('/')
def index():
    return render_template('main.html')

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)

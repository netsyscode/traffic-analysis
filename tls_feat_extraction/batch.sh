#!/bin/bash

#!/bin/bash

folder="../MobileTraffic_Dataset/data1/traffic"

# 获取所有用户目录
user_dirs=$(find "$folder" -mindepth 1 -maxdepth 1 -type d)

last_user_file="../MobileTraffic_Dataset/data1/last_user.txt"

if [ -f "$last_user_file" ]; then
    last_user=$(cat "$last_user_file")
    new_process=false
else
    new_process=true
fi

for user_dir in $user_dirs; do
    if [ "$new_process" = true ]; then
        echo "Processing $user_dir"
        user_label=$(basename "$user_dir")
        find "$user_dir" -mindepth 1 -maxdepth 1 -type d | while IFS= read -r date_dir; do
            echo "Processing files in $date_dir"
            # 遍历并处理每个.pcap文件
            find "$date_dir" -type f -name "*.pcap" | while IFS= read -r file_path; do
                if [[ "$file_path" != *_ipv6.pcap ]]; then
                    ./Bin/TLSFingerprinting -r "$file_path" -l "$user_label" 
                fi
            done
        done
        echo "$user_dir" > "$last_user_file"
        break # 处理完一个用户后退出循环
    fi
    if [ "$user_dir" = "$last_user" ]; then
        new_process=true
    fi
done

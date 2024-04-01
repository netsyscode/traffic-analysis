#!/bin/bash

folder1="../dataset/android"
app_label=0 # 初始化app_label

files=$(ls $folder1)
for file in $files;
do
    let app_label=app_label+1 # 每次循环增加app_label的值
    file_path=$folder1"/"$file
    ./Bin/TLSFingerprinting -r $file_path -l $app_label # 假设您的命令可以接受-l参数来使用app_label
done


"""
build dataset from ./dataset/**
default label: 0
"""

import numpy as np
import pandas as pd
import os
import re
import random
from datetime import datetime

# read all csv files from ./tls_feat_extraction/Output/
dir_path = "./tls_feat_extraction/Output"
target_regex = ".*bili.*" 

# Compile the regular expression
pattern = re.compile(target_regex)

# Initialize lists to hold filenames
files_with_target = []
files_without_target = []

# Iterate over all files in the directory
# 训练不需要很多的负例，所以随机选取40种流量
file_list = os.listdir(dir_path)
random.shuffle(file_list )
n = 40

for filename in file_list:
    # Check if the target regex matches the filename
    if pattern.search(filename):
        files_with_target.append(filename)
    elif n > 0:
        files_without_target.append(filename)
        n = n  -1

now = datetime.now()
formatted_now = now.strftime("%Y-%m-%d %H:%M:%S")
log = formatted_now + "\n"
print("Positive:",files_with_target)
log += "Positive:" + str(files_with_target) + "\n"
print("Negative:",files_without_target)
log += "Negative:" + str(files_without_target) + "\n"
total_data = np.ndarray((0, 25)) # combine and output dataset.csv

# read csv file from file_path
def read_csv(file_path):
    df = pd.read_csv(file_path, sep="\t", header=0)
    df = df.drop(columns=df.columns[[0, 1]])
    return df.values

for path in files_with_target:
    # check if current path is a file
    file_path = os.path.join(dir_path, path)
    if os.path.isfile(file_path):
        values = read_csv(file_path)
        ones = np.ones((values.shape[0], 1))
        values = np.hstack((values, ones))
        total_data = np.append(total_data, values, axis=0)
        

for path in files_without_target:
    # check if current path is a file
    file_path = os.path.join(dir_path, path)
    if os.path.isfile(file_path):
        values = read_csv(file_path)
        ones = np.zeros((values.shape[0], 1))
        values = np.hstack((values, ones))
        total_data = np.append(total_data, values, axis=0)

# output the total data to csv file
np.savetxt("dataset/dataset.csv", total_data, delimiter=",")
with open('dataset/log.txt', 'w') as f:
    f.write(log)

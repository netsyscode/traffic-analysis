import os
import pandas as pd
from collections import Counter
import numpy as np
import matplotlib.pyplot as plt
from imblearn.over_sampling import RandomOverSampler
from imblearn.under_sampling import RandomUnderSampler
from sklearn import svm
from sklearn.utils.class_weight import compute_class_weight
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, confusion_matrix, precision_score, recall_score, f1_score, cohen_kappa_score
from sklearn.compose import ColumnTransformer
from sklearn.preprocessing import OneHotEncoder
from sklearn.multiclass import OneVsRestClassifier
import json
import csv
import re
from random import shuffle

# 读取CSV文件,生成对应英文对应的中文
try:
    df = pd.read_csv('./result_utf8.csv')
    # 生成字典，第一列作为键，第二列作为值
    dictionary = {str(key).strip(): str(value).strip() for key, value in zip(df.iloc[0:, 0], df.iloc[0:, 1])}
    print("Application of English Name to Chinese Name Dictionary Created Successfully")
except Exception as e:
    print("Error:", e)

# 假设你有一个包含文件夹路径的列表
folders = ['dataset/Merge']

# 创建一个空的列表来记录没有数据的 CSV 文件
empty_files = []
total_data = np.ndarray((0, 25))
label = 1

label2app = {}

for folder in folders:
    dir = os.listdir(folder)
    shuffle(dir)
    for filename in dir:
        # 设置数量
        if filename.endswith('.csv')  and label < 21:
            filepath = os.path.join(folder, filename)
            # 试着读取文件中的数据，假设每个文件至少有3列
            try:
                data = np.genfromtxt(filepath, delimiter='\t', usecols=range(2, 26)) 
                # 如果 CSV 文件为空，将其路径添加到 empty_files 列表中
                if data.size == 24:
                    empty_files.append(filepath)
                else:
                    ones = np.ones((data.shape[0], 1)) * label
                    filename = re.sub(r'(\_merged)|(\.csv)', '', filename)
                    label2app[label] = filename# 名称对应
                    label = label + 1
                    data = np.hstack((data, ones))
                    total_data = np.append(total_data, data, axis=0)
            except IndexError:
                print(f"File {filepath} doesn't have 3 or more columns")
                continue
            
print("Application of Label to English Name Dictionary Created Successfully")
Xy = np.nan_to_num(total_data)

print("Total csv:",label - 1)
#print(label2app)
#print("Empty csv:",len(empty_files))

# One-hot编码：对象是常见端口号和TLS

# 非常见端口
# 创建 ColumnTransformer 对象,同时设置handle_unknown='ignore'，使得识别未知端口
column_transformer = ColumnTransformer(
    transformers=[
        ('encoder', OneHotEncoder(handle_unknown='ignore'), list(range(2, 12))) 
    ],
    remainder='passthrough'  # 不转换的列保持原状
)

# 应用转换
#Xy = column_transformer.fit_transform(Xy).toarray()

X = Xy[:, 0:-1]
Y = Xy[:, -1]

x_train, x_test, y_train, y_test = train_test_split(
    X, Y,
    test_size=0.2,
    random_state=42
)

labels = np.unique(Y)

#为代表较少样本的类分配较高的权重来解决分类问题中不平衡类分布的问题
weight = compute_class_weight(class_weight='balanced', classes=labels, y=Y)
weight_dict = dict(zip(labels, weight))

#print(weight_dict )

# normalize the X data
x_mean = np.mean(x_train, axis=0)
x_std = np.std(x_train, axis=0)
standard_x_train = (x_train - x_mean) / x_std
standard_x_test = (x_test - x_mean) / x_std
standard_x_train = np.nan_to_num(standard_x_train)
standard_x_test = np.nan_to_num(standard_x_test)


# fit the model
# 加入信心值
clf = OneVsRestClassifier(svm.SVC(kernel='rbf'))
clf.fit(standard_x_train, y_train)

# test the model, print the metrics
y_predicted = clf.predict(standard_x_test)

# print('yp:_length',len(y_predicted), len(y_test))
# print('y_predict:', y_predicted)
# print('y_test:', y_test)

count = {}#key为标签
for num in y_test:
    if num in count:
        count[num] += 1
    else:
        count[num] = 1
    
err = {}#key为标签
tot = list(zip(y_test, y_predicted))
for values in tot:
    yt, yp = values[0], values[1]
    if yt != yp:
        if yt in err:
            err[yt] += 1
        else:
            err[yt] = 1
# print(dict(sorted(count.items())))
# print(dict(sorted(err.items())))

accu = {} #key为标签
for i in range(1, label):
    if i in count:
        if count[i] != 0:
            if i not in err:
                err[i] = 0
            accu[i] = (count[i] - err[i]) / count[i]
                
    
print('app_accuracy:')

for item in accu:
    print(dictionary[label2app[item]], accu[item])

# 指定CSV文件的路径
csv_file_path = './accuracy.csv'

# 将字典按列写入CSV文件
with open(csv_file_path, 'w', newline='', encoding='utf-8') as csv_file:
    # 创建CSV写入器
    csv_writer = csv.writer(csv_file)
    # 写入列名
    csv_writer.writerow(['应用', '准确率'])
    # 写入字典数据
    for key, value in accu.items():
        csv_writer.writerow([dictionary[label2app[key]], value])


# acc =  accuracy_score(y_test, y_predicted)
# print("train_accuracy: ", acc)
# pre = precision_score(y_test, y_predicted,average='macro')
# print("precision: ", pre)
# rec = recall_score(y_test, y_predicted,average='macro')
# print("recall: ", rec)
# f1 = f1_score(y_test, y_predicted,average='macro')
# print("f1: ", f1)
# kappa = cohen_kappa_score(y_test, y_predicted)
# print("kappa: ", kappa)


plt.figure()
C = confusion_matrix(y_test, y_predicted, labels=labels)
plt.matshow(C, cmap=plt.cm.Reds) # 根据最下面的图按自己需求更改颜色

for i in range(len(C)):
    for j in range(len(C)):
        plt.annotate(C[j, i], xy=(i, j), horizontalalignment='center', verticalalignment='center')

# plt.ylabel('True label')
# plt.xlabel('Predicted label')
# plt.savefig("./conf_mat_ovr.jpg", dpi=300)

# 保存模型
# 提取模型参数
# print('support_vectors:', [[len(est.support_vectors_),len(est.support_vectors_[0])] for est in clf.estimators_])
# print('dual_coef:', [len(est.dual_coef_[0]) for est in clf.estimators_])
# print('intercept:', [len(est.intercept_) for est in clf.estimators_])
# print(x_mean.tolist())
# print(x_std.tolist())
# 提取模型参数
model_params = {
    'support_vectors': [est.support_vectors_.tolist() for est in clf.estimators_],
    'dual_coef': [est.dual_coef_.tolist() for est in clf.estimators_],
    'intercept': [float(est.intercept_) for est in clf.estimators_],
    'mean': x_mean.tolist(),
    'std': x_std.tolist()
}
# 将参数保存到 JSON 文件
with open('model/ovr_params.json', 'w') as f:
    json.dump(model_params, f)
    

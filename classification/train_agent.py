"""
Support Vector Machine: SVM base classifier
Each app should implement this classifier
"""

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
import json

#load data from ./dataset/dataset.csv
Xy = np.genfromtxt("./dataset/dataset.csv", delimiter=',', filling_values=0)
Xy = np.nan_to_num(Xy)

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

X = Xy[:, 0:-1] #except for the last column
Y = Xy[:, -1] #the last column

x_train, x_test, y_train, y_test = train_test_split(
    X, Y,
    test_size=0.2,
    random_state=42
)

labels = np.unique(Y)

weight = compute_class_weight(class_weight='balanced', classes=labels, y=Y)
weight_dict = dict(zip(labels, weight))

# normalize the X data
x_mean = np.mean(x_train, axis=0)
x_std = np.std(x_train, axis=0)
standard_x_train = (x_train - x_mean) / x_std
standard_x_test = (x_test - x_mean) / x_std
standard_x_train = np.nan_to_num(standard_x_train)
standard_x_test = np.nan_to_num(standard_x_test)

# oversampling
ros = RandomUnderSampler(random_state=0)
x_resampled, y_resampled = ros.fit_resample(x_train, y_train)
print(sorted(Counter(y_resampled).items()))

# fit the model
# 加入信心值
clf = svm.SVC(kernel='rbf', class_weight=weight_dict, decision_function_shape='ovo')
clf.fit(standard_x_train, y_train)

# test the model, print the metrics
y_predicted = clf.predict(standard_x_test)
acc =  accuracy_score(y_test, y_predicted)
pre = precision_score(y_test, y_predicted)
rec = recall_score(y_test, y_predicted)
f1 = f1_score(y_test, y_predicted)
kappa = cohen_kappa_score(y_test, y_predicted)
print("accuracy: ", acc)
print("precision: ", pre)
print("recall: ", rec)
print("f1: ", f1)
print("kappa: ", kappa)


plt.figure()
C = confusion_matrix(y_test, y_predicted, labels=labels)
plt.matshow(C, cmap=plt.cm.Reds) # 根据最下面的图按自己需求更改颜色

for i in range(len(C)):
    for j in range(len(C)):
        plt.annotate(C[j, i], xy=(i, j), horizontalalignment='center', verticalalignment='center')

plt.ylabel('True label')
plt.xlabel('Predicted label')
plt.savefig("./conf_mat.jpg", dpi=300)

# 保存模型
# 提取模型参数
model_params = {
    'support_vectors': clf.support_vectors_.tolist(),
    'dual_coef': clf.dual_coef_.tolist(),
    'intercept': clf.intercept_.tolist(),
}

# 将参数保存到 JSON 文件
with open('svm_params.json', 'w') as f:
    json.dump(model_params, f)
    
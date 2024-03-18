function calculateAppDataProportions(jsonData) {
  //将app_label映射为app名称
  const appLabelsChinese = jsonData.map(([appLabel]) => num2appname[appLabel.toString()]); // 使用箭头函数
  // 将总字节数转换为MB
  const totalBytesMB = jsonData.map(([, totalBytes]) => Math.floor(Number(totalBytes) / 1048576));
  const totalBytesSum = totalBytesMB.reduce((acc, cur) => acc + cur, 0);
  // 计算各个应用的流量占比
  const proportions = totalBytesMB.map(totalBytes => totalBytesSum > 0 ? Math.round((totalBytes / totalBytesSum) * 10000) / 100 : 0);

  return [appLabelsChinese.reverse(), totalBytesMB.reverse(), proportions.reverse()];
}

function populateData(data) {
  const container = document.querySelector('.data-container');

  // 检查并移除已存在的表格
  const existingTable = container.querySelector('.data-table');
  if (existingTable) {
    container.removeChild(existingTable);
  }
  // 创建表格元素
  const table = document.createElement('table');
  table.className = 'data-table';

  // 创建并添加表头
  const thead = document.createElement('thead');
  const headerRow = document.createElement('tr');
  const headers = ['应用名称', '总字节数(MB)', '占总流量比例', '加密流量占比','包数', '源IP数', '目的IP数'];
  headers.forEach(headerText => {
    const header = document.createElement('th');
    header.textContent = headerText;
    headerRow.appendChild(header);
  });
  thead.appendChild(headerRow);
  table.appendChild(thead);

  // 创建并添加数据行
  const tbody = document.createElement('tbody');
  data.slice(0, 6).forEach(item => {
    const dataRow = document.createElement('tr');
    
    dataRow.innerHTML = `
      <td>${item.appName}</td>
      <td>${item.totalBytes }</td>
      <td>${item.percentage}%</td>
      <td>80%</td>
      <td>${item.pktcnt}</td>
      <td>${item.sourceIPs}</td>
      <td>${item.destIPs}</td>
    `;
    tbody.appendChild(dataRow);
  });
  table.appendChild(tbody);

  // 将表格添加到容器
  container.appendChild(table);
}

function transformData(jsonData) {
  const totalBytesSum = jsonData.reduce((sum, item) => sum + parseInt(item[1]), 0);

  // 转换数据格式
  const finalData = jsonData.map(item => ({
    appName:  num2appname[item[0].toString()], // appName不变，转换为字符串以确保一致性
    totalBytes: parseFloat(parseInt(item[1]) / 1024 / 1024).toFixed(4), 
    percentage: parseFloat(((parseInt(item[1]) / totalBytesSum) * 100).toFixed(2)), // 计算流量百分比，并保留两位小数
    pktcnt: parseInt(item[2]), // 包数
    sourceIPs: item[3], // 源IP数
    destIPs: item[4] // 目的IP数
  }));
  return finalData;
}

function sumEncryptionCount(arr) {
  let TLS_count = 0;
  let NonTLS_count = 0;
  
  // 遍历数组的每个内部数组
  arr.forEach(innerArr => {
    TLS_count += parseInt(innerArr[1], 10);
    NonTLS_count += parseInt(innerArr[2], 10);
  });
  
  // 返回包含两个累加结果的数组
  return [TLS_count, NonTLS_count];
}

async function fetchDataAndProcessConcurrently() {
  try {
    fetch('get_top10_and_total_traffic')
      .then(response => response.json())
      .then(data => {
        // top10流量应用信息
        top10_data = data[0] //  Array(5)
        top10_rank = calculateAppDataProportions(top10_data)
        echarts_4(top10_rank[0], top10_rank[1], top10_rank[2])
        
        //流量累计值
        total_traffic = data[1]
        var totalFormatted = (total_traffic / 1024 / 1024 / 1024).toPrecision(5);
        document.getElementById("totalTraffic").innerText = totalFormatted;

        // 加密套件占比
        encry_suite = sumEncryptionCount(data[2])
        console.log(encry_suite)
        echarts6(encry_suite[0], encry_suite[1])//TLS加密套件占比

        // 调用函数来填充数据
        // 表格数据填充
        populateData(transformData(data[0]));

        // 选择应用显示增长率情况
        //echarts_2(top10_rank[0], top10_rank[1])
      })
  } catch (error) {
    console.error('发生错误:', error);
  }
}

document.addEventListener('DOMContentLoaded', function () {
  // 暂时不需要刷新的部分
  map()
  pe01()//今日流量总计三部分
  pe02()
  pe03()
  echarts_1()
  echarts_2()
  // 需要刷新的部分
  fetchDataAndProcessConcurrently()
  setInterval(fetchDataAndProcessConcurrently, 5000); // 5秒刷新一次 
});

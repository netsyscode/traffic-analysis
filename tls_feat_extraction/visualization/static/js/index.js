$(window).load(function () {
    $(".loading").fadeOut()

})

$(document).ready(function () {
    var whei = $(window).width()
    $("html").css({ fontSize: whei / 20 })
    $(window).resize(function () {
        var whei = $(window).width()
        $("html").css({ fontSize: whei / 20 })
    });

    fetchDataAndProcessConcurrently()
    cl2()
})

function echarts_5(top10data) {
    // 基于准备好的dom，初始化echarts实例
    var myChart = echarts.init(document.getElementById('echarts5'));
    var myColor = ['#eb2100', '#eb3600', '#d0570e', '#d0a00e', '#34da62', '#00e9db', '#00c0e9', '#0096f3'];
    option = {

        grid: {
            left: '2%',
            top: '1%',
            right: '5%',
            bottom: '0%',
            containLabel: true
        },
        xAxis: [{
            show: false,
        }],
        yAxis: [{
            axisTick: 'none',
            axisLine: 'none',
            offset: '7',
            axisLabel: {
                textStyle: {
                    color: 'rgba(255,255,255,.6)',
                    fontSize: '0.16rem',
                }
            },
            data: top10data[0]
        }, {
            axisTick: 'none',
            axisLine: 'none',
            axisLabel: {
                textStyle: {
                    color: 'rgba(255,255,255,.6)',
                    fontSize: '0.14rem',
                }
            },
            data: top10data[2]

        }, {
            name: '单位：件',
            nameGap: '50',
            nameTextStyle: {
                color: 'rgba(255,255,255,.6)',
                fontSize: '0.2rem',
            },
            axisLine: {
                lineStyle: {
                    color: 'rgba(0,0,0,0)'
                }
            },
            data: [],
        }],
        series: [{
            name: '条',
            type: 'bar',
            yAxisIndex: 0,
            data: top10data[1],
            label: {
                normal: {
                    show: true,
                    position: 'right',
                    formatter: function (param) {
                        return param.value + '%';
                    },
                    textStyle: {
                        color: 'rgba(255,255,255,.8)',
                        fontSize: '0.14rem',
                    }
                }
            },
            barWidth: 15,
            itemStyle: {
                normal: {
                    color: new echarts.graphic.LinearGradient(1, 0, 0, 0, [{
                        offset: 0,
                        color: '#03c893'
                    },
                    {
                        offset: 1,
                        color: '#0091ff'
                    }
                    ]),
                    barBorderRadius: 15,
                }
            },
            z: 2
        }, {
            name: '白框',
            type: 'bar',
            yAxisIndex: 1,
            barGap: '-100%',
            data: [99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5],
            barWidth: 15,
            itemStyle: {
                normal: {
                    color: 'rgba(255,255,255,.01)',
                    barBorderRadius: 15,
                }
            },
            z: 1
        }]
    };


    // 使用刚指定的配置项和数据显示图表。
    myChart.setOption(option);
    window.addEventListener("resize", function () {
        myChart.resize();
    });
}

function echarts_4(jsonData) {
    var myChart = echarts.init(document.getElementById('echart4'));
    option1 = {
        //  backgroundColor: '#00265f',
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'shadow'
            }
        },
        legend: {
            data: ['入站流量', '出站流量'],
            top: '5%',
            textStyle: {
                color: "#fff",
                fontSize: '12',

            },

            //itemGap: 35
        },
        grid: {
            left: '0%',
            top: '40px',
            right: '0%',
            bottom: '0',
            containLabel: true
        },
        xAxis: [{
            type: 'category',
            data: jsonData.time,
            axisLine: {
                show: true,
                lineStyle: {
                    color: "rgba(255,255,255,.1)",
                    width: 1,
                    type: "solid"
                },
            },
            axisTick: {
                show: true,
            },
            axisLabel: {
                interval: 2,
                rotate:10,
                show: true,
                //splitNumber: 12,
                textStyle: {
                    color: "rgba(255,255,255,.6)",
                    fontSize: '8',
                },
            },
        }],
        yAxis: [
            {
                type: 'value',
                axisLabel: {
                    formatter: '{value} MB',
                    show: true,
                    textStyle: {
                        color: "rgba(255,255,255,.6)",
                        fontSize: '10',
                    },
                },
                splitNumber:4,
                axisTick: {
                    show: false,
                },
                axisLine: {
                    show: true,
                    lineStyle: {
                        color: "rgba(255,255,255,.1	)",
                        width: 1,
                        type: "solid"
                    },
                },
                splitLine: {
                    lineStyle: {
                        color: "rgba(255,255,255,.1)",
                    }
                    
                }
            }],
        series: [

            {
                name: '入站流量',
                type: 'line',
                smooth: true,
                data: jsonData.ingress_flow,
                barWidth: '15',
                // barGap: 1,
                itemStyle: {
                    normal: {
                        color: '#62c98d',
                        opacity: 1,
                        barBorderRadius: 5,
                    }
                }
            },
            {
                name: '出站流量',
                type: 'line',
                smooth: true,
                data: jsonData.engress_flow,

                itemStyle: {
                    normal: {
                        color: '#ffc000',
                        opacity: 1,

                        barBorderRadius: 5,
                    }
                }
            }
        ]
    };


    myChart.setOption(option1);
    window.addEventListener("resize", function () {
        myChart.resize();
    });
    $(".sebtn a").click(function () {
        $(this).addClass("active").siblings().removeClass("active")
    })
    $(".sebtn a").eq(0).click(function () {
        myChart.setOption(option1);
    })
    $(".sebtn a").eq(1).click(function () {
        myChart.setOption(option2);
    })
    $(".sebtn a").eq(2).click(function () {
        myChart.setOption(option3);
    })

}

function dbj() {
    var dbj = echarts.init(document.getElementById('dbj'));
    option = {
        tooltip: {
            formatter: "{a} <br/>{c} {b}"
        },
        toolbox: {
            show: false,
            feature: {
                restore: {
                    show: true
                },
                saveAsImage: {
                    show: true
                }
            }
        },
        series: [{
            name: '响应时间',
            type: 'gauge',
            center: ['25%', '30%'],
            z: 3,
            min: 0,
            max: 20,
            splitNumber: 10,
            radius: '42%',
            axisLine: {
                lineStyle: {
                    color: [[0.2, '#7FFF00'], [0.8, '#00FFFF'], [1, '#FF0000']],
                    width: 6
                }
            },
            axisTick: {
                length: 5,
                lineStyle: {
                    color: 'auto'
                }
            },
            splitLine: {
                length: 20,
                lineStyle: {
                    color: 'auto'
                }
            },
            pointer: {
                width: 2
            },
            title: {
                textStyle: {
                    fontWeight: 'normal',
                    color: '#fff',
                    fontSize: 12,
                    offsetCenter: ['15%', '-20%'],
                }
            },
            detail: {
                textStyle: {
                    fontWeight: 'normal',
                    fontSize: 12
                }
            },
            data: [{
                value: (Math.random() * 30).toFixed(2) - 0,
                name: '响应时间'
            }]
        },
        {
            name: '吞吐量',
            type: 'gauge',
            center: ['75%', '30%'],
            z: 3,
            min: 0,
            max: 8,
            splitNumber: 8,
            radius: '42%',
            axisLine: {
                lineStyle: {
                    color: [[0.2, '#7CFC00'], [0.8, '#00FFFF'], [1, '#FF0000']],
                    width: 6
                }
            },
            axisTick: {
                length: 15,
                lineStyle: {
                    color: 'auto'
                }
            },
            splitLine: {
                length: 20,
                lineStyle: {
                    color: 'auto'
                }
            },
            pointer: {
                width: 4
            },
            title: {
                textStyle: {
                    fontWeight: 'normal',
                    color: '#fff',
                    fontSize: 12,
                }
            },
            detail: {
                textStyle: {
                    fontWeight: 'normal',
                    fontSize: 12
                }
            },
            data: [{
                value: (Math.random() * 8).toFixed(2) - 0,
                name: '吞吐量'
            }]
        },
        {
            name: '错误率',
            type: 'gauge',
            center: ['25%', '75%'],
            z: 3,
            min: 0,
            max: 1,
            splitNumber: 5,
            radius: '42%',
            axisLine: {
                lineStyle: {
                    color: [[0.2, '#7FFF00'], [0.8, '#00FFFF'], [1, '#FF0000']],
                    width: 6
                }
            },
            axisTick: {
                length: 5,
                lineStyle: {
                    color: 'auto'
                }
            },
            splitLine: {
                length: 20,
                lineStyle: {
                    color: 'auto'
                }
            },
            pointer: {
                width: 2
            },
            title: {
                textStyle: {
                    fontWeight: 'normal',
                    color: '#fff',
                    fontSize: 12,
                    offsetCenter: ['15%', '-20%'],
                }
            },
            detail: {
                textStyle: {
                    fontWeight: 'normal',
                    fontSize: 12
                }
            },
            data: [{
                value: (Math.random() / 5).toFixed(2) - 0,
                name: '错误率'
            }]
        },
        {
            name: '重传率',
            type: 'gauge',
            center: ['75%', '75%'],
            z: 3,
            min: 0,
            max: 1,
            splitNumber: 5,
            radius: '42%',
            axisLine: {
                lineStyle: {
                    color: [[0.2, '#7CFC00'], [0.8, '#00FFFF'], [1, '#FF0000']],
                    width: 6
                }
            },
            axisTick: {
                length: 15,
                lineStyle: {
                    color: 'auto'
                }
            },
            splitLine: {
                length: 20,
                lineStyle: {
                    color: 'auto'
                }
            },
            pointer: {
                width: 4
            },
            title: {
                textStyle: {
                    fontWeight: 'normal',
                    color: '#fff',
                    fontSize: 12,
                }
            },
            detail: {
                textStyle: {
                    fontWeight: 'normal',
                    fontSize: 12
                }
            },
            data: [{
                value: (Math.random() / 8).toFixed(2) - 0,
                name: '重传率'
            }]
        }]
    };
    dbj.setOption(option);
    $(document).ready(function () {
        dbj.resize()
    });
    window.addEventListener("resize", function () {
        dbj.resize()
    })
}

function calculateAppDataProportions(jsonData) {
    //将app_label映射为app名称
    const appLabelsChinese = jsonData.map(([appLabel]) => appLabel.toString()); // 使用箭头函数
    // 将总字节数转换为MB
    const totalBytesMB = jsonData.map(([, totalBytes]) => Math.floor(Number(totalBytes) / 1048576));
    const totalBytesSum = totalBytesMB.reduce((acc, cur) => acc + cur, 0);
    // 计算各个应用的流量占比
    const proportions = totalBytesMB.map(totalBytes => totalBytesSum > 0 ? Math.round((totalBytes / totalBytesSum) * 10000) / 100 : 0);
    const userCount = jsonData.map(([, , users]) => users);
    return [appLabelsChinese.reverse(), proportions.reverse(), userCount.reverse()];
}

function extractHourlyTrafficData(jsonData) {
    const times = [];
    const ingress_flows = [];
    const engress_flows = [];

    jsonData.forEach(item => {
        times.push(item[0]);     
        ingress_flows.push(item[1]);    
        engress_flows.push(item[2]);  
    });

    return {
        time: times,
        ingress_flow: ingress_flows,
        engress_flow: engress_flows
    };
}

async function fetchDataAndProcessConcurrently() {
    try {
        // 使用await直接等待fetch请求的结果
        const response = await fetch('/get_top10_traffic');
        const data = await response.json(); // 等待解析JSON
        const top10_rank = calculateAppDataProportions(data); // 计算数据比例
        echarts_5(top10_rank); // 使用echarts展示数据
        dbj(); // 调用其他逻辑处理函数
    } catch (error) {
        console.error('发生错误:', error); // 错误处理
    }
}

function clearHighlight() {
    document.querySelectorAll('.title p span').forEach(function(span) {
      span.classList.remove('span_c');
    });
}

function cl1() {
    clearHighlight();
    document.querySelector('.title p span:nth-child(1)').classList.add('span_c');
    fetch('/get_hourly_data')
      .then(response => response.json())
      .then(data => {
        hourlyTrafficData = extractHourlyTrafficData(data)
        echarts_4(hourlyTrafficData)
      })
      .catch(error => console.error('Error:', error));
}

function cl2() {
    clearHighlight();
    document.querySelector('.title p span:nth-child(2)').classList.add('span_c');
    fetch('/get_daily_data')
      .then(response => response.json())
      .then(data => {
        hourlyTrafficData = extractHourlyTrafficData(data)
        echarts_4(hourlyTrafficData)
      })
      .catch(error => console.error('Error:', error));
}

function cl3() {
    clearHighlight();
    document.querySelector('.title p span:nth-child(3)').classList.add('span_c');
    fetch('/get_weekly_data')
      .then(response => response.json())
      .then(data => {
        hourlyTrafficData = extractHourlyTrafficData(data)
        echarts_4(hourlyTrafficData)
      })
      .catch(error => console.error('Error:', error));
}

function cl4() {
    clearHighlight();
    document.querySelector('.title p span:nth-child(4)').classList.add('span_c');
    fetch('/get_monthly_data')
      .then(response => response.json())
      .then(data => {
        hourlyTrafficData = extractHourlyTrafficData(data)
        echarts_4(hourlyTrafficData)
      })
      .catch(error => console.error('Error:', error));
}



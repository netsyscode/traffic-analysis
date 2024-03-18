/**jQuery**/
$(document).ready(function () {
  var whei = $(window).width()
  $("html").css({ fontSize: whei / 20 })
  $(window).resize(function () {
    var whei = $(window).width()
    $("html").css({ fontSize: whei / 20 })
  });
});

$(window).on('load', function () {
  $(".loading").fadeOut();
});

let data = [{
  name: '张家口',
  value: 47
}, {
  name: '佛山',
  value: 22
}];

let convertData = function (data) {
  let res = [];
  for (let i = 0; i < data.length; i++) {
    let geoCoord = geoCoordMap[data[i].name];
    if (geoCoord) {
      res.push({
        name: data[i].name,
        value: geoCoord.concat(data[i].value)
      });
    }
  };
  // console.log(res);
  return res;
};
let convertDataLines = function (data) {
  var res = [];
  for (var i = 0; i < data.length; i++) {

    var dataItem = data[i];

    var fromCoord = geoCoordMap[dataItem.fromName];
    var toCoord = geoCoordMap[dataItem.toName];
    if (fromCoord && toCoord) {
      res.push({
        fromName: dataItem.fromName,
        toName: dataItem.toName,
        coords: [fromCoord, toCoord],
        value: dataItem.value
      });
    }
  }
  // console.log(res);
  return res;
};

function echarts_1() {
  var myChart = echarts.init(document.getElementById('echarts1'));

  option = {
    tooltip: {
      trigger: 'axis',
      axisPointer: { type: 'shadow' },
    }, "grid": {
      "top": "20%",
      "right": "50",
      "bottom": "20",
      "left": "30",
    },
    legend: {
      data: ['抖音', 'QQ', '百度', 'App4', 'App5', 'App6'],
      right: 'center', width: '100%',
      textStyle: {
        color: "#fff"
      },
      itemWidth: 12,
      itemHeight: 10,
    },

    "xAxis": [
      {
        "type": "category",
        data: ['2017/06', '2017/07', '2017/08', '2017/09'],
        axisLine: { lineStyle: { color: "rgba(255,255,255,.1)" } },
        axisLabel: {
          textStyle: { color: "rgba(255,255,255,.7)", fontSize: '14', },
        },

      },
    ],
    "yAxis": [
      {
        "type": "value",
        "name": "单位万",
        axisTick: { show: false },
        splitLine: {
          show: false,

        },
        "axisLabel": {
          "show": true,
          fontSize: 14,
          color: "rgba(255,255,255,.6)"

        },
        axisLine: {
          min: 0,
          max: 10,
          lineStyle: { color: 'rgba(255,255,255,.1)' }
        },//左线色

      },
      {
        "type": "value",
        "name": "增速",
        "show": true,
        "axisLabel": {
          "show": true,
          fontSize: 14,
          formatter: "{value} %",
          color: "rgba(255,255,255,.6)"
        },
        axisTick: { show: false },
        axisLine: { lineStyle: { color: 'rgba(255,255,255,.1)' } },//右线色
        splitLine: { show: true, lineStyle: { color: 'rgba(255,255,255,.1)' } },//x轴线
      },
    ],
    "series": [

      {
        "name": "抖音",
        "type": "bar",
        "data": [36.6, 38.80, 40.84, 41.60],
        "barWidth": "15%",
        "itemStyle": {
          "normal": {
            barBorderRadius: 15,
            color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
              offset: 0,
              color: '#8bd46e'
            }, {
              offset: 1,
              color: '#09bcb7'
            }]),
          }
        },
        "barGap": "0.2"
      },
      {
        "name": "QQ",
        "type": "bar",
        "data": [14.8, 14.1, 15, 16.30],
        "barWidth": "15%",
        "itemStyle": {
          "normal": {
            barBorderRadius: 15,
            color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
              offset: 0,
              color: '#248ff7'
            }, {
              offset: 1,
              color: '#6851f1'
            }]),
          }
        },
        "barGap": "0.2"
      },
      {
        "name": "百度",
        "type": "bar",
        "data": [9.2, 9.1, 9.85, 8.9],
        "barWidth": "15%",
        "itemStyle": {
          "normal": {
            barBorderRadius: 15,
            color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
              offset: 0,
              color: '#fccb05'
            }, {
              offset: 1,
              color: '#f5804d'
            }]),
          }
        },
        "barGap": "0.2"
      },
      {
        "name": "",
        "type": "line",
        smooth: true,
        "yAxisIndex": 1,
        "data": [0, 6.01, 5.26, 1.48],
        lineStyle: {
          normal: {
            width: 2
          },
        },
        "itemStyle": {
          "normal": {
            "color": "#86d370",

          }
        },

      }
      ,
      {
        "name": "",
        "type": "line",
        "yAxisIndex": 1,

        "data": [0, -4.73, 6.38, 8.67],
        lineStyle: {
          normal: {
            width: 2
          },
        },
        "itemStyle": {
          "normal": {
            "color": "#3496f8",

          }
        },
        "smooth": true
      },
      {
        "name": "",
        "type": "line",
        "yAxisIndex": 1,

        "data": [0, 1.09, 8.24, 9.64],
        lineStyle: {
          normal: {
            width: 2
          },
        },
        "itemStyle": {
          "normal": {
            "color": "#fbc30d",

          }
        },
        "smooth": true
      }
    ]
  };

  myChart.setOption(option);
  window.addEventListener("resize", function () {
    myChart.resize();
  });
}
//function echarts_2(appOptions, appData) {
function echarts_2() {
  // 基于准备好的dom，初始化echarts实例
  var myChart = echarts.init(document.getElementById('echarts2'));

  option = {
    tooltip: {
      trigger: 'axis',
      axisPointer: { type: 'shadow' },
      // formatter:'{c}' ,
    },
    grid: {
      left: '0',
      top: '30',
      right: '10',
      bottom: '-20',
      containLabel: true
    },
    legend: {
      data: ['增长情况', '增长率'],
      right: 'center',
      top: 0,
      textStyle: {
        color: "#fff"
      },
      itemWidth: 12,
      itemHeight: 10,
      // itemGap: 35
    },

    xAxis: [{
      type: 'category',
      boundaryGap: false,
      axisLabel: {
        rotate: -90,
        textStyle: {
          color: "rgba(255,255,255,.6)",
          fontSize: 14,

        },
      },
      axisLine: {
        lineStyle: {
          color: 'rgba(255,255,255,.1)'
        }

      },

      data: ['17年3月', '17年6月', '17年9月', '17年12月', '18年3月', '18年6月', '18年9月', '18年12月', '19年3月', '19年6月', '19年9月', '19年12月']

    }, {

      axisPointer: { show: false },
      axisLine: { show: false },
      position: 'bottom',
      offset: 20,

    }],

    yAxis: [{
      type: 'value',
      axisTick: { show: false },
      // splitNumber: 6,
      axisLine: {
        lineStyle: {
          color: 'rgba(255,255,255,.1)'
        }
      },
      axisLabel: {
        formatter: "{value} ",
        textStyle: {
          color: "rgba(255,255,255,.6)",
          fontSize: 14,
        },
      },

      splitLine: {
        lineStyle: {
          color: 'rgba(255,255,255,.1)'
        }
      }
    }],
    series: [
      {
        name: '增长情况',
        type: 'line',
        smooth: true,
        symbol: 'circle',
        symbolSize: 5,
        showSymbol: false,
        lineStyle: {
          normal: {
            color: 'rgba(228, 228, 126, 1)',
            width: 2
          }
        },
        areaStyle: {
          normal: {
            color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
              offset: 0,
              color: 'rgba(228, 228, 126, .2)'
            }, {
              offset: 1,
              color: 'rgba(228, 228, 126, 0)'
            }], false),
            shadowColor: 'rgba(0, 0, 0, 0.1)',
          }
        },
        itemStyle: {
          normal: {
            color: 'rgba(228, 228, 126, 1)',
            borderColor: 'rgba(228, 228, 126, .1)',
            borderWidth: 12
          }
        },
        data: [18, 24, 21, 29, 20, 22, 30, 32, 22, 24, 27, 30]

      }, {
        name: '增长率',
        type: 'line',
        smooth: true,
        symbol: 'circle',
        symbolSize: 5,
        showSymbol: false,
        lineStyle: {

          normal: {
            color: 'rgba(255, 128, 128, 1)',
            width: 2
          }
        },
        areaStyle: {
          normal: {
            color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
              offset: 0,
              color: 'rgba(255, 128, 128,.2)'
            }, {
              offset: 1,
              color: 'rgba(255, 128, 128, 0)'
            }], false),
            shadowColor: 'rgba(0, 0, 0, 0.1)',
          }
        },
        itemStyle: {
          normal: {
            color: 'rgba(255, 128, 128, 1)',
            borderColor: 'rgba(255, 128, 128, .1)',
            borderWidth: 12

          }
        },
        data: [24, 21, 16, 22, 21, 20, 22, 31, 25, 22, 23, 29, 30]
      },
    ]
  };
  // 使用刚指定的配置项和数据显示图表。
  myChart.setOption(option);

  // 创建选项框并设置其事件处理
  // var selectBox = document.createElement('select');
  // selectBox.setAttribute('id', 'appSelectBox');
  // appOptions.names.forEach(function(name) {
  //   var option = document.createElement('option');
  //   option.value = name;
  //   option.text = name;
  //   selectBox.appendChild(option);
  // });
  // document.getElementById('echarts2').appendChild(selectBox);

  // // 监听选项框的变化
  // selectBox.addEventListener('change', function() {
  //   var appName = this.value;
  //   setChartData(appName); // 更新图表数据
  // });

  // // 默认加载第一个应用的数据
  // if (appOptions.names.length > 0) {
  //   setChartData(appOptions.names[0]);
  // }


  window.addEventListener("resize", function () {
    myChart.resize();
  });
}
function echarts_3() {
  // 基于准备好的dom，初始化echarts实例
  var myChart = echarts.init(document.getElementById('echarts3'));

  option = {

  };

  // 使用刚指定的配置项和数据显示图表。
  myChart.setOption(option);
  window.addEventListener("resize", function () {
    myChart.resize();
  });
}
function map() {
  var chartContainer = document.getElementById('map');
  var myChart = echarts.getInstanceByDom(chartContainer) || echarts.init(chartContainer);
  option = {
    title: {
      top: 10,
      text: '流量流向',
      left: 'center',
      textStyle: {
        color: '#fff'
      }
    },

    legend: {
      orient: 'vertical',
      top: 'bottom',
      left: 'right',
      // data:['西安 Top3', '西宁 Top3', '银川 Top3'],
      textStyle: {
        color: '#fff'
      },
      selectedMode: 'multiple'
    },
    tooltip: {
      show: true
    },
    visualMap: [{
      show: false,
      //值是分散的0-10,10-20.。。。还是连续的0-300
      type: 'continuous',

      seriesIndex: 0,

      //两端的文本，如 ['High', 'Low']
      text: [''],
      //是否显示拖拽用的手柄（手柄能拖拽调整选中范围）。
      //ture则拖拽手柄过程中实时更新图表视图
      calculable: true,
      max: 100,
      inRange: {
        color: ['#edfbfb', '#b7d6f3', '#40a9ed', '#3598c1', '#215096']
      }
    }, {
      show: false,
      type: 'continuous',
      seriesIndex: 1,
      id: 1,
      text: ['1'],
      calculable: true,
      max: 10,
      inRange: {
        color: ['yellow']
      }
    }, {
      show: false,
      type: 'continuous',
      seriesIndex: 2,
      id: 2,
      text: ['1'],
      calculable: true,
      min: 10,
      max: 40,
      inRange: {
        color: ['white', 'yellow']

      }
    }, {
      show: false,
      type: 'continuous',
      seriesIndex: 3,
      id: 3,
      text: ['1'],
      calculable: true,
      min: 10,
      max: 40,
      inRange: {
        color: ['blue', 'green']

      }
    }, {
      show: false,
      type: 'continuous',
      seriesIndex: 4,
      id: 4,
      text: ['1'],
      calculable: true,
      min: 10,
      max: 40,
      inRange: {
        color: ['white']

      }
    }],
    geo: {
      map: 'china',
      aspectScale: 1,
      label: {
        normal: {
          show: false
        }
      },
      itemStyle: {
        normal: {
          borderColor: 'rgba(147, 235, 248, 1)',
          borderWidth: 1,
          areaColor: {
            type: 'radial',
            x: 1,
            y: 1,
            r: 1,
            colorStops: [{
              offset: 0,
              color: 'rgba(147, 235, 248, .9)' // 0% 处的颜色
            }, {
              offset: 1,
              color: 'rgba(147, 235, 248, .9)' // 100% 处的颜色
            }],
            globalCoord: false // 缺省为 false
          },
          shadowColor: 'rgba(128, 217, 248, .2)',
          shadowOffsetX: 12,
          shadowOffsetY: 12,
          shadowBlur: 1
        },
        emphasis: {
          areaColor: 'yellow'
        }
      },
      z: 2
    },
    series: [{
      type: 'map',
      map: 'china',
      geoIndex: 0,
      name: '热力分布',
      id: 0,
      tooltip: {
        show: true
      },
      label: {
        normal: {
          show: false,
          formatter: function (val) {
            console.log(val, 9999999999)
            var area_content = '{a|' + val.name + '}' + '-' + '{b|' + val.value + '}';
            return area_content.split("-").join("\n");
          }, //让series 中的文字进行换行
          rich: {
            a: {
              color: '#fff'
            },
            b: {
              color: '#fff',
              fontFamily: 'Microsoft YaHei',
              fontSize: 14,
              width: 16,
              height: 16,
              borderRadius: 10,
              borderWidth: 1,
              borderColor: '#f00',
              textAlign: 'center',
              padding: 2
            }
          }, //富文本样式,就是上面的formatter中'{a|'和'{b|'
        },
        emphasis: {
          show: false
        }
      }, //地图中文字内容及样式控制

      aspectScale: 1,
      roam: false,
      itemStyle: {
        normal: {
          borderColor: 'rgba(147, 235, 248, 0.6)',
          borderWidth: 1,
          areaColor: 'rgba(147, 235, 248, 0)'
        },
        emphasis: {
          areaColor: 'rgba(147, 235, 248, 0)'
        }
      },
      zlevel: 5,
      data: datavalue
    }, {
      name: '重要点',
      type: 'effectScatter',
      id: 1,
      geoIndex: 0,
      coordinateSystem: 'geo',
      data: convertData(data),
      symbol: '',
      symbolSize: function (val) {
        console.log(val);
        return val[2] / 5;
      },
      showEffectOn: 'render',
      rippleEffect: {
        brushType: 'stroke'
      },
      hoverAnimation: true,
      label: {
        normal: {
          formatter: '{b}',
          position: 'right',
          show: true
        }
      },
      itemStyle: {
        normal: {
          color: '#f4e925',
          shadowBlur: 10,
          shadowColor: '#333'
        }
      },
      zlevel: 5
    }, {
      name: '线路',
      type: 'lines',
      coordinateSystem: 'geo',
      zlevel: 2,
      large: false,
      effect: {
        show: true,
        period: 4, //箭头指向速度，值越小速度越快
        trailLength: 0.02, //特效尾迹长度[0,1]值越大，尾迹越长重
        symbol: "arrow", //箭头图标
        symbolSize: 5 //图标大小

      },
      lineStyle: {
        normal: {
          width: 1, //尾迹线条宽度
          opacity: 1, //尾迹线条透明度
          curveness: 0.3 //尾迹线条曲直度
        }
      },
      data: convertDataLines(moveLines)
    }, {
      name: '线路1',
      type: 'lines',
      coordinateSystem: 'geo',
      zlevel: 2,
      large: false,
      effect: {
        show: true,
        period: 4, //箭头指向速度，值越小速度越快
        trailLength: 0.02, //特效尾迹长度[0,1]值越大，尾迹越长重
        symbol: "arrow", //箭头图标
        symbolSize: 5 //图标大小

      },
      lineStyle: {
        normal: {
          width: 1, //尾迹线条宽度
          opacity: 1, //尾迹线条透明度
          curveness: 0.3 //尾迹线条曲直度
        }
      },
      data: convertDataLines(moveLines1)
    }, {
      name: '线路2',
      type: 'lines',
      coordinateSystem: 'geo',
      zlevel: 2,
      large: false,
      effect: {
        show: true,
        period: 4, //箭头指向速度，值越小速度越快
        trailLength: 0, //特效尾迹长度[0,1]值越大，尾迹越长重
        symbol: "circle", //箭头图标
        symbolSize: 5 //图标大小

      },
      lineStyle: {
        normal: {
          width: 0, //尾迹线条宽度
          opacity: 1, //尾迹线条透明度
          curveness: 0 //尾迹线条曲直度
        }
      },
      data: convertDataLines(moveLines2)
    }]
  };
    // 使用刚指定的配置项和数据显示图表。
    myChart.setOption(option);
    window.addEventListener("resize", function () {
      myChart.resize();
})
}
function echarts_5() {
  // 基于准备好的dom，初始化echarts实例
  var myChart = echarts.init(document.getElementById('echarts5'));

  option = {
    tooltip: {
      trigger: 'axis',
      axisPointer: { type: 'shadow' },
    }, "grid": {
      "top": "15%",
      "right": "10%",
      "bottom": "20",
      "left": "10%",
    },
    legend: {
      data: ['异常事件数量', '增长率'],
      right: 'center',
      top: 0,
      textStyle: {
        color: "#fff"
      },
      itemWidth: 12,
      itemHeight: 10,
    },
    "xAxis": [
      {
        "type": "category",

        data: ['2017/06', '2017/07', '2017/08', '2017/09'],
        axisLine: { lineStyle: { color: "rgba(255,255,255,.1)" } },
        axisLabel: {
          textStyle: { color: "rgba(255,255,255,.7)", fontSize: '14', },
        },

      },
    ],
    "yAxis": [
      {
        "type": "value",
        "name": "数量",
        splitLine: { show: false },
        axisTick: { show: false },
        "axisLabel": {
          "show": true,
          color: "rgba(255,255,255,.6)"

        },
        axisLine: { lineStyle: { color: 'rgba(255,255,255,.1)' } },//左线色

      },
      {
        "type": "value",
        "name": "增长率",
        "show": true,
        axisTick: { show: false },
        "axisLabel": {
          "show": true,
          formatter: "{value} %",
          color: "rgba(255,255,255,.6)"
        },
        axisLine: { lineStyle: { color: 'rgba(255,255,255,.1)' } },//右线色
        splitLine: { show: true, lineStyle: { color: 'rgba(255,255,255,.1)' } },//x轴线
      },
    ],
    "series": [

      {
        "name": "异常事件数量",
        "type": "bar",
        "data": [
          18453.35, 20572.22, 24274.22, 30500.00
        ],
        "barWidth": "20%",

        "itemStyle": {
          "normal": {
            barBorderRadius: 15,
            color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
              offset: 0,
              color: '#fccb05'
            }, {
              offset: 1,
              color: '#f5804d'
            }]),
          }
        },
        "barGap": "0"
      },
      {
        "name": "增长率",
        "type": "line",
        "yAxisIndex": 1,

        "data": [0, 1.48, 1.00, 1.65],
        lineStyle: {
          normal: {
            width: 2
          },
        },
        "itemStyle": {
          "normal": {
            "color": "#ff3300",

          }
        },
        "smooth": true
      }
    ]
  };
  // 使用刚指定的配置项和数据显示图表。
  myChart.setOption(option);
  window.addEventListener("resize", function () {
    myChart.resize();
  });
}
function echarts_4(app_name, app_packet_count, app_packet_ratio) {

  // 基于准备好的dom，初始化echarts实例
  var myColor = ['#eb2100', '#eb3600', '#d0570e', '#d0a00e', '#34da62', '#00e9db', '#00c0e9', '#0096f3'];
  var chartContainer = document.getElementById('echarts4');
  var myChart = echarts.getInstanceByDom(chartContainer) || echarts.init(chartContainer);
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
          fontSize: '14',
        }
      },
      data: app_name
    }, {//两个标签
      axisTick: 'none',
      axisLine: 'none',
      axisLabel: {
        textStyle: {
          color: 'rgba(255,255,255,.6)',
          fontSize: '14',
        }
      },
      data: app_packet_count
    }, {
      name: '单位：件',
      nameGap: '50',
      nameTextStyle: {
        color: 'rgba(255,255,255,.6)',
        fontSize: '16',
      },
      axisLine: {
        lineStyle: {
          color: 'rgba(0,0,0,0)'
        }
      },
      data: []
    }],
    series: [{
      name: '条',
      type: 'bar',
      yAxisIndex: 0,
      data: app_packet_ratio,
      label: {
        normal: {
          show: true,
          position: 'right',
          formatter: function (param) {
            return param.value + '%';
          },
          textStyle: {
            color: 'rgba(255,255,255,.8)',
            fontSize: '12',
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
    },
    {
      name: '白框',//背景bar
      type: 'bar',
      yAxisIndex: 1,
      barGap: '-100%',
      data: [99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5, 99.5],
      barWidth: 15,
      itemStyle: {
        normal: {
          color: 'rgba(0,0,0,0)',
          barBorderRadius: 15,
        }
      },
      z: 1
    }
    ]
  };
  // 使用刚指定的配置项和数据显示图表。
  myChart.setOption(option);
  window.addEventListener("resize", function () {
    myChart.resize();
  });
}

function echarts6(prevTLS, otherEncry) {
  // 基于准备好的dom，初始化echarts实例
  var chartContainer = document.getElementById('echarts6');
  var myChart = echarts.getInstanceByDom(chartContainer) || echarts.init(chartContainer);

  var option = {
    title: {
      text: otherEncry + prevTLS,
      subtext: '流总计',
      x: 'center',
      y: '40%',
      textStyle: {
        color: '#fff',
        fontSize: 22,
        lineHeight: 10,
      },
      subtextStyle: {
        color: '#90979c',
        fontSize: 16,
        lineHeight: 10,
      },
    },
    tooltip: {
      trigger: 'item',
      formatter: "{b} : {c} ({d}%)"
    },
    visualMap: {
      show: false,
      min: 500,
      max: 600,
      inRange: {
        //colorLightness: [0, 1]
      }
    },
    series: [{
      name: '访问来源',
      type: 'pie',
      radius: ['50%', '70%'],
      center: ['50%', '50%'],
      color: ['#FE5050', 'rgb(131,249,103)',  '#FBFE27'],
      data: [
        {
          "value": prevTLS,
          "name": "TLS"
        }, {
          "value": otherEncry,
          "name": "其他"
        }
        // 移除了之前存在的 TLS1.2 相关的数据
      ].sort(function (a, b) {
        return a.value - b.value
      }),
      roseType: 'radius',
      label: {
        normal: {
          formatter: ['{c|{c}}', '{b|{b}}'].join('\n'),
          rich: {
            c: {
              color: 'rgb(241,246,104)',
              fontSize: 15,
              fontWeight: 'bold',
              lineHeight: 5
            },
            b: {
              color: 'rgb(98,137,169)',
              fontSize: 15,
              height: 44
            },
          },
        }
      },
      labelLine: {
        normal: {
          lineStyle: {
            color: 'rgb(98,137,169)',
          },
          smooth: 0.2,
          length: 10,
          length2: 20,
        }
      }
    }]
  };


  // 使用刚指定的配置项和数据显示图表。
  myChart.setOption(option);
  window.addEventListener("resize", function () {
    myChart.resize();
  });
}

function pe01() {
  // 基于准备好的dom，初始化echarts实例
  var chartContainer = document.getElementById('pe01');
  var myChart = echarts.getInstanceByDom(chartContainer) || echarts.init(chartContainer);

  var txt = 71
  option = {
    title: {
      text: txt + '%',
      x: 'center',
      y: 'center',
      textStyle: {
        fontWeight: 'normal',
        color: '#fff',
        fontSize: '12'
      }
    },
    color: 'rgba(255,255,255,.3)',

    series: [{
      name: 'Line 1',
      type: 'pie',
      clockWise: true,
      radius: ['65%', '80%'],
      itemStyle: {
        normal: {
          label: {
            show: false
          },
          labelLine: {
            show: false
          }
        }
      },
      hoverAnimation: false,
      data: [{
        value: txt,
        name: '已使用',
        itemStyle: {
          normal: {
            color: '#eaff00',
            label: {
              show: false
            },
            labelLine: {
              show: false
            }
          }
        }
      }, {
        name: '未使用',
        value: 100 - txt
      }]
    }]
  };

  // 使用刚指定的配置项和数据显示图表。
  myChart.setOption(option);
  window.addEventListener("resize", function () {
    myChart.resize();
  });
}
function pe02() {
  // 基于准备好的dom，初始化echarts实例
  var chartContainer = document.getElementById('pe02');
  var myChart = echarts.getInstanceByDom(chartContainer) || echarts.init(chartContainer);
  var txt = 25
  option = {
    title: {
      text: txt + '%',
      x: 'center',
      y: 'center',
      textStyle: {
        fontWeight: 'normal',
        color: '#fff',
        fontSize: '18'
      }
    },
    color: 'rgba(255,255,255,.3)',

    series: [{
      name: 'Line 1',
      type: 'pie',
      clockWise: true,
      radius: ['65%', '80%'],
      itemStyle: {
        normal: {
          label: {
            show: false
          },
          labelLine: {
            show: false
          }
        }
      },
      hoverAnimation: false,
      data: [{
        value: txt,
        name: '已使用',
        itemStyle: {
          normal: {
            color: '#ea4d4d',
            label: {
              show: false
            },
            labelLine: {
              show: false
            }
          }
        }
      }, {
        name: '未使用',
        value: 100 - txt
      }]
    }]
  };

  // 使用刚指定的配置项和数据显示图表。
  myChart.setOption(option);
  window.addEventListener("resize", function () {
    myChart.resize();
  });
}
function pe03() {
  // 基于准备好的dom，初始化echarts实例
  var chartContainer = document.getElementById('pe03');
  var myChart = echarts.getInstanceByDom(chartContainer) || echarts.init(chartContainer);
  var txt = 4
  option = {
    title: {
      text: txt + '%',
      x: 'center',
      y: 'center',
      textStyle: {
        fontWeight: 'normal',
        color: '#fff',
        fontSize: '18'
      }
    },
    color: 'rgba(255,255,255,.3)',

    series: [{
      name: 'Line 1',
      type: 'pie',
      clockWise: true,
      radius: ['65%', '80%'],
      itemStyle: {
        normal: {
          label: {
            show: false
          },
          labelLine: {
            show: false
          }
        }
      },
      hoverAnimation: false,
      data: [{
        value: txt,
        name: '已使用',
        itemStyle: {
          normal: {
            color: '#395ee6',
            label: {
              show: false
            },
            labelLine: {
              show: false
            }
          }
        }
      }, {
        name: '未使用',
        value: 100 - txt
      }]
    }]
  };
  myChart.setOption(option);
  window.addEventListener("resize", function () {
    myChart.resize();
  });
}


























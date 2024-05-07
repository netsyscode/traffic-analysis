$(window).load(function () {
    $(".loading").fadeOut()
})

/****/
$(document).ready(function () {
    var whei = $(window).width()
    $("html").css({ fontSize: whei / 20 })
    $(window).resize(function () {
        var whei = $(window).width()
        $("html").css({ fontSize: whei / 20 })
    });
    pie1();
    pie2();

})

data = {
    legendData : [
        "微信", "支付宝", "抖音", "快手", "微博",
        "淘宝", "京东", "美团", "滴滴出行", "百度",
        "QQ", "QQ音乐", "网易云音乐", "爱奇艺", "优酷",
        "搜狐视频", "腾讯视频", "今日头条", "知乎", "豆瓣",
        "天猫", "苏宁易购", "饿了么", "大众点评", "高德地图",
        "百度地图", "腾讯地图", "QQ浏览器", "UC浏览器", "360浏览器",
        "百度云", "腾讯微云", "华为云", "小米云", "网易有道词典",
        "搜狗输入法", "百度输入法", "QQ输入法", "金山词霸", "迅雷",
        "WPS Office", "钉钉", "华为应用市场", "OPPO软件商店", "vivo应用商店",
        "小米应用商店", "360手机助手", "安卓市场", "百度手机助手", "谷歌播放商店"
    ],
    seriesData: [{"name": "微信", "value": 100000}, {"name": "支付宝", "value": 99027}, 
    {"name": "抖音", "value": 98040}, {"name": "快手", "value": 97056}, {"name": "微博", "value": 96064}, 
    {"name": "淘宝", "value": 95125}, {"name": "京东", "value": 94129}, {"name": "美团", "value": 93174}, 
    {"name": "滴滴出行", "value": 92231}, {"name": "百度", "value": 91294}, {"name": "QQ", "value": 90374}, 
    {"name": "QQ音乐", "value": 89441}, {"name": "网易云音乐", "value": 88470}, {"name": "爱奇艺", "value": 87495}, 
    {"name": "优酷", "value": 86558}, {"name": "搜狐视频", "value": 85578}, {"name": "腾讯视频", "value": 84677}, 
    {"name": "今日头条", "value": 83746}, {"name": "知乎", "value": 82765}, {"name": "豆瓣", "value": 81841}, 
    {"name": "天猫", "value": 80908}, {"name": "苏宁易购", "value": 79970}, {"name": "饿了么", "value": 79008}, 
    {"name": "大众点评", "value": 78099}, {"name": "高德地图", "value": 77151}, {"name": "百度地图", "value": 76211}, 
    {"name": "腾讯地图", "value": 75221}, {"name": "QQ浏览器", "value": 74316}, {"name": "UC浏览器", "value": 73398}, 
    {"name": "360浏览器", "value": 72445}, {"name": "百度云", "value": 71462}, {"name": "腾讯微云", "value": 70483}, 
    {"name": "华为云", "value": 69488}, {"name": "小米云", "value": 68533}, {"name": "网易有道词典", "value": 67616}, 
    {"name": "搜狗输入法", "value": 66681}, {"name": "百度输入法", "value": 65703}, {"name": "QQ输入法", "value": 64709}, 
    {"name": "金山词霸", "value": 63784}, {"name": "迅雷", "value": 62806}, {"name": "WPS Office", "value": 61869}, 
    {"name": "钉钉", "value": 60965}, {"name": "华为应用市场", "value": 60064}, {"name": "OPPO软件商店", "value": 59160}, 
    {"name": "vivo应用商店", "value": 58247}, {"name": "小米应用商店", "value": 57283}, {"name": "360手机助手", "value": 56320}, 
    {"name": "安卓市场", "value": 55343}, {"name": "百度手机助手", "value": 54344}, {"name": "谷歌播放商店", "value": 53432}]
}

function pie1() {
    // 基于准备好的dom，初始化echarts实例
    var myChart = echarts.init(document.getElementById('pie1'));
    option = {
        legend: {
            top: 'bottom',
            textStyle: {
                color: '#FFFFFF' // 设置标题文字颜色为白色
            }
        },
        tooltip: {
            trigger: 'item',
            formatter: '{a} <br/>{b} : {c} ({d}%)',
    
          },
        toolbox: {
            show: true,
            feature: {
                mark: { show: false},
                // dataView: { show: true, readOnly: false },
                restore: { show: true },
                saveAsImage: { show: true }
            },
        },
        series: [
            {
                name: '应用类别',
                type: 'pie',
                radius: [50, 200],
                center: ['50%', '50%'],
                roseType: 'area',
                itemStyle: {
                    borderRadius: 8
                },
                label:{
                    show: true, // 一直显示标签
                    color: 'white', // 标签的字体颜色设置为白色
                    // 其他样式配置...
                },
                data: [
                    { value: 100, name: '影音类' },
                    { value: 88, name: '社交类' },
                    { value: 78, name: '新闻类' },
                    { value: 60, name: '购物类' },
                    { value: 48, name: '金融类' },
                    { value: 32, name: '下载类' },
                    { value: 30, name: '聊天类' },
                    { value: 24, name: '游戏类' },
                    { value: 18, name: '生活类' },
                ]
            }
        ]
    };


    // 使用刚指定的配置项和数据显示图表。
    myChart.setOption(option);
    window.addEventListener("resize", function () {
        myChart.resize();
    });
}

function pie2() {
    // 基于准备好的dom，初始化echarts实例
    var chartContainer = document.getElementById('pie2');
    var myChart = echarts.getInstanceByDom(chartContainer) || echarts.init(chartContainer);
    option = {
        title: {
            text: '应用流量详情',
            left: 'center',
            textStyle: {
                color: '#FFFFFF' // 设置标题文字颜色为白色
            }
        },
        tooltip: {
            trigger: 'item',
            formatter: '{a} <br/>{b} : {c} ({d}%)'
        },
        toolbox: {
            show: true,
            feature: {
                mark: { show: false},
                // dataView: { show: true, readOnly: false },
                restore: { show: true },
                saveAsImage: { show: true }
            },
        },
        legend: {
            type: 'scroll',
            orient: 'vertical',
            right: 10,
            top: 20,
            bottom: 10,
            textStyle: {
                color: '#FFFFFF' // 设置标题文字颜色为白色
            },
            pageIconColor: '#FFFFFF',
            // pageIconInactiveColor: '#FFFFFF',
            pageTextStyle: {
                color: '#FFFFFF' // 这里设置滚动条文字颜色为白色
            },
            data: data.legendData
        },
        series: [
            {
                name: 'app',
                type: 'pie',
                radius: '55%',
                center: ['40%', '50%'],
                label:{
                    show: true, // 一直显示标签
                    color: 'white', // 标签的字体颜色设置为白色
                    // 其他样式配置...
                },
                data: data.seriesData,
                emphasis: {
                    itemStyle: {
                        shadowBlur: 100,
                        shadowOffsetX: 0,
                        shadowColor: 'rgba(0, 0, 0, 0.5)',
                    },
                }
            }
        ]
    };
    // 使用刚指定的配置项和数据显示图表。
    myChart.setOption(option);
    window.addEventListener("resize", function () {
        myChart.resize();
    });
}

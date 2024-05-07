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
    //echarts_3()
    dbj();
    echarts_3();
})

var data = [
    {
        "source_IP": "116.236.39.133",
        "destination_IP": "156.234.169.140",
        "source_port": 33640,
        "destination_port": 171,
        "connection_status": "FIN_WAIT_1",
        "transferred_bytes": 91781,
        "retransmissions": 19,
        "RTT": "195ms",
        "bandwidth": "954Mbps",
        "creation_time": "2024-04-09 17:00:50",
        "duration": "2847s",
        "packet_count": 3143,
        "window_size": 15439,
        "protocol_type": "TCP",
        "traffic_type": "FTP"
    },
    {
        "source_IP": "204.176.85.212",
        "destination_IP": "76.126.166.214",
        "source_port": 54925,
        "destination_port": 349,
        "connection_status": "CLOSED",
        "transferred_bytes": 3762,
        "retransmissions": 3,
        "RTT": "99ms",
        "bandwidth": "79Mbps",
        "creation_time": "2024-04-09 18:20:19",
        "duration": "1397s",
        "packet_count": 4555,
        "window_size": 51962,
        "protocol_type": "TCP",
        "traffic_type": "FTP"
    },
    {
        "source_IP": "116.80.31.117",
        "destination_IP": "59.91.224.115",
        "source_port": 45635,
        "destination_port": 385,
        "connection_status": "SYN_SENT",
        "transferred_bytes": 2530,
        "retransmissions": 0,
        "RTT": "92ms",
        "bandwidth": "13Mbps",
        "creation_time": "2024-04-09 17:15:56",
        "duration": "439s",
        "packet_count": 9556,
        "window_size": 8189,
        "protocol_type": "TCP",
        "traffic_type": "HTTP"
    },
    {
        "source_IP": "103.104.19.110",
        "destination_IP": "115.77.157.45",
        "source_port": 28660,
        "destination_port": 325,
        "connection_status": "SYN_SENT",
        "transferred_bytes": 71039,
        "retransmissions": 19,
        "RTT": "36ms",
        "bandwidth": "616Mbps",
        "creation_time": "2024-04-09 17:33:49",
        "duration": "940s",
        "packet_count": 4466,
        "window_size": 21615,
        "protocol_type": "TCP",
        "traffic_type": "HTTP"
    },
    {
        "source_IP": "18.87.124.74",
        "destination_IP": "87.199.40.51",
        "source_port": 7253,
        "destination_port": 333,
        "connection_status": "SYN_SENT",
        "transferred_bytes": 55927,
        "retransmissions": 14,
        "RTT": "170ms",
        "bandwidth": "532Mbps",
        "creation_time": "2024-04-09 17:50:53",
        "duration": "3280s",
        "packet_count": 1786,
        "window_size": 19452,
        "protocol_type": "TCP",
        "traffic_type": "HTTPS"
    },
    {
        "source_IP": "128.227.90.224",
        "destination_IP": "118.46.82.111",
        "source_port": 46218,
        "destination_port": 428,
        "connection_status": "FIN_WAIT_1",
        "transferred_bytes": 8163,
        "retransmissions": 18,
        "RTT": "82ms",
        "bandwidth": "170Mbps",
        "creation_time": "2024-04-09 17:03:31",
        "duration": "1558s",
        "packet_count": 3722,
        "window_size": 11132,
        "protocol_type": "TCP",
        "traffic_type": "FTP"
    },
    {
        "source_IP": "217.212.69.160",
        "destination_IP": "86.189.114.192",
        "source_port": 21893,
        "destination_port": 247,
        "connection_status": "ESTABLISHED",
        "transferred_bytes": 91346,
        "retransmissions": 12,
        "RTT": "108ms",
        "bandwidth": "327Mbps",
        "creation_time": "2024-04-09 17:13:22",
        "duration": "895s",
        "packet_count": 7642,
        "window_size": 12947,
        "protocol_type": "TCP",
        "traffic_type": "FTP"
    },
    {
        "source_IP": "245.215.240.18",
        "destination_IP": "161.15.240.111",
        "source_port": 42951,
        "destination_port": 205,
        "connection_status": "ESTABLISHED",
        "transferred_bytes": 87454,
        "retransmissions": 10,
        "RTT": "138ms",
        "bandwidth": "539Mbps",
        "creation_time": "2024-04-09 17:35:38",
        "duration": "764s",
        "packet_count": 5141,
        "window_size": 20936,
        "protocol_type": "TCP",
        "traffic_type": "HTTP"
    },
    {
        "source_IP": "125.227.35.118",
        "destination_IP": "215.255.151.27",
        "source_port": 42764,
        "destination_port": 381,
        "connection_status": "SYN_SENT",
        "transferred_bytes": 45374,
        "retransmissions": 2,
        "RTT": "179ms",
        "bandwidth": "569Mbps",
        "creation_time": "2024-04-09 18:08:52",
        "duration": "886s",
        "packet_count": 2099,
        "window_size": 21007,
        "protocol_type": "TCP",
        "traffic_type": "FTP"
    },
    {
        "source_IP": "105.188.238.65",
        "destination_IP": "110.186.61.97",
        "source_port": 40486,
        "destination_port": 405,
        "connection_status": "CLOSED",
        "transferred_bytes": 83157,
        "retransmissions": 6,
        "RTT": "153ms",
        "bandwidth": "1000Mbps",
        "creation_time": "2024-04-09 17:17:12",
        "duration": "382s",
        "packet_count": 5166,
        "window_size": 36521,
        "protocol_type": "TCP",
        "traffic_type": "HTTPS"
    },
    {
        "source_IP": "167.86.19.226",
        "destination_IP": "156.157.42.37",
        "source_port": 3322,
        "destination_port": 313,
        "connection_status": "SYN_SENT",
        "transferred_bytes": 65461,
        "retransmissions": 0,
        "RTT": "181ms",
        "bandwidth": "558Mbps",
        "creation_time": "2024-04-09 18:22:38",
        "duration": "2851s",
        "packet_count": 7626,
        "window_size": 29726,
        "protocol_type": "TCP",
        "traffic_type": "HTTP"
    },
    {
        "source_IP": "120.48.106.236",
        "destination_IP": "155.45.3.159",
        "source_port": 15208,
        "destination_port": 273,
        "connection_status": "ESTABLISHED",
        "transferred_bytes": 23788,
        "retransmissions": 16,
        "RTT": "51ms",
        "bandwidth": "750Mbps",
        "creation_time": "2024-04-09 16:51:25",
        "duration": "908s",
        "packet_count": 5905,
        "window_size": 62570,
        "protocol_type": "TCP",
        "traffic_type": "FTP"
    },
    {
        "source_IP": "195.187.127.64",
        "destination_IP": "125.33.16.146",
        "source_port": 28417,
        "destination_port": 88,
        "connection_status": "SYN_SENT",
        "transferred_bytes": 10126,
        "retransmissions": 15,
        "RTT": "18ms",
        "bandwidth": "550Mbps",
        "creation_time": "2024-04-09 16:29:57",
        "duration": "2771s",
        "packet_count": 220,
        "window_size": 9442,
        "protocol_type": "TCP",
        "traffic_type": "HTTPS"
    },
    {
        "source_IP": "99.234.218.91",
        "destination_IP": "130.233.56.145",
        "source_port": 3096,
        "destination_port": 337,
        "connection_status": "ESTABLISHED",
        "transferred_bytes": 95296,
        "retransmissions": 5,
        "RTT": "197ms",
        "bandwidth": "580Mbps",
        "creation_time": "2024-04-09 18:22:48",
        "duration": "3170s",
        "packet_count": 303,
        "window_size": 64572,
        "protocol_type": "TCP",
        "traffic_type": "HTTPS"
    },
    {
        "source_IP": "74.220.185.126",
        "destination_IP": "134.168.238.156",
        "source_port": 29121,
        "destination_port": 367,
        "connection_status": "ESTABLISHED",
        "transferred_bytes": 81798,
        "retransmissions": 10,
        "RTT": "108ms",
        "bandwidth": "805Mbps",
        "creation_time": "2024-04-09 18:07:13",
        "duration": "1506s",
        "packet_count": 9390,
        "window_size": 38020,
        "protocol_type": "TCP",
        "traffic_type": "HTTP"
    }
]
var init = {
	eventList: function () {
		var $pathList = $('#implantation').contents().find('#pathList').find('path');
		$('body').on('click', '#close', this.closeOnClick);
		$pathList.on('mouseover', this.pathListMouseover);
		$pathList.on('mouseout', this.pathListMouseout);
	},

	pathListMouseout: function () {
		var $this = $(this);
		var uid = $this.attr('id');
		$('#' + uid).find('.tips').hide();
	},
	fillList: function () {
		var $orderItems = $('#orderItems');
		var orderItemsArr = [];
		$.each(data, function (ind, key) {
			orderItemsArr.push(
				'<ul class="rolling rolStyle", style="width:101%">',
				'<li>', key.source_IP, '</li>',
				'<li>', key.destination_IP, '</li>',
				'<li>', key.source_port, '</li>',
				'<li>', key.destination_port, '</li>',
				'<li>', key.connection_status, '</li>',
                '<li>', key.transferred_bytes, '</li>',
				'<li>', key.retransmissions, '</li>',
				'<li>', key.RTT, '</li>',
				'<li>', key.bandwidth, '</li>',
				'<li>', key.creation_time, '</li>',
                '<li>', key.duration, '</li>',
				'<li>', key.packet_count, '</li>',
				'<li>', key.window_size, '</li>',
				'<li>', key.protocol_type, '</li>',
				'<li>', key.traffic_type, '</li>',
				'</ul>'
			)
		})
		$orderItems.html(orderItemsArr.join(''));
	}
}
init.fillList();
init.eventList();

function echarts_3() {
    var myChart = echarts.init(document.getElementById('echart3'));
    option1 = {
        //  backgroundColor: '#00265f',
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'shadow'
            }
        },

        legend: {
            data: ['TCP传输速率'],
            top: '5%',
            textStyle: {
                color: "#fff",
                fontSize: '12',
            },
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
            data: ['2', '4', '6', '8', '10', '12', '14', '16', '18', '20', '22', '24'],
            axisLine: {
                show: true,
                lineStyle: {
                    color: "rgba(255,255,255,.1)",
                    width: 1,
                    type: "solid"
                },
            },
            axisTick: {
                show: false,
            },
            axisLabel: {
                interval: 0,
                // rotate:50,
                show: true,
                //  splitNumber: 2,
                textStyle: {
                    color: "rgba(255,255,255,.6)",
                    fontSize: '12',
                },
            },
        }],
        yAxis: [
            {
                type: 'value',
                axisLabel: {
                    formatter: '{value} Mbps',
                    show: true,
                    textStyle: {
                        color: "rgba(255,255,255,.6)",
                        fontSize: '12',
                    },
                },
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
                name: 'TCP传输速率',
                type: 'line',
                smooth: true,
                data: [5, 7, 6, 4, 5, 12, 8, 2, 6, 4, 5, 12],
                barWidth: '15',
                // barGap: 1,
                itemStyle: {
                    normal: {
                        color: '#62c98d',
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
            show: true,
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
            name: '总体网络状态',
            type: 'gauge',
            center: ['18%', '50%'],
            z: 3,
            min: 0,
            max: 20,
            splitNumber: 10,
            radius: '80%',
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
                name: '总体网络状态'
            }]
        },
        {
            name: '活跃连接数',
            type: 'gauge',
            center: ['50%', '50%'],
            z: 3,
            min: 0,
            max: 8,
            splitNumber: 8,
            radius: '80%',
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
                name: '活跃连接数'
            }]
        },
        {
            name: '平均延迟',
            type: 'gauge',
            center: ['82%', '50%'],
            z: 3,
            min: 0,
            max: 1,
            splitNumber: 5,
            radius: '80%',
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
                name: '平均延迟'
            }]
        }
]
    };
    dbj.setOption(option);
    $(document).ready(function () {
        dbj.resize()
    });
    window.addEventListener("resize", function () {
        dbj.resize()
    })
}

setInterval(dbj, 2000)
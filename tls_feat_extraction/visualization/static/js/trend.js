$(document).ready(function () {
    var whei = $(window).width()
    $("html").css({ fontSize: whei / 20 })
    $(window).resize(function () {
        var whei = $(window).width()
        $("html").css({ fontSize: whei / 20 })
    });
    echarts_1();
    echarts_2();
    echarts_3();
    echarts_4();
    echarts_5();
    echarts_6();
})

function echarts_1() {
    var myChart = echarts.init(document.getElementById('echart1'));
    option1 = {
        //  backgroundColor: '#00265f',
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'shadow'
            }
        },
        legend: {
            data: ['load 1m', 'load 5m',  'load 15m'],
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
                    formatter: '{value} %',
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
                name: 'load 1m',
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
            },
            {
                name: 'load 5m',
                type: 'line',
                smooth: true,
                data: [7, 11, 8,  2, 6, 4, 5, 12, 13, 10, 13, 10],

                itemStyle: {
                    normal: {
                        color: '#ffc000',
                        opacity: 1,

                        barBorderRadius: 5,
                    }
                }
            },
            {
                name: 'load 15m',
                type: 'line',
                smooth: true,
                data: [0, 1, 2,  2, 2, 1, 2, 1, 1, 0, 1, 1],

                itemStyle: {
                    normal: {
                        color: '#a2cc00',
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

function echarts_2() {
    var myChart = echarts.init(document.getElementById('echart2'));
    option1 = {
        //  backgroundColor: '#00265f',
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'shadow'
            }
        },

        legend: {
            data: ['响应时间'],
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
                    formatter: '{value} s',
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
                name: '响应时间',
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
            data: ['cpu使用率'],
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
                    formatter: '{value} %',
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
                name: 'cpu使用率',
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

function echarts_4() {
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
            data: ['内存利用率'],
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
                    formatter: '{value} %',
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
                name: '内存利用率',
                type: 'line',
                smooth: true,
                data: [7, 11, 8,  2, 6, 4, 5, 12, 13, 10, 13, 10],

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

function echarts_5() {
    var myChart = echarts.init(document.getElementById('echart5'));
    option1 = {
        //  backgroundColor: '#00265f',
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'shadow'
            }
        },
        legend: {
            data: ['系统吞吐量'],
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
                    //formatter: '{value} %'
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
                name: '系统吞吐量',
                type: 'line',
                smooth: true,
                data: [7, 11, 8,  2, 6, 4, 5, 12, 13, 10, 13, 10],

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

function echarts_6() {
    var myChart = echarts.init(document.getElementById('echart6'));
    option1 = {
        //  backgroundColor: '#00265f',
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'shadow'
            }
        },
        legend: {
            data: ['公网带宽', '内网带宽'],
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
                name: '公网带宽',
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
            },
            {
                name: '内网带宽',
                type: 'line',
                smooth: true,
                data: [7, 11, 8,  2, 6, 4, 5, 12, 13, 10, 13, 10],

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

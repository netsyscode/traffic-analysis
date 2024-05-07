// 产量溯源
var day_cl = document.getElementsByClassName('day_cl')[0]
var month_cl = document.getElementsByClassName('month_cl')[0]
var year_cl = document.getElementsByClassName('year_cl')[0]
var span_cl1 = document.getElementsByClassName('span_cl1')[0]
var span_cl2 = document.getElementsByClassName('span_cl2')[0]
var span_cl3 = document.getElementsByClassName('span_cl3')[0]
var span_cl4 = document.getElementsByClassName('span_cl4')[0]
// console.log(span_cl1.getAttribute("data-value"));

cldata = [
    // 日
    [10, 10, 10, 10],
    // 月
    [300, 300, 300, 300],
    // 年
    [3650, 3650, 3650, 3650]
]

// span_cl1.innerHTML = cldata[0][0]
// span_cl2.innerHTML = cldata[0][1]
// span_cl3.innerHTML = cldata[0][2]
// span_cl4.innerHTML = cldata[0][3]

function cl1() {
    day_cl.classList.add('span_c')
    month_cl.classList.remove('span_c')
    year_cl.classList.remove('span_c')
    span_cl1.setAttribute("data-value", cldata[0][0])
    span_cl2.setAttribute("data-value", cldata[0][1])
    span_cl3.setAttribute("data-value", cldata[0][2])
    span_cl4.setAttribute("data-value", cldata[0][3])
    init();
}

function cl3() {
    day_cl.classList.remove('span_c')
    month_cl.classList.remove('span_c')
    year_cl.classList.add('span_c')
    span_cl1.setAttribute("data-value", cldata[2][0])
    span_cl2.setAttribute("data-value", cldata[2][1])
    span_cl3.setAttribute("data-value", cldata[2][2])
    span_cl4.setAttribute("data-value", cldata[2][3])
    init();
}


function cl2() {
    day_cl.classList.remove('span_c')
    month_cl.classList.add('span_c')
    year_cl.classList.remove('span_c')
    span_cl1.setAttribute("data-value", cldata[1][0])
    span_cl2.setAttribute("data-value", cldata[1][1])
    span_cl3.setAttribute("data-value", cldata[1][2])
    span_cl4.setAttribute("data-value", cldata[1][3])
    init();
}

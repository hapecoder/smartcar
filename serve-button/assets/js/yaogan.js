$(function () {

    function init() {
        joystickdraw(BIG_CIRCUIT_R + POS_X, BIG_CIRCUIT_R + POS_Y);   //注意该参数是不考虑小圆R
    }
    /*
    x，y，输出值：都是以大圆圆心为零点的值
    输出是限制在大圆半径内的 xy组合
     */
    function getXY(x, y) {

        let mouse2centerlength = Math.sqrt(x * x + y * y);
        let newx = x;
        let newy = y;
        if (mouse2centerlength > BIG_CIRCUIT_R) {
            let proportion = mouse2centerlength / BIG_CIRCUIT_R;
            newx = x / proportion;
            newy = y / proportion;
        }
        return {x: newx, y: newy};
    }

    function joystickdraw(posx, posy) {

        //$('#xvalue').text(posx+","+posy);


        //这里绘图
        //清楚画面
        ctx.clearRect(0, 0, 600, 600);
        //保存转态
        ctx.save();
        //绘底图
        ctx.drawImage(img2, POS_X, POS_Y, BIG_CIRCUIT_R * 2, BIG_CIRCUIT_R * 2);
        //平移，根据鼠标位置
        let pos = getXY(posx - POS_X - BIG_CIRCUIT_R, posy - POS_Y - BIG_CIRCUIT_R);  //这里去掉偏移值
        //console.log(pos);
        output.x = parseInt(pos.x / BIG_CIRCUIT_R * 1000);
        output.y = parseInt(pos.y / BIG_CIRCUIT_R * 1000);
        // sendwebsocket(output);
        // $('#xvalue').text(output.x+","+output.y);
        console.log(output)

        //console.log(output);
        ctx.translate(pos.x + POS_X + BIG_CIRCUIT_R - SAMLL_CIRCUIT_R, pos.y + POS_Y + BIG_CIRCUIT_R - SAMLL_CIRCUIT_R);
        //会上球
        ctx.drawImage(img1, 0, 0, SAMLL_CIRCUIT_R * 2, SAMLL_CIRCUIT_R * 2);
        //还原状态
        ctx.restore();
    }

    function sendwebsocket(pos) {
        if (ws) {
            if (ws.readyState === WebSocket.OPEN)
                ws.send(JSON.stringify(pos))
        }
    }

    function dragstart_event(e) {
        mousedown = true;
        joystickdraw(e.offsetX, e.offsetY)
    }

    function dragging_event(e) {
        if (mousedown) {
            joystickdraw(e.offsetX, e.offsetY)
        }
    }

    function dragstop_event() {
        mousedown = false;
        init();
    }

    function touchstart_event(e) {
        mousedown = true;
        joystickdraw(e.touches[0].clientX, e.touches[0].clientY)
    }

    function touching_event(e) {
        if (mousedown) {
            joystickdraw(e.touches[0].clientX, e.touches[0].clientY)
        }
    }

    function touchstop_event() {
        mousedown = false;
        init();
    }

    

    let mousedown = false;
    let img1 = document.getElementById('smallcircuit');
    let img2 = document.getElementById('bigcircuit');
    let mycanvas = document.getElementById('canvas');
    let ctx = mycanvas.getContext('2d');

    let POS_X = 20; //因为触控区域肯定比较大，但是真正空间区域比较小，所有要有一个位置参数
    let POS_Y = 20;
    let SAMLL_CIRCUIT_R = 20; //小圆半径
    let BIG_CIRCUIT_R = 80;//大圆半径
    let output = {}; //摇杆输出值， -1000~1000 -1000~1000 这样好统一！！！

    mycanvas.onmousedown = dragstart_event;
    mycanvas.onmousemove = dragging_event;
    mycanvas.onmouseup = dragstop_event;

    mycanvas.ontouchstart = touchstart_event;
    mycanvas.ontouchmove = touching_event;
    mycanvas.ontouchend = touchstop_event;

    init();
});
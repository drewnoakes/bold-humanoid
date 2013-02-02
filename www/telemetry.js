var getWebSocketUrl = function()
{
  var pcol = "ws://",
      u = document.URL;
  if (u.substring(0, 4) === "http")
    u = u.substr(7);
  u = u.split('/');
  return pcol + u[0];
};

var openSocket = function(protocol)
{
  var socket = typeof MozWebSocket !== "undefined"
    ? new MozWebSocket(getWebSocketUrl(), protocol)
    : new WebSocket(getWebSocketUrl(), protocol);
  var trafficLight = $('<div></div>').text(protocol);
  trafficLight.css({'background-color':'yellow'});
  $('#sockets').append(trafficLight);
  socket.onopen = function()
  {
    trafficLight.css({'background-color':'green'});
  };
  socket.onclose = function()
  {
    trafficLight.css({'background-color':'red'});
  };
  socket.onerror = function(e)
  {
    console.error('ERROR', protocol, e);
  };
  return socket;
}

var initialiseIncrementProtocol = function()
{
  var socket = openSocket("dumb-increment-protocol");

  $('#resetCounter').click(function()
  {
    socket.send("reset\n");
  });

  socket.onmessage = function(msg)
  {
    document.getElementById("number").textContent = msg.data + "\n";
  };
};

var initialiseMirrorProtocol = function()
{
  var isDown = false,
      hasLast = false,
      lastX = 0,
      lastY = 0,
      color = "#000000";

  $('#color').change(function()
  {
    color = document.getElementById("color").value;
  });

  var socket = openSocket("lws-mirror-protocol");

  socket.onmessage = function(msg)
  {
    j = msg.data.split(';');
    f = 0;
    while (f < j.length - 1) {
      i = j[f].split(' ');
      if (i[0] === 'd') {
        ctx.strokeStyle = i[1];
        ctx.beginPath();
        ctx.moveTo(+(i[2]), +(i[3]));
        ctx.lineTo(+(i[4]), +(i[5]));
        ctx.stroke();
      }
      if (i[0] === 'c') {
        ctx.strokeStyle = i[1];
        ctx.beginPath();
        ctx.arc(+(i[2]), +(i[3]), +(i[4]), 0, Math.PI*2, true);
        ctx.stroke();
      }

      f++;
    }
  };

  var canvas = document.createElement('canvas');
  canvas.height = 300;
  canvas.width = 480;
  var ctx = canvas.getContext("2d");

  document.getElementById('wslm_drawing').appendChild(canvas);

  // Find position of canvas in document
  var offsetX = 0,
      offsetY = 0,
      element = canvas;

  if (element.offsetParent) {
    do {
      offsetX += element.offsetLeft;
      offsetY += element.offsetTop;
    } while ((element = element.offsetParent));
  }

  canvas.addEventListener('mousedown', function()
  {
    isDown = true;
  });

  canvas.addEventListener('mouseup', function()
  {
    isDown = false;
    hasLast = false;
  });

  canvas.addEventListener('mousemove', function(ev)
  {
    var x = typeof(ev.offsetX) !== undefined ? ev.offsetX : ev.layerX - offsetX,
        y = typeof(ev.offsetY) !== undefined ? ev.offsetY : ev.layerY - offsetY;

    if (!isDown)
      return;

    if (!hasLast)
    {
      hasLast = true;
      lastX = x;
      lastY = y;
      return;
    }

    socket.send("d " + color + " " + lastX + " " + lastY + " " + x + " " + y + ";");

    lastX = x;
    lastY = y;
  });
};

var initialiseTiming = function()
{
  var chart = new SmoothieChart({
    grid: {
      strokeStyle:'rgb(125, 0, 0)',
      fillStyle:'rgb(60, 0, 0)',
      lineWidth: 1,
      millisPerLine: 250,
      verticalSections: 6
    },
    labels: {
      fillStyle:'rgb(60, 0, 0)'
    }
  });

  var line1 = new TimeSeries();
  chart.addTimeSeries(line1, { strokeStyle:'rgb(0, 255, 0)', fillStyle:'rgba(0, 255, 0, 0.4)', lineWidth:4 });
  chart.streamTo(document.getElementById("timing-chart"));

  var socket = openSocket("timing-protocol");

  socket.onmessage = function(msg)
  {
    // TODO parse values
    // data of format "timestamp_ms_long|duration_ms_float"
    var matches = /^([0-9]+)\|([0-9.]+)/.exec(msg.data);
    if (matches) {
      var time = new Date().getTime(), //parseInt(matches[1]),
          value = parseFloat(matches[2])/1000000.0;
      console.log(time, value);
      line1.append(time, value);
    }
  }
};

var initialiseGameControl = function()
{
  var socket = openSocket("game-control-protocol");

  socket.onmessage = function(msg)
  {
    $('#secondsRemaining').text(msg.data);
  }
};

$(document).ready(function()
{
  BrowserDetect.init();

  document.getElementById("brow").textContent = " " + BrowserDetect.browser + " " + BrowserDetect.version + " " + BrowserDetect.OS;

  document.getElementById("websocket-url").textContent = getWebSocketUrl();

  initialiseIncrementProtocol();

  initialiseMirrorProtocol();

  initialiseTiming();

  initialiseGameControl();
});

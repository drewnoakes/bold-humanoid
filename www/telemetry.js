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

var initialiseTiming = function()
{
  var chart = new SmoothieChart({
    grid: {
      strokeStyle:'rgb(125, 0, 0)',
      fillStyle:'rgb(0, 0, 0)',
      lineWidth: 1,
      millisPerLine: 250,
      verticalSections: 6
    },
    labels: {
      fillStyle:'#ffffff'
    },
    minValue: 0
  });

  var line1 = new TimeSeries();
  var line2 = new TimeSeries();
  var line3 = new TimeSeries();
  chart.addTimeSeries(line1, { strokeStyle:'rgb(255, 0, 0)', fillStyle:'rgba(255, 0, 0, 0.4)', lineWidth:1 });
  chart.addTimeSeries(line2, { strokeStyle:'rgb(0, 255, 0)', fillStyle:'rgba(0, 255, 0, 0.4)', lineWidth:1 });
  chart.addTimeSeries(line3, { strokeStyle:'rgb(0, 0, 255)', fillStyle:'rgba(0, 0, 255, 0.4)', lineWidth:1 });
  chart.streamTo(document.getElementById("timing-chart"));

  var socket = openSocket("timing-protocol");

  socket.onmessage = function(msg)
  {
    // TODO parse values
    var matches = /^([0-9.]+)\|([0-9.]+)\|([0-9.]+)/.exec(msg.data);
    if (matches) {
      var time = new Date().getTime(),
          value1 = parseFloat(matches[1]),
          value2 = parseFloat(matches[2]),
          value3 = parseFloat(matches[3]);
      line1.append(time, value1);
      line2.append(time, value2);
      line3.append(time, value3);
    }
  }
};

var initialiseGameState = function()
{
  var socket = openSocket("game-state-protocol");

  socket.onmessage = function(msg)
  {
    $('#secondsRemaining').text(msg.data);
  }
};

var initialiseAgentModel = function()
{
  var options = {
    grid: {
      strokeStyle:'rgb(125, 0, 0)',
      fillStyle:'rgb(0, 0, 0)',
      lineWidth: 1,
      millisPerLine: 250,
      verticalSections: 6
    },
    labels: {
      fillStyle:'#ffffff'
    }
  };

  var gyroX = new TimeSeries();
  var gyroY = new TimeSeries();
  var gyroZ = new TimeSeries();
  var gyroChart = new SmoothieChart(options);
  gyroChart.addTimeSeries(gyroX, { strokeStyle:'rgb(255, 0, 0)', lineWidth:1 });
  gyroChart.addTimeSeries(gyroY, { strokeStyle:'rgb(0, 255, 0)', lineWidth:1 });
  gyroChart.addTimeSeries(gyroZ, { strokeStyle:'rgb(0, 0, 255)', lineWidth:1 });
  gyroChart.streamTo(document.getElementById("gyro-chart"));

  var accX = new TimeSeries();
  var accY = new TimeSeries();
  var accZ = new TimeSeries();
  var accChart = new SmoothieChart(options);
  gyroChart.addTimeSeries(accX, { strokeStyle:'rgb(255, 0, 0)', lineWidth:1 });
  accChart.addTimeSeries(accY, { strokeStyle:'rgb(0, 255, 0)', lineWidth:1 });
  accChart.addTimeSeries(accZ, { strokeStyle:'rgb(0, 0, 255)', lineWidth:1 });
  accChart.streamTo(document.getElementById("acc-chart"));

  var socket = openSocket("agent-model-protocol");

  socket.onmessage = function(msg)
  {
    // TODO parse values
    var matches = /^([-0-9.]+)\|([-0-9.]+)\|([-0-9.]+)\|([-0-9.]+)\|([-0-9.]+)\|([-0-9.]+)/.exec(msg.data);
    if (matches) {
      var time = new Date().getTime(),
          gx = parseFloat(matches[1]),
          gy = parseFloat(matches[2]),
          gz = parseFloat(matches[3]),
          ax = parseFloat(matches[4]),
          ay = parseFloat(matches[5]),
          az = parseFloat(matches[6]);
      gyroX.append(time, gx);
      gyroY.append(time, gy);
      gyroZ.append(time, gz);
      accX.append(time, ax);
      accY.append(time, ay);
      accZ.append(time, az);
    }
  }
};

$(document).ready(function()
{
  document.getElementById("websocket-url").textContent = getWebSocketUrl();

  initialiseTiming();
  initialiseGameState();
  initialiseAgentModel();
});

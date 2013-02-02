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
      fillStyle:'rgb(60, 0, 0)',
      lineWidth: 1,
      millisPerLine: 250,
      verticalSections: 6
    },
    labels: {
      fillStyle:'rgb(60, 0, 0)'
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
    // data of format "timestamp_ms_long|duration_ms_float"
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

$(document).ready(function()
{
  document.getElementById("websocket-url").textContent = getWebSocketUrl();

  initialiseTiming();

  initialiseGameState();
});

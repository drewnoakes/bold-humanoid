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
  var connectionIndicator = $('<div></div>').addClass('connection-indicator connecting').attr({title:protocol});
  $('#sockets').append(connectionIndicator);
  socket.onopen = function()
  {
    connectionIndicator.attr({'class':'connection-indicator connected'});
  };
  socket.onclose = function()
  {
    connectionIndicator.attr({'class':'connection-indicator disconnected'});
  };
  socket.onerror = function(e)
  {
    connectionIndicator.attr({'class':'connection-indicator error'});
  };
  return socket;
};

var chartOptions = {
  grid: {
    strokeStyle:'rgb(40, 40, 40)',
    fillStyle:'rgb(0, 0, 0)',
    lineWidth: 1,
    millisPerLine: 250,
    verticalSections: 6
  },
  labels: {
    fillStyle:'#ffffff'
  }
};

var initialiseTiming = function()
{
  var timingOptions = JSON.parse(JSON.stringify(chartOptions)); // poor man's clone
  timingOptions.minValue = 0;
  var chart = new SmoothieChart(timingOptions);

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
  var gyroX = new TimeSeries();
  var gyroY = new TimeSeries();
  var gyroZ = new TimeSeries();
  var gyroChart = new SmoothieChart(chartOptions);
  gyroChart.addTimeSeries(gyroX, { strokeStyle:'rgb(255, 0, 0)', lineWidth:1 });
  gyroChart.addTimeSeries(gyroY, { strokeStyle:'rgb(0, 255, 0)', lineWidth:1 });
  gyroChart.addTimeSeries(gyroZ, { strokeStyle:'rgb(0, 0, 255)', lineWidth:1 });
  gyroChart.streamTo(document.getElementById("gyro-chart"));

  var accX = new TimeSeries();
  var accY = new TimeSeries();
  var accZ = new TimeSeries();
  var accChart = new SmoothieChart(chartOptions);
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

var fieldDimensions = {
  fieldX: 6.0,
  fieldY: 4.0,
  goalX: 0.5,
  goalY: 1.5,
  goalZ: 0.8,
  goalPostDiameter: 0.1,
  goalAreaX: 0.6,
  goalAreaY: 2.2,
  penaltyMarkDistance: 1.8,
  circleDiameter: 1.2,
  lineWidth:0.05,
  penaltyLineLength: 0.1,
  outerMarginMinimum: 0.7,
  ballDiameter: 0.067 // according to Wikipedia
};

var fieldCenterX, fieldCenterY, scale, minScale;

var drawFieldMap = function()
{
  var canvas = $('#field-map').get(0);
  var context = canvas.getContext('2d');

  context.save();

  context.fillStyle = '#008800';
  context.fillRect(0, 0, canvas.width, canvas.height);

  context.translate(fieldCenterX, fieldCenterY);

  // prepare to draw field lines
  context.lineWidth = fieldDimensions.lineWidth * scale;
  context.strokeStyle = '#ffffff';

  // center circle
  context.beginPath();
  context.arc(0, 0, scale * fieldDimensions.circleDiameter/2, 0, Math.PI*2, true);

  var halfCrossLengthScaled = scale * fieldDimensions.penaltyLineLength / 2;
  var penaltyX = scale * (fieldDimensions.fieldX/2 - fieldDimensions.penaltyMarkDistance);
  var penaltyInnerX = penaltyX - halfCrossLengthScaled;
  var penaltyOuterX = penaltyX + halfCrossLengthScaled;

  // center cross mark
  context.moveTo(-halfCrossLengthScaled, 0);
  context.lineTo(+halfCrossLengthScaled, 0);

  // left penalty mark
  context.moveTo(-penaltyInnerX, 0);
  context.lineTo(-penaltyOuterX, 0);
  context.moveTo(-penaltyX, halfCrossLengthScaled);
  context.lineTo(-penaltyX, -halfCrossLengthScaled);

  // right penalty mark
  context.moveTo(penaltyInnerX, 0);
  context.lineTo(penaltyOuterX, 0);
  context.moveTo(penaltyX, halfCrossLengthScaled);
  context.lineTo(penaltyX, -halfCrossLengthScaled);

  // outer square
  var x = scale * fieldDimensions.fieldX/2,
      y = scale * fieldDimensions.fieldY/2;
  context.strokeRect(-x, -y, scale * fieldDimensions.fieldX, scale * fieldDimensions.fieldY);

  context.moveTo(0, y);
  context.lineTo(0, -y);

  var goalAreaY = scale * fieldDimensions.goalAreaY / 2;

  // left goal area
  context.moveTo(-x, -goalAreaY);
  context.lineTo(-x + scale*fieldDimensions.goalAreaX, -goalAreaY);
  context.lineTo(-x + scale*fieldDimensions.goalAreaX, goalAreaY);
  context.lineTo(-x, goalAreaY);

  // right goal area
  context.moveTo(x, -goalAreaY);
  context.lineTo(x - scale*fieldDimensions.goalAreaX, -goalAreaY);
  context.lineTo(x - scale*fieldDimensions.goalAreaX, goalAreaY);
  context.lineTo(x, goalAreaY);

  context.stroke();

  var goalY = scale * fieldDimensions.goalY / 2;

  // TODO actually the position of these circles is WRONG! as is many of the lines -- the insides should be used, considering line width

  context.strokeStyle = 'yellow';

  context.beginPath();
  context.arc(+x, +goalY, scale * fieldDimensions.goalPostDiameter/2, 0, Math.PI*2, true);
  context.stroke();
  context.beginPath();
  context.arc(+x, -goalY, scale * fieldDimensions.goalPostDiameter/2, 0, Math.PI*2, true);
  context.stroke();
  context.beginPath();
  context.arc(-x, +goalY, scale * fieldDimensions.goalPostDiameter/2, 0, Math.PI*2, true);
  context.stroke();
  context.beginPath();
  context.arc(-x, -goalY, scale * fieldDimensions.goalPostDiameter/2, 0, Math.PI*2, true);
  context.stroke();

  context.beginPath();

  // left goal
  context.moveTo(-x, -goalY);
  context.lineTo(-x - scale*fieldDimensions.goalX, -goalY);
  context.lineTo(-x - scale*fieldDimensions.goalX, goalY);
  context.lineTo(-x, goalY);

  // right goal
  context.moveTo(x, -goalY);
  context.lineTo(x + scale*fieldDimensions.goalX, -goalY);
  context.lineTo(x + scale*fieldDimensions.goalX, goalY);
  context.lineTo(x, goalY);

  context.stroke();

  context.restore();
};

$(document).ready(function() {
  var $canvas = $('#field-map');
  var canvas = $canvas.get(0);

  fieldCenterX = canvas.width/2;
  fieldCenterY = canvas.height/2;
  scale = Math.min(
      canvas.width / (fieldDimensions.fieldX + 2*fieldDimensions.outerMarginMinimum),
      canvas.height / (fieldDimensions.fieldY + 2*fieldDimensions.outerMarginMinimum));
  minScale = scale * 0.8; // can't zoom out too far

  $canvas.on('mousewheel', function(event) {
    // TODO zoom around the mouse pointer, rather than (0,0)
    scale += event.originalEvent.wheelDelta / 20;
    scale = Math.max(minScale, scale);
    drawFieldMap();
  });

  var isMouseDown = false, dragStartX, dragStartY;
  $canvas.on('mousedown', function(event) {
    isMouseDown = true;
    dragStartX = event.screenX;
    dragStartY = event.screenY;
  });

  $canvas.on('mouseup', function() {
    isMouseDown = false;
  });

  $canvas.on('mousemove', function(event) {
    if (isMouseDown) {
      var dx = event.screenX - dragStartX,
          dy = event.screenY - dragStartY;
      dragStartX = event.screenX;
      dragStartY = event.screenY;
      fieldCenterX += dx;
      fieldCenterY += dy;
      drawFieldMap();
    }
  });

  document.getElementById("websocket-url").textContent = getWebSocketUrl();

  initialiseTiming();
  initialiseGameState();
  initialiseAgentModel();
  drawFieldMap();
});

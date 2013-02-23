/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/WebSocketFactory'
    ],
    function(WebSocketFactory)
    {
        WebSocketFactory.toString();

        var chartOptions = {
            grid: {
                strokeStyle: 'rgb(40, 40, 40)',
                fillStyle: 'rgb(0, 0, 0)',
                lineWidth: 1,
                millisPerLine: 250,
                verticalSections: 6,
                sharpLines: true
            },
            labels: {
                fillStyle: '#ffffff'
            }
        };

        var timingChartOptions = JSON.parse(JSON.stringify(chartOptions)); // poor man's clone
        timingChartOptions.minValue = 0;

        var protocolDefinitions = [
            {
                protocol: 'timing-protocol',
                charts: [
                    {
                        // TODO this should be a stacked chart
                        title: 'timing',
                        options: timingChartOptions,
                        series: [
                            { strokeStyle: 'rgb(255, 0, 0)', fillStyle: 'rgba(255, 0, 0, 0.4)', lineWidth: 1 },
                            { strokeStyle: 'rgb(0, 255, 0)', fillStyle: 'rgba(0, 255, 0, 0.4)', lineWidth: 1 },
                            { strokeStyle: 'rgb(0, 0, 255)', fillStyle: 'rgba(0, 0, 255, 0.4)', lineWidth: 1 },
                            { strokeStyle: 'rgb(0, 255, 255)', fillStyle: 'rgba(0, 255, 255, 0.4)', lineWidth: 1 }
                        ]
                    }
                ]
            },
            {
                protocol: 'agent-model-protocol',
                charts: [
                    {
                        title: 'gyro',
                        options: chartOptions,
                        series: [
                            { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 },
                            { strokeStyle: 'rgb(0, 255, 0)', lineWidth: 1 },
                            { strokeStyle: 'rgb(0, 0, 255)', lineWidth: 1 }
                        ]
                    },
                    {
                        title: 'acc',
                        options: chartOptions,
                        series: [
                            { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 },
                            { strokeStyle: 'rgb(0, 255, 0)', lineWidth: 1 },
                            { strokeStyle: 'rgb(0, 0, 255)', lineWidth: 1 }
                        ]
                    }
                ]
            }
        ];

        var chartWidth = 640,
            chartHeight = 120,
            parseFloats = function (s)
            {
                var bits = s.split('|');
                var floats = [];
                for (var i = 0; i < bits.length; i++) {
                    floats.push(parseFloat(bits[i]));
                }
                return floats;
            },
            container = $('#streaming-charts');

        _.each(protocolDefinitions, function(protocolDefinition)
        {
            var seriesArray = [];

            _.each(protocolDefinition.charts, function(chartDefinition)
            {
                var chart = new SmoothieChart(chartDefinition.options);

                container.append($('<h2></h2>', {text:chartDefinition.title}));
                var canvas = document.createElement('canvas');
                canvas.width = chartWidth;
                canvas.height = chartHeight;
                container.append(canvas);

                chart.streamTo(canvas, /*delayMs*/ 200);

                _.each(chartDefinition.series, function(seriesDefinition)
                {
                    var series = new TimeSeries();
                    chart.addTimeSeries(series, seriesDefinition);
                    seriesArray.push(series);
                });
            });
            
            var socket = WebSocketFactory.open(protocolDefinition.protocol);
            socket.onmessage = function (msg)
            {
                var time = new Date().getTime();
                var floats = parseFloats(msg.data);
                for (var f = 0; f < floats.length && f < seriesArray.length; f++) {
                    seriesArray[f].append(time, floats[f]);
                }
            }
        });
    }
);

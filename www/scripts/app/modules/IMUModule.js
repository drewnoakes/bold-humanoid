/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'Protocols',
        'PolarTrace'
    ],
    function(DataProxy, Protocols, PolarTrace)
    {
        'use strict';

        // TODO reuse RGB <==> XYZ colour coding on polar trace axes

        var chartOptions = {
            grid: {
                strokeStyle: 'rgb(40, 40, 40)',
                fillStyle: 'rgb(0, 0, 0)',
                lineWidth: 1,
                millisPerLine: 250,
                verticalSections: 6,
                sharpLines: true,
                borderVisible: false
            },
            labels: {
                fillStyle: '#ffffff'
            }
        };

        var red = '#ED303C',
            grn = '#44C425',
            blu = '#00A8C6';

        var charts = [
            {
                title: 'gyro',
                options: chartOptions,
                series: [
                    { strokeStyle: red, lineWidth: 1 },
                    { strokeStyle: grn, lineWidth: 1 },
                    { strokeStyle: blu, lineWidth: 1 }
                ]
            },
            {
                title: 'acc',
                options: chartOptions,
                series: [
                    { strokeStyle: red, lineWidth: 1 },
                    { strokeStyle: grn, lineWidth: 1 },
                    { strokeStyle: blu, lineWidth: 1 }
                ]
            }
        ];

        var chartHeight = 150;

        var IMUModule = function()
        {
            this.container = $('<div></div>');

            /////

            this.title = 'IMU';
            this.id = 'sensors';
            this.panes = [
                {
                    title: 'main',
                    element: this.container,
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        IMUModule.prototype.load = function()
        {
            this.seriesArray = [];
            this.chartCanvases = [];

            this.buildCharts();

            var addPolarTrace = function(name)
            {
                var trace = new PolarTrace(),
                    div = $('<div></div>', {'class':'polar-trace-container'});
                div.append($('<h2></h2>').text(name));
                div.append(trace.element);

                this.container.append(div);
                return trace;
            }.bind(this);

            this.polarTraceXY = addPolarTrace('X|Y');
            this.polarTraceYZ = addPolarTrace('Y|Z');
            this.polarTraceXZ = addPolarTrace('X|Z');

            this.subscription = DataProxy.subscribe(
                Protocols.hardwareState,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );
        };

        IMUModule.prototype.unload = function()
        {
            _.each(this.charts, function (chart) { chart.stop(); });

            this.container.empty();
            this.subscription.close();
        };

        IMUModule.prototype.buildCharts = function()
        {
            this.charts = [];
            _.each(charts, function(chartDefinition)
            {
                var chart = new SmoothieChart(chartDefinition.options);
                this.charts.push(chart);
                chart.options.yRangeFunction = function(range)
                {
                    // Find the greatest absolute value
                    var max = Math.max(Math.abs(range.min), Math.abs(range.max));
                    // Ensure we're viewing at least a quarter of the range, so that
                    // very small values don't appear exaggeratedly large
                    max = Math.max(max, 1);
                    return {min:-max, max:max};
                };
                chart.options.horizontalLines.push({color:'#ffffff', lineWidth: 1, value: 0});

                this.container.append($('<h2></h2>', {text:chartDefinition.title}));
                var canvas = document.createElement('canvas');
                canvas.width = 640;
                canvas.height = chartHeight;
                this.container.append(canvas);

                this.chartCanvases.push(canvas);

                chart.streamTo(canvas, /*delayMs*/ 200);

                _.each(chartDefinition.series, function(seriesDefinition)
                {
                    var series = new TimeSeries();
                    chart.addTimeSeries(series, seriesDefinition);
                    this.seriesArray.push(series);
                }.bind(this));
            }.bind(this));
        };

        IMUModule.prototype.onResized = function(width, height)
        {
            _.each(this.chartCanvases, function(canvas)
            {
                canvas.width = width;
            });
        };

        IMUModule.prototype.onData = function (data)
        {
            var time = new Date().getTime();

            // copy values to charts
            this.seriesArray[0].append(time, data.gyro[0]);
            this.seriesArray[1].append(time, data.gyro[1]);
            this.seriesArray[2].append(time, data.gyro[2]);
            this.seriesArray[3].append(time, data.acc[0]);
            this.seriesArray[4].append(time, data.acc[1]);
            this.seriesArray[5].append(time, data.acc[2]);

            this.polarTraceXY.addValue(data.acc[0], data.acc[1]);
            this.polarTraceYZ.addValue(data.acc[1], data.acc[2]);
            this.polarTraceXZ.addValue(data.acc[0], data.acc[2]);
        };

        return IMUModule;
    }
);

/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/DataProxy',
        'scripts/app/Protocols'
    ],
    function(DataProxy, Protocols)
    {
        'use strict';

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

        var charts = [
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
        ];

        var chartHeight = 200;

        var SensorModule = function()
        {
            this.container = $('<div></div>');
            this.seriesArray = [];
            this.chartCanvases = [];

            this.buildCharts();

            /////

            this.title = 'sensors';
            this.moduleClass = 'sensors';
            this.panes = [
                {
                    title: 'main',
                    element: this.container,
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        SensorModule.prototype.buildCharts = function()
        {
            var self = this;
            _.each(charts, function(chartDefinition)
            {
                var chart = new SmoothieChart(chartDefinition.options);
                chart.yRangeFunction = function(range)
                {
                    // Find the greatest absolute value
                    var max = Math.max(Math.abs(range.min), Math.abs(range.max));
                    // Ensure we're viewing at least a quarter of the range, so that
                    // very small values don't appear exaggeratedly large
                    max = Math.max(max, 1);
                    return {min:-max, max:max};
                };
                chart.options.horizontalLines.push({color:'#ffffff', lineWidth: 1, value: 0});

                self.container.append($('<h2></h2>', {text:chartDefinition.title}));
                var canvas = document.createElement('canvas');
                canvas.width = 640;
                canvas.height = chartHeight;
                self.container.append(canvas);

                self.chartCanvases.push(canvas);

                chart.streamTo(canvas, /*delayMs*/ 200);

                _.each(chartDefinition.series, function(seriesDefinition)
                {
                    var series = new TimeSeries();
                    chart.addTimeSeries(series, seriesDefinition);
                    self.seriesArray.push(series);
                });
            });
        };

        SensorModule.prototype.onResized = function(width, height)
        {
            _.each(this.chartCanvases, function(canvas)
            {
                canvas.width = width;
            });
        };

        SensorModule.prototype.load = function()
        {
            this.subscription = DataProxy.subscribe(Protocols.agentModel, { onmessage: _.bind(this.onmessage, this) });
        };

        SensorModule.prototype.unload = function()
        {
            this.subscription.cancel();
        };

        SensorModule.prototype.onmessage = function (message)
        {
            var time = new Date().getTime(),
                floats = DataProxy.parseFloats(message.data);

            // copy values to charts
            for (var f = 0; f < floats.length && f < this.seriesArray.length; f++) {
                this.seriesArray[f].append(time, floats[f]);
            }
        };

        return SensorModule;
    }
);

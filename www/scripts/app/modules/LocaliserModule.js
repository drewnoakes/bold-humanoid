/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/DataProxy',
        'scripts/app/ControlBuilder',
        'scripts/app/Protocols'
    ],
    function(DataProxy, ControlBuilder, Protocols)
    {
        'use strict';

        var chartOptions = {
            grid: {
                strokeStyle: 'rgb(40, 40, 40)',
                fillStyle: 'rgb(0, 0, 0)',
                lineWidth: 1,
                millisPerLine: 250,
                verticalSections: 6,
                sharpLines: true,
                borderVisible:  false
            },
            minValue: 0,
            labels: {
                fillStyle: '#ffffff'
            }
        };

        var chartHeight = 150;

        var LocaliserModule = function()
        {
            this.$container = $('<div></div>');

            this.buildChart();

            this.title = 'Localiser';
            this.moduleClass = 'localiser';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container,
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        LocaliserModule.prototype.buildChart = function()
        {
            var chart = new SmoothieChart(chartOptions);

            this.chartCanvas = document.createElement('canvas');
            this.chartCanvas.width = 640;
            this.chartCanvas.height = chartHeight;
            this.$container.append(this.chartCanvas);

            chart.streamTo(this.chartCanvas, /*delayMs*/ 200);

            this.averageSeries = new TimeSeries();
            this.maxSeries = new TimeSeries();

            chart.addTimeSeries(this.averageSeries, {lineWidth:1, strokeStyle:'#0040ff', fillStyle:'rgba(0,64,255,0.26)'});
            chart.addTimeSeries(this.maxSeries, {lineWidth:1, strokeStyle:'#00ff00'});
        };

        LocaliserModule.prototype.onResized = function(width, height)
        {
            _.each(this.chartCanvases, function(canvas)
            {
                canvas.width = width;
            });
        };

        LocaliserModule.prototype.load = function()
        {
            this.subscription = DataProxy.subscribe(
                Protocols.particleState,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );

            ControlBuilder.build('localiser', $('<div></div>', {'class': 'control-container localiser-controls'}).appendTo(this.$container));
        };

        LocaliserModule.prototype.unload = function()
        {
            this.subscription.close();
        };

        LocaliserModule.prototype.onData = function (data)
        {
            var time = new Date().getTime();

            var count = 0, sum = 0, max = 0;
            for (var i = 0; i < data.length; i++)
            {
                var w = data[i][3];
                if (w !== 0)
                {
                    count++;
                    sum += w;
                    if (max < w)
                        max = w;
                }
            }

            if (count !== 0)
            {
                this.averageSeries.append(time, sum/count);
                this.maxSeries.append(time, max);
            }
        };

        return LocaliserModule;
    }
);

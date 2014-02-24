/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'ControlBuilder',
        'Protocols'
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

            this.title = 'localiser';
            this.id = 'localiser';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container,
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        LocaliserModule.prototype.load = function()
        {
            this.buildChart();

            this.particleSubscription = DataProxy.subscribe(Protocols.particleState,    { json: true, onmessage: _.bind(this.onParticleData, this) });
            this.cameraSubscription   = DataProxy.subscribe(Protocols.cameraFrameState, { json: true, onmessage: _.bind(this.onCameraData, this) });

            var container = $('<div></div>', {'class': 'control-container localiser-controls flow'}).appendTo(this.$container).get(0);
            ControlBuilder.actions('localiser', container);
            ControlBuilder.buildAll('localiser', container);

            // TODO show what is in the camera frame (ball, goals, lines...)
            this.$ballVisibleMarker = $('<div></div>').addClass('marker ball').appendTo(this.$container);
            this.$goal1VisibleMarker = $('<div></div>').addClass('marker goal').appendTo(this.$container);
            this.$goal2VisibleMarker = $('<div></div>').addClass('marker goal').appendTo(this.$container);
        };

        LocaliserModule.prototype.unload = function()
        {
            this.particleSubscription.close();
            this.cameraSubscription.close();
            this.chart.stop();
            this.$container.empty();
        };

        LocaliserModule.prototype.buildChart = function()
        {
            this.chart = new SmoothieChart(chartOptions);

            this.chartCanvas = document.createElement('canvas');
            this.chartCanvas.width = 640;
            this.chartCanvas.height = chartHeight;
            this.$container.append(this.chartCanvas);

            this.chart.streamTo(this.chartCanvas, /*delayMs*/ 200);

            this.averageSeries = new TimeSeries();
            this.maxSeries = new TimeSeries();

            this.chart.addTimeSeries(this.averageSeries, {lineWidth:1, strokeStyle:'#0040ff', fillStyle:'rgba(0,64,255,0.26)'});
            this.chart.addTimeSeries(this.maxSeries, {lineWidth:1, strokeStyle:'#00ff00'});
        };

        LocaliserModule.prototype.onResized = function(width, height)
        {
            this.chartCanvas.width = width;
        };

        LocaliserModule.prototype.onCameraData = function (data)
        {
            if (data.ball) {
                this.$ballVisibleMarker.addClass('visible');
            } else {
                this.$ballVisibleMarker.removeClass('visible');
            }

            switch (data.goals.length) {
                case 0: {
                    this.$goal1VisibleMarker.removeClass('visible');
                    this.$goal2VisibleMarker.removeClass('visible');
                    break;
                }
                case 1: {
                    this.$goal1VisibleMarker.addClass('visible');
                    this.$goal2VisibleMarker.removeClass('visible');
                    break;
                }
                default: {
                    this.$goal1VisibleMarker.addClass('visible');
                    this.$goal2VisibleMarker.addClass('visible');
                    break;
                }
            }
        };

        LocaliserModule.prototype.onParticleData = function (data)
        {
            var time = new Date().getTime(),
                particles = data.particles;

            var count = 0, sum = 0, max = 0;
            for (var i = 0; i < particles.length; i++)
            {
                var w = particles[i][3];
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

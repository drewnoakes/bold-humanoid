/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'Protocols'
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
                sharpLines: true,
                borderVisible: false
            },
            labels: {
                fillStyle: '#ffffff'
            },
            minValue: 0
        };

        var chartHeight = 150;

        var CommsModule = function()
        {
            this.$container = $('<div></div>');

            /////

            this.title = 'communication';
            this.id = 'comms';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        CommsModule.prototype.load = function()
        {
            this.chart = new SmoothieChart(chartOptions);
            this.canvas = document.createElement('canvas');
            this.canvas.height = chartHeight;
            this.$container.append(this.canvas);

            this.ignoreSeries = new TimeSeries();
            this.gameSeries = new TimeSeries();
            this.chart.addTimeSeries(this.ignoreSeries, { strokeStyle: 'rgb(128, 0, 0)', fillStyle: 'rgba(0, 255, 0, 0.3)', lineWidth: 1 });
            this.chart.addTimeSeries(this.gameSeries,   { strokeStyle: 'rgb(0, 0, 255)', fillStyle: 'rgba(0, 255, 0, 0.3)', lineWidth: 1 });
            this.chart.streamTo(this.canvas, /*delayMs*/ 100);

            this.subscription = DataProxy.subscribe(
                Protocols.debug,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );
        };

        CommsModule.prototype.unload = function()
        {
            this.chart.stop();
            this.$container.empty();
            this.subscription.close();
        };

        CommsModule.prototype.onData = function(data)
        {
            var time = new Date().getTime();

            this.ignoreSeries.append(time, data.ignoredMessages);
            this.gameSeries.append(time, data.gameControllerMessages);
        };

        CommsModule.prototype.onResized = function(width, height)
        {
            this.canvas.width = width;
        };

        return CommsModule;
    }
);

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

        // TODO allow resetting maximums
        // TODO reorder table rows lexicographically

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

        var seriesOptions = {
            strokeStyle: 'rgb(0, 255, 0)',
            fillStyle: 'rgba(0, 255, 0, 0.3)',
            lineWidth: 1
        };

        var chartHeight = 150;

        var TimingModule = function()
        {
            this.$container = $('<div></div>');

            /////

            this.title = 'timing';
            this.id = 'timing';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        TimingModule.prototype.load = function()
        {
            this.chart = new SmoothieChart(chartOptions);
            this.canvas = document.createElement('canvas');
            this.canvas.height = chartHeight;
            this.$container.append(this.canvas);

            this.series = new TimeSeries();
            this.chart.addTimeSeries(this.series, seriesOptions);
            this.chart.streamTo(this.canvas, /*delayMs*/ 100);
            this.chart.options.horizontalLines.push({color:'#FF0000', lineWidth: 1, value: 30});

            this.$fps = $('<div></div>', {'class':'fps'}).appendTo(this.$container);

            this.table = $('<table></table>', {'class':'timing-details'}).appendTo(this.$container);

            this.entryByLabel = {};

            this.subscription = DataProxy.subscribe(
                Protocols.debug,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );

            this.fpsInterval = setInterval(function ()
            {
                this.$fps.text(this.fpsCount ? this.fpsCount + ' FPS' : '');
                this.fpsCount = 0;
            }.bind(this), 1000);
        };

        TimingModule.prototype.unload = function()
        {
            this.chart.stop();
            this.$container.empty();
            this.subscription.close();

            clearInterval(this.fpsInterval);
            delete this.fpsInterval;
        };

        TimingModule.prototype.onData = function(data)
        {
            this.fpsCount++;
            var time = new Date().getTime();

            _.each(_.keys(data.timings), function (key)
            {
                var millis = data.timings[key];
                this.getOrCreateEntry(key).update(time, millis);
            }.bind(this));

            this.updateChart(time);
        };

        TimingModule.prototype.onResized = function(width, height)
        {
            this.canvas.width = width;
        };

        TimingModule.prototype.updateChart = function(time)
        {
            var totalTime = 0;

            _.each(_.values(this.entryByLabel), function(entry)
            {
                if (entry.time === time && entry.label.indexOf('/') === -1)
                    totalTime += entry.millis;
            });

            this.series.append(time, totalTime);
        };

        TimingModule.prototype.getOrCreateEntry = function(label)
        {
            var entry = this.entryByLabel[label];
            if (!entry) {
                var row = $('<tr></tr>').appendTo(this.table);

                $('<td></td>').text(label).appendTo(row);
                var cellMillis = $('<td></td>', {'class': 'duration'}).appendTo(row),
                    cellMaxMillis = $('<td></td>', {'class': 'max-duration'}).appendTo(row);

                entry = {
                    label: label,
                    update: function(timestamp, millis) {
                        entry.time = timestamp;
                        entry.millis = millis;
                        if (!entry.maxMillis || entry.maxMillis < millis)
                        {
                            entry.maxMillis = millis;
                            cellMaxMillis.text(millis.toFixed(3));
                        }
                        cellMillis.text(millis.toFixed(3));
                    }
                };

                this.entryByLabel[label] = entry;
            }
            return entry;
        };

        return TimingModule;
    }
);

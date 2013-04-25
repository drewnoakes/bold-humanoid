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
        var parseRegex = /([^=|]+)=([^|]+)\|/g;

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
            var canvas = document.createElement('canvas');
            canvas.height = chartHeight;
            this.$container.append(canvas);

            var series = new TimeSeries();
            this.chart.addTimeSeries(series, seriesOptions);
            this.chart.streamTo(canvas, /*delayMs*/ 100);
            this.chart.options.horizontalLines.push({color:'#FF0000', lineWidth: 1, value: 30});

            this.table = $('<table></table>', {'class':'timing-details'}).appendTo(this.$container);

            this.series = series;
            this.canvas = canvas;
            this.entryByLabel = {};

            this.subscription = DataProxy.subscribe(
                Protocols.timing,
                {
                    json: false,
                    onmessage: _.bind(this.onmessage, this)
                }
            );
        };

        TimingModule.prototype.unload = function()
        {
            this.chart.stop();
            this.$container.empty();
            this.subscription.close();
        };

        TimingModule.prototype.onmessage = function(msg)
        {
            var time = new Date().getTime(),
                match;

            while (match = parseRegex.exec(msg.data)) {
                this.getOrCreateEntry(match[1])
                    .update(time, parseFloat(match[2]));
            }

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

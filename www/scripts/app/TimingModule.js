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
            },
            minValue: 0
        };

        var seriesOptions = {
            strokeStyle: 'rgb(0, 255, 0)',
            fillStyle: 'rgba(0, 255, 0, 0.4)',
            lineWidth: 1
        };

        var chartWidth = 640,
            chartHeight = 120,
            container = $('#timing'),
            chart = new SmoothieChart(chartOptions),
            canvas = document.createElement('canvas');

        canvas.width = chartWidth;
        canvas.height = chartHeight;
        container.append(canvas);

        var series = new TimeSeries();
        chart.addTimeSeries(series, seriesOptions);
        chart.streamTo(canvas, /*delayMs*/ 100);

        var updateChart = function(time)
        {
            var totalTime = 0;

            _.each(_.values(entryByLabel), function(entry)
            {
                if (entry.time === time && entry.label.indexOf('/') === -1)
                    totalTime += entry.millis;
            });

            series.append(time, totalTime);
        };

        var entryByLabel = {};
        var table = $('<table></table>', {'class':'timing-details'}).appendTo(container);

        var getOrCreateEntry = function(label)
        {
            var entry = entryByLabel[label];
            if (!entry) {
                var row = $('<tr></tr>').appendTo(table);

                $('<td></td>').text(label).appendTo(row);
                var timeCell = $('<td></td>').appendTo(row);

                entry = {
                    label: label,
                    update: function(time, durationMillis) {
                        entry.time = time;
                        entry.millis = durationMillis;
                        timeCell.text(durationMillis.toFixed(3));
                    }
                };

                entryByLabel[label] = entry;
            }
            return entry;
        };

        var parseRegex = /([^=|]+)=([^|]+)\|/g;

        // TODO when the concept of Modules are introduced, track this subscription for module unload
        var subscription = DataProxy.subscribe(
            Protocols.timing,
            {
                onmessage: function(msg)
                {
                    var time = new Date().getTime(),
                        match;

                    while (match = parseRegex.exec(msg.data)) {
                        getOrCreateEntry(match[1]).update(time, parseFloat(match[2]));
                    }

                    updateChart(time);
                }
            }
        );
    }
);

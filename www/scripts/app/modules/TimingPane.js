/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy'
    ],
    function (DataProxy)
    {
        var seriesOptions = {
            strokeStyle: 'rgb(0, 255, 0)',
            fillStyle: 'rgba(0, 255, 0, 0.3)',
            lineWidth: 1
        };

        var chartHeight = 150;

        var TimingPane = function (protocol, thresholdMillis)
        {
            this.$container = $('<div></div>');
            this.container = this.$container.get(0);
            this.protocol = protocol;
            this.thresholdMillis = thresholdMillis;
        };

        TimingPane.prototype.load = function ()
        {
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
                yRangeFunction: function (range)
                {
                    return {min: 0, max: Math.max(this.thresholdMillis, range.max)};
                }.bind(this),
                horizontalLines: [
                    {color: '#FF0000', lineWidth: 1, value: this.thresholdMillis}
                ]
            };

            this.chart = new SmoothieChart(chartOptions);
            this.canvas = document.createElement('canvas');
            this.canvas.height = chartHeight;
            this.$container.append(this.canvas);

            this.series = new TimeSeries();
            this.chart.addTimeSeries(this.series, seriesOptions);
            this.chart.streamTo(this.canvas, /*delayMs*/ 0);

            this.$fps = $('<div></div>', {'class': 'fps'}).appendTo(this.$container);
            $('<a></a>', {'class': 'reset', href: '#'}).text('reset maximums').click(function ()
            {
                this.resetMaximums();
                return false;
            }.bind(this)).appendTo(this.$container);

            this.table = $('<table></table>', {'class': 'timing-details'}).appendTo(this.$container);

            this.entryByLabel = {};

            this.subscription = DataProxy.subscribe(
                this.protocol,
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

        TimingPane.prototype.unload = function ()
        {
            this.chart.stop();
            this.$container.empty();
            this.subscription.close();

            clearInterval(this.fpsInterval);
            delete this.fpsInterval;

            delete this.series;
            delete this.chart;
            delete this.table;
            delete this.canvas;
            delete this.entryByLabel;
            delete this.subscription;
        };

        TimingPane.prototype.onData = function (data)
        {
            if (this.lastCycleNumber) {
                this.fpsCount += data.cycle - this.lastCycleNumber;
            }
            this.lastCycleNumber = data.cycle;
            var time = new Date().getTime();
            var timings = data.timings;

            _.each(_.keys(timings), function (key)
            {
                var millis = timings[key];
                this.getOrCreateEntry(key).update(time, millis);
            }.bind(this));

            this.updateChart(time);
        };

        TimingPane.prototype.onResized = function (width, height)
        {
            this.canvas.width = width;
        };

        TimingPane.prototype.updateChart = function (time)
        {
            var totalTime = 0;

            _.each(_.values(this.entryByLabel), function (entry)
            {
                if (entry.time === time && entry.label.indexOf('/') === -1)
                    totalTime += entry.millis;
            });

            this.series.append(time, totalTime);
        };

        TimingPane.prototype.resetMaximums = function ()
        {
            _.each(_.values(this.entryByLabel), function (entry)
            {
                entry.maxMillis = 0;
            });
        };

        TimingPane.prototype.getOrCreateEntry = function (label)
        {
            var entry = this.entryByLabel[label];
            if (!entry) {
                // If this entry is a child of another entry, try to find it's parent
                var parts = label.split('/'),
                    hasParent = parts.length !== 1,
                    parent;

                // Some messing around to get paths in a nice order, where parents appear above children
                // Image Processing
                // Image Processing/Pixel Label
                // Image Processing/Pixel Label/Find Horizon
                // Image Processing/Pixel Label/Pixels Above
                for (var i = 1; i < parts.length; i++) {
                    var parentPath = parts.slice(0, i).join('/');
                    parent = this.getOrCreateEntry(parentPath);
                    parent.row.classList.add('parent');
                    if (i === 1)
                        parent.row.classList.add('root');
                }

                var row = $('<tr></tr>');

                var cellExpand = $('<td></td>', {'class': 'expander'}).appendTo(row).get(0),
                    cellLabel = $('<td></td>').text(label).appendTo(row).get(0),
                    cellMillis = $('<td></td>', {'class': 'duration'}).appendTo(row).get(0),
                    cellMaxMillis = $('<td></td>', {'class': 'max-duration'}).appendTo(row).get(0);

                entry = {
                    label: label,
                    update: function (timestamp, millis)
                    {
                        entry.time = timestamp;
                        entry.millis = millis;
                        if (!entry.maxMillis || entry.maxMillis < millis) {
                            entry.maxMillis = millis;
                            cellMaxMillis.textContent = millis.toFixed(3);
                        }
                        cellMillis.textContent = millis.toFixed(3);
                    },
                    row: row.get(0),
                    children: [],
                    isExpanded: false
                };

                var setChildRowDisplay = function (e)
                {
                    var display = e.isExpanded ? 'table-row' : 'none';
                    _.each(e.children, function (child)
                    {
                        child.row.style.display = display;
                        setChildRowDisplay(child);
                    });
                };

                var setRowExpansion = function (isExpanded)
                {
                    entry.isExpanded = isExpanded;

                    if (isExpanded)
                        entry.row.classList.add("expanded");
                    else
                        entry.row.classList.remove("expanded");
                };

                entry.row.addEventListener('click', function ()
                {
                    setRowExpansion(!entry.isExpanded);
                    setChildRowDisplay(entry);
                });

                this.entryByLabel[label] = entry;

                if (hasParent) {
                    parent.children.push(entry);
                    entry.row.style.display = parent.isExpanded ? 'table-row' : 'none';
                }

                setRowExpansion(entry.isExpanded);

                row.appendTo(this.table)
            }
            return entry;
        };

        return TimingPane;
    }
);
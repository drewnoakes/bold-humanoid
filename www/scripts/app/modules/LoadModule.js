/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'Constants',
        'Protocols',
        'BodyFigure'
    ],
    function (DataProxy, Constants, Protocols, BodyFigure)
    {
        'use strict';

        var chartOptions = {
            millisPerPixel: 20,
            interpolation:'linear',
            grid: {
                strokeStyle: 'rgb(40, 40, 40)',
                fillStyle: 'rgb(0, 0, 0)',
                lineWidth: 0.5,
                millisPerLine: 500,
                verticalSections: 7,
                sharpLines: true,
                borderVisible: false
            },
            labels: {
                fillStyle: '#ffffff',
                precision: 2
            },
            horizontalLines: [
                { color: '#ffffff', lineWidth: 1, value: 0 }
            ]
        };

        var chartHeight = 300,
            chartWidth = 430,
            lowDiff = 5,
            highDiff = 50;

        var LoadModule = function ()
        {
            this.container = $('<div></div>');

            /////

            this.title = 'load';
            this.id = 'load';
            this.panes = [
                {
                    title: 'main',
                    element: this.container,
                    supports: { fullScreen: false }
                }
            ];
        };

        LoadModule.prototype.load = function ()
        {
            this.diffCanvas = document.createElement('canvas');
            this.diffCanvas.width = chartWidth;
            this.diffCanvas.height = chartHeight;
            this.diffChart = new SmoothieChart(chartOptions);
            this.diffChart.options.yRangeFunction = function(range)
            {
                var lim = Math.max(Math.abs(range.min), Math.abs(range.max));
                if (lim < 50)
                    lim = 50;
                return {
                    min: -lim,
                    max:  lim
                };
            };
            this.diffChart.streamTo(this.diffCanvas, /*delayMs*/ 0);
            this.container.append($('<h2></h2>').text('position error'));
            this.container.append(this.diffCanvas);

            this.diffSeriesById = [undefined];

            for (var i = 1; i <= 20; i++) {
                var series = new TimeSeries();
                this.diffChart.addTimeSeries(series, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });
                this.diffSeriesById.push(series);
            }

            this.bodyFigure = new BodyFigure({ hasHover: true, hasSelection: true });
            this.bodyFigure.hoverJointId.track(this.updateSeriesColours.bind(this));
            this.bodyFigure.selectedJointIds.track(this.updateSeriesColours.bind(this));
            this.container.append(this.bodyFigure.element);

            this.subscription = DataProxy.subscribe(
                Protocols.bodyState,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );
        };

        LoadModule.prototype.updateSeriesColours = function()
        {
            _.each(_.range(1,21), function(jointId)
            {
                var series = this.diffSeriesById[jointId],
                    isHovered = jointId === this.bodyFigure.hoverJointId.getValue(),
                    isSelected = this.bodyFigure.selectedJointIds.getValue().indexOf(jointId) !== -1;

                this.diffChart.getTimeSeriesOptions(series).strokeStyle
                    = isHovered || isSelected
                      ? 'rgba(121, 36, 133, 0.7)'
                      : '#ffffff';

                if (isHovered || isSelected)
                {
                    this.diffChart.bringToFront(series);
                }

            }.bind(this));
        };

        LoadModule.prototype.unload = function ()
        {
            this.diffChart.stop();

            this.container.empty();
            this.subscription.close();

            delete this.diffCanvas;
            delete this.diffChart;
            delete this.diffSeriesById;

            delete this.bodyFigure;
            delete this.subscription;
        };

        LoadModule.prototype.onData = function (data)
        {
            var time = new Date().getTime();

            _.each(_.range(1,21), function(jointId)
            {
                var error = data.errors[jointId];

                this.diffSeriesById[jointId].append(time, error);

                var jointElement = this.bodyFigure.getJointElement(jointId),
                    t = Math.max(Math.min(highDiff, Math.abs(error)), lowDiff),
                    ratio = (t - lowDiff) / (highDiff - lowDiff),
                    hue = 140 * (1 - ratio),
                    hsl = 'hsl('  + hue + ', 100%, 50%)';

                jointElement.textContent = error;
                jointElement.style.background = hsl;
            }.bind(this));
        };

        return LoadModule;
    }
);

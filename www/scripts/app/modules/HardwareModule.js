/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'ControlBuilder',
        'constants',
        'BodyFigure'
    ],
    function (DataProxy, ControlBuilder, constants, BodyFigure)
    {
        'use strict';

        var chartOptions = {
            millisPerPixel: 1000,
            interpolation:'linear',
            grid: {
                strokeStyle: 'rgb(40, 40, 40)',
                fillStyle: 'rgb(0, 0, 0)',
                lineWidth: 0.5,
                millisPerLine: 60000,
                verticalSections: 6,
                sharpLines: true,
                borderVisible: false
            },
            labels: {
                fillStyle: '#ffffff',
                precision: 0
            }
        };

        var chartHeight = 150,
            chartWidth = 430,
            lowTemperature = 25,
            highTemperature = 65,
            lowVoltage = 11,
            highVoltage = 13;

        var HardwareModule = function ()
        {
            this.container = $('<div></div>');

            /////

            this.title = 'hardware';
            this.id = 'hardware';
            this.panes = [
                {
                    title: 'main',
                    element: this.container,
                    supports: { fullScreen: false }
                }
            ];
        };

        HardwareModule.prototype.load = function ()
        {
            this.voltageCanvas = document.createElement('canvas');
            this.voltageCanvas.width = chartWidth;
            this.voltageCanvas.height = chartHeight;
            this.voltageChart = new SmoothieChart(chartOptions);
            this.voltageChart.options.yRangeFunction = function(range)
            {
                return {
                    min: Math.floor(Math.min(range.min, lowVoltage)),
                    max: Math.ceil(Math.max(range.max, highVoltage))
                };
            };
            this.voltageChart.streamTo(this.voltageCanvas, /*delayMs*/ 200);
            this.container.append($('<h2></h2>').text('voltage'));
            this.container.append(this.voltageCanvas);

            this.voltageSeries = new TimeSeries();
            this.voltageChart.addTimeSeries(this.voltageSeries, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });


            this.temperatureCanvas = document.createElement('canvas');
            this.temperatureCanvas.width = chartWidth;
            this.temperatureCanvas.height = chartHeight;
            this.temperatureChart = new SmoothieChart(chartOptions);
            this.temperatureChart.options.yRangeFunction = function(range)
            {
                return {
                    min: Math.floor(Math.min(range.min, lowTemperature)),
                    max: Math.ceil(Math.max(range.max, highTemperature))
                };
            };
            this.temperatureChart.streamTo(this.temperatureCanvas, /*delayMs*/ 200);
            this.container.append($('<h2></h2>').text('temperature'));
            this.container.append(this.temperatureCanvas);

            this.temperatureSeriesById = [undefined];

            for (var i = 1; i <= 20; i++) {
                var series = new TimeSeries();
                this.temperatureChart.addTimeSeries(series, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });
                this.temperatureSeriesById.push(series);
            }

            this.bodyFigure = new BodyFigure();
            this.container.append(this.bodyFigure.element);

            var controlContainer = document.createElement('div');
            controlContainer.className = 'control-container';
            ControlBuilder.actions("hardware", controlContainer);
            this.container.append(controlContainer);

            this.subscription = DataProxy.subscribe(
                constants.protocols.hardwareState,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );
        };

        HardwareModule.prototype.unload = function ()
        {
            this.voltageChart.stop();
            this.temperatureChart.stop();

            this.container.empty();
            this.subscription.close();

            delete this.voltageCanvas;
            delete this.voltageChart;
            delete this.voltageSeries;

            delete this.temperatureCanvas;
            delete this.temperatureChart;
            delete this.temperatureSeriesById;

            delete this.bodyFigure;
            delete this.subscription;
        };

        HardwareModule.prototype.onData = function (data)
        {
            var time = new Date().getTime();

            // Only allow one point per second, to avoid using too much memory
            if (this.lastDataTime && (time - this.lastDataTime) < 1000)
                return;

            this.lastDataTime = time;

            this.voltageSeries.append(time, data.volts);

            _.each(data.joints, function(joint)
            {
                this.temperatureSeriesById[joint.id].append(time, joint.temp);

                var jointElement = this.bodyFigure.getJointElement(joint.id),
                    t = Math.max(Math.min(highTemperature, joint.temp), lowTemperature),
                    ratio = (t - lowTemperature) / (highTemperature - lowTemperature),
                    hue = 140 * (1 - ratio),
                    hsl = 'hsl('  + hue + ', 100%, 50%)';

                jointElement.textContent = joint.temp;
                jointElement.style.background = hsl;
            }.bind(this));
        };

        return HardwareModule;
    }
);

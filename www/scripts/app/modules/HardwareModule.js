/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'Constants',
        'Protocols'
    ],
    function (DataProxy, Constants, Protocols)
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

        var bodyTemplate = Handlebars.compile($('#hardware-body-model-template').html());

        var chartHeight = 150,
            chartWidth = 430,
            lowTemperature = 25,
            highTemperature = 45,
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
            this.container.append(this.temperatureCanvas);

            this.temperatureSeriesById = [undefined];

            for (var i = 1; i <= 20; i++) {
                var series = new TimeSeries();
                this.temperatureChart.addTimeSeries(series, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });
                this.temperatureSeriesById.push(series);
            }

            this.buildJointBlocks();

            this.subscription = DataProxy.subscribe(
                Protocols.hardwareState,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );
        };

        HardwareModule.prototype.buildJointBlocks = function()
        {
            var shoulderOffset = 60,
                elbowOffset = 65,
                hipOffset = 40,
                kneeOffset = 45,
                footOffset = 50;

            // TODO rename Constants.jointIds to use consistent pitch/roll/yaw naming

            var bodyData = {
                joints: [
                    { id: Constants.jointIds.headTilt, name: 'Head Tilt', row: 0, group: 0, xOffset: 0 },
                    { id: Constants.jointIds.headPan,  name: 'Head Pan', row: 1, group: 0, xOffset: 0 },

                    { id: Constants.jointIds.shoulderForwardLeft,  name: 'Left Shoulder Pitch', row: 2, group: 1, xOffset: shoulderOffset },
                    { id: Constants.jointIds.shoulderForwardRight, name: 'Right Shoulder Pitch', row: 2, group: 1, xOffset: -shoulderOffset },
                    { id: Constants.jointIds.shoulderOutwardLeft,  name: 'Left Shoulder Roll', row: 3, group: 1, xOffset: shoulderOffset },
                    { id: Constants.jointIds.shoulderOutwardRight, name: 'Right Shoulder Roll', row: 3, group: 1, xOffset: -shoulderOffset },

                    { id: Constants.jointIds.elbowLeft,  name: 'Left Elbow',  row: 4, group: 2, xOffset: elbowOffset },
                    { id: Constants.jointIds.elbowRight, name: 'Right Elbow', row: 4, group: 2, xOffset: -elbowOffset },

                    { id: Constants.jointIds.legTurnLeft,     name: 'Left Hip Yaw', row: 5, group: 3, xOffset: hipOffset },
                    { id: Constants.jointIds.legTurnRight,    name: 'Right Hip Yaw', row: 5, group: 3, xOffset: -hipOffset },
                    { id: Constants.jointIds.legOutLeft,      name: 'Left Hip Roll', row: 6, group: 3, xOffset: hipOffset },
                    { id: Constants.jointIds.legOutRight,     name: 'Right Hip Roll', row: 6, group: 3, xOffset: -hipOffset },
                    { id: Constants.jointIds.legForwardLeft,  name: 'Left Hip Pitch', row: 7, group: 3, xOffset: hipOffset },
                    { id: Constants.jointIds.legForwardRight, name: 'Right Hip Pitch', row: 7, group: 3, xOffset: -hipOffset },

                    { id: Constants.jointIds.kneeLeft,  name: 'Left Knee', row: 8, group: 4, xOffset: kneeOffset },
                    { id: Constants.jointIds.kneeRight, name: 'Right Knee', row: 8, group: 4, xOffset: -kneeOffset },

                    { id: Constants.jointIds.footForwardLeft,  name: 'Left Ankle Pitch', row: 9, group: 5, xOffset: footOffset },
                    { id: Constants.jointIds.footForwardRight, name: 'Right Ankle Pitch', row: 9, group: 5, xOffset: -footOffset },
                    { id: Constants.jointIds.footOutLeft,      name: 'Left Ankle Roll', row: 10, group: 5, xOffset: footOffset },
                    { id: Constants.jointIds.footOutRight,     name: 'Right Ankle Roll', row: 10, group: 5, xOffset: -footOffset }
                ],
                containerHeight: 0,
                containerWidth: 0
            };

            // layout constants
            var blockHeight = 20;
            var blockWidth = 70;
            var blockSpacing = 25; // from top-to-top, not bottom-to-top
            var groupSpacing = 6;
            var center = 70;

            _.each(bodyData.joints, function (joint)
            {
                joint.y = groupSpacing * joint.group + blockSpacing * joint.row;
                joint.x = joint.xOffset >= 0
                    ? center + joint.xOffset
                    : center + joint.xOffset;// - blockWidth;
                joint.height = blockHeight;
                joint.width = blockWidth;

                bodyData.containerWidth = Math.max(bodyData.containerWidth, joint.x + joint.width);
                bodyData.containerHeight = Math.max(bodyData.containerHeight, joint.y + joint.height);
            });

            var bodyHtml = bodyTemplate(bodyData);
            var $bodyContainer = $('<div></div>').html(bodyHtml).children().appendTo(this.container);

            this.jointBlockById = [undefined];

            for (var i = 1; i <= 20; i++) {
                var jointBlock = $bodyContainer.find('div.joint[data-joint-id=' + i + ']').get(0);
                jointBlock.temperature = $(jointBlock).find('div.temperature');
                this.jointBlockById.push(jointBlock);
            }
        };

        HardwareModule.prototype.unload = function ()
        {
            this.voltageChart.stop();
            this.temperatureChart.stop();

            this.container.empty();
            this.subscription.close();
        };

        HardwareModule.prototype.onData = function (data)
        {
            var time = new Date().getTime();

            if (this.lastDataTime && (time - this.lastDataTime) < 1000)
                return;

            this.lastDataTime = time;

            // TODO this probably causes too many data points to be added -- conflate

            this.voltageSeries.append(time, data.volts);

            _.each(data.joints, function(joint)
            {
                this.temperatureSeriesById[joint.id].append(time, joint.temp);

                this.jointBlockById[joint.id].temperature.text(joint.temp);

                var t = Math.max(Math.min(highTemperature, joint.temp), lowTemperature),
                    ratio = (t - lowTemperature) / (highTemperature - lowTemperature),
                    hue = 140 * (1 - ratio),
                    hsl = 'hsl('  + hue + ', 100%, 50%)';

                this.jointBlockById[joint.id].style.background = hsl;
            }.bind(this));
        };

        return HardwareModule;
    }
);

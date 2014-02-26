/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'constants',
        'DataProxy',
        'DOMTemplate',
        'ControlBuilder'
    ],
    function(constants, DataProxy, DOMTemplate, ControlBuilder)
    {
        'use strict';

        var radarSize = 200,
            moveScale = 3,
            moduleTemplate = new DOMTemplate("walk-module-template");

        var chartOptions = {
            grid: {
                strokeStyle: 'rgba(0,0,0,0.1)',
                fillStyle: 'transparent',
                lineWidth: 1,
                millisPerLine: 250,
                verticalSections: 5,
                sharpLines: true
            },
            labels: {
                fillStyle: 'rgba(0,0,0,0.5)',
                precision: 0
            }
        };

        var WalkModule = function()
        {
            this.$container = $('<div></div>');

            this.title = 'walk';
            this.id = 'walk';
            this.element = this.$container.get(0);
        };

        WalkModule.prototype.load = function()
        {
            var element = moduleTemplate.create({radarSize: radarSize, chartWidth: 440, chartHeight: 60});
            this.$container.append(element);

            this.runningIndicator = element.querySelector('.connection-indicator');
            this.radarCanvas = element.querySelector('canvas.radar');

            ControlBuilder.buildAll('ambulator', element.querySelector('.ambulator-controls'));
            ControlBuilder.buildAll('options.approach-ball', element.querySelector('.approach-ball-controls'));
            ControlBuilder.buildAll('walk-module', element.querySelector('.walk-controls'));

            this.subscription = DataProxy.subscribe(constants.protocols.ambulatorState, { json: true, onmessage: _.bind(this.onData, this) });

            this.drawRadar();

            this.pitchSeries = new TimeSeries();
            this.xAmpCurrentSeries = new TimeSeries();
            this.xAmpTargetSeries = new TimeSeries();
            this.angleCurrentSeries = new TimeSeries();
            this.angleTargetSeries = new TimeSeries();

            this.pitchChart = new SmoothieChart(_.extend({}, chartOptions, {minValue: 10, maxValue: 15}));
            this.pitchChart.addTimeSeries(this.pitchSeries, { strokeStyle: 'rgb(0, 0, 255)', lineWidth: 1 });
            this.pitchChart.streamTo(element.querySelector('canvas.pitch-chart'), /*delayMs*/ 0);

            this.xAmpChart = new SmoothieChart(_.extend({}, chartOptions, {minValue: 0, maxValue: 40}));
            this.xAmpChart.addTimeSeries(this.xAmpCurrentSeries, { strokeStyle: 'rgb(121, 36, 133)', lineWidth: 1 });
            this.xAmpChart.addTimeSeries(this.xAmpTargetSeries, { strokeStyle: 'rgba(121, 36, 133, 0.4)', lineWidth: 1 });
            this.xAmpChart.streamTo(element.querySelector('canvas.x-amp-chart'), /*delayMs*/ 0);

            this.turnChart = new SmoothieChart(_.extend({}, chartOptions, {minValue: -25, maxValue: 25}));
            this.turnChart.addTimeSeries(this.angleCurrentSeries, { strokeStyle: 'rgb(121, 36, 133)', lineWidth: 1 });
            this.turnChart.addTimeSeries(this.angleTargetSeries, { strokeStyle: 'rgba(121, 36, 133, 0.4)', lineWidth: 1 });
            this.turnChart.streamTo(element.querySelector('canvas.turn-chart'), /*delayMs*/ 0);
        };

        WalkModule.prototype.unload = function()
        {
            this.$container.empty();
            this.subscription.close();
            this.pitchChart.stop();
            this.xAmpChart.stop();
            this.turnChart.stop();
            delete this.pitchChart;
            delete this.xAmpChart;
            delete this.turnChart;
            delete this.radarCanvas;
            delete this.runningIndicator;
            delete this.subscription;
        };

        WalkModule.prototype.drawRadar = function (data)
        {
            // TODO use a dirty flag and only draw on animation frames

            var context = this.radarCanvas.getContext('2d');

            context.clearRect(0, 0, radarSize, radarSize);

            // Draw crosshairs

            var mid = Math.round(radarSize / 2);

            context.strokeStyle = 'rgba(0, 0, 0, 0.3)';
            context.lineWidth = 2;
            context.beginPath();
            context.moveTo(mid, 0);
            context.lineTo(mid, radarSize);
            context.moveTo(0, mid);
            context.lineTo(radarSize, mid);
            context.stroke();

            if (!data)
                return;

            mid = (radarSize / 2) + 0.5;

            //
            // Angles
            //

            // Target
            context.strokeStyle = 'rgba(121, 36, 133, 0.3)';
            context.beginPath();
            context.lineCap = 'round';
            context.lineWidth = 20;
            context.arc(mid, mid, radarSize * 0.4, -Math.PI / 2, -Math.PI / 2 - (data.target[2] * Math.PI / 180), data.target[2] > 0);
            context.stroke();

            // Current
            context.strokeStyle = 'rgba(121, 36, 133, 1)';
            context.beginPath();
            context.lineCap = 'round';
            context.lineWidth = 9;
            context.arc(mid, mid, radarSize * 0.4, -Math.PI / 2, -Math.PI / 2 - (data.current[2] * Math.PI / 180), data.current[2] > 0);
            context.stroke();

            //
            // Movement Direction
            //

            // Target
            context.strokeStyle = 'rgba(121, 36, 133, 0.3)';
            context.beginPath();
            context.lineCap = 'round';
            context.lineWidth = 20;
            context.moveTo(mid, mid);
            context.lineTo(mid + (data.target[1] * moveScale), mid - (data.target[0] * moveScale));
            context.stroke();

            // Current
            context.strokeStyle = 'rgba(121, 36, 133, 1)';
            context.beginPath();
            context.lineCap = 'round';
            context.lineWidth = 9;
            context.moveTo(mid, mid);
            context.lineTo(mid + (data.current[1] * moveScale), mid - (data.current[0] * moveScale));
            context.stroke();
        };

        WalkModule.prototype.onData = function(data)
        {
//            var data = {
//                target: [5,4,30],   // x, y, angle
//                current: [8,10,20], // x, y, angle
//                delta: [1,-1,2],    // x, y, angle
//                running: true,
//                phase: 3,
//                hipPitch: 3,
//                bodySwingY: 123,
//                bodySwingZ: 321
//            };

            if (data.running)
            {
                this.runningIndicator.classList.add('connected');
                this.runningIndicator.classList.remove('disconnected');
                this.runningIndicator.textContent = data.phase;
            }
            else
            {
                this.runningIndicator.classList.remove('connected');
                this.runningIndicator.classList.add('disconnected');
                this.runningIndicator.textContent = '';
            }

            var time = new Date().getTime();
            this.pitchSeries.append(time, data.hipPitch);
            this.xAmpCurrentSeries.append(time, data.current[0]);
            this.xAmpTargetSeries.append(time, data.target[0]);
            this.angleCurrentSeries.append(time, data.current[2]);
            this.angleTargetSeries.append(time, data.target[2]);

            this.drawRadar(data);
        };

        return WalkModule;
    }
);

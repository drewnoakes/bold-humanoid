/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'Protocols',
        'DataProxy',
        'DOMTemplate',
        'ControlBuilder'
    ],
    function(Protocols, DataProxy, DOMTemplate, ControlBuilder)
    {
        'use strict';

        var size = 300,
            moveScale = 3,
            moduleTemplate = new DOMTemplate("walk-module-template");

        var WalkModule = function()
        {
            this.$container = $('<div></div>');

            this.title = 'walk';
            this.id = 'walk';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    supports: { fullScreen: true, advanced: false }
                }
            ];
        };

        WalkModule.prototype.load = function()
        {
            var element = moduleTemplate.create({size: size});
            this.$container.append(element);

            this.runningIndicator = element.querySelector('.connection-indicator');
            this.canvas = element.querySelector('canvas');
            this.context = this.canvas.getContext('2d');

            ControlBuilder.buildAll('ambulator', element.querySelector('.ambulator-controls'));
            ControlBuilder.buildAll('options.approach-ball', element.querySelector('.approach-ball-controls'));
            ControlBuilder.buildAll('walk-module', element.querySelector('.walk-controls'));

            this.subscription = DataProxy.subscribe(Protocols.ambulatorState, { json: true, onmessage: _.bind(this.onData, this) });

            this.drawRadar();
        };

        WalkModule.prototype.unload = function()
        {
            this.$container.empty();
            this.subscription.close();
            delete this.canvas;
            delete this.runningIndicator;
            delete this.subscription;
        };

        WalkModule.prototype.drawRadar = function (data)
        {
            var context = this.context;

            context.clearRect(0, 0, size, size);

            // Draw crosshairs

            var mid = Math.round(size / 2);

            context.strokeStyle = 'rgba(0, 0, 0, 0.3)';
            context.lineWidth = 2;
            context.beginPath();
            context.moveTo(mid, 0);
            context.lineTo(mid, size);
            context.moveTo(0, mid);
            context.lineTo(size, mid);
            context.stroke();

            if (!data)
                return;

            mid = (size / 2) + 0.5;

            //
            // Angles
            //

            // Target
            context.strokeStyle = 'rgba(121, 36, 133, 0.3)';
            context.beginPath();
            context.lineCap = 'round';
            context.lineWidth = 20;
            context.arc(mid, mid, size * 0.4, -Math.PI / 2, -Math.PI / 2 - (data.target[2] * Math.PI / 180), data.target[2] > 0);
            context.stroke();

            // Current
            context.strokeStyle = 'rgba(121, 36, 133, 1)';
            context.beginPath();
            context.lineCap = 'round';
            context.lineWidth = 9;
            context.arc(mid, mid, size * 0.4, -Math.PI / 2, -Math.PI / 2 - (data.current[2] * Math.PI / 180), data.current[2] > 0);
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

            this.drawRadar(data);
        };

        return WalkModule;
    }
);

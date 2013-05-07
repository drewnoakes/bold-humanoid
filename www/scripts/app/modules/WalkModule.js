/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'Protocols',
        'DataProxy'
    ],
    function(Protocols, DataProxy)
    {
        'use strict';

        var size = 300,
            moveScale = 8;

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
            this.$runningIndicator = $('<div></div>').addClass('connection-indicator connecting').appendTo(this.$container);
            this.canvas = $('<canvas>', { width: size, height: size }).appendTo(this.$container).get(0);
            this.canvas.width = size;
            this.canvas.height = size;
            this.context = this.canvas.getContext('2d');

            this.subscription = DataProxy.subscribe(Protocols.ambulatorState, { json: true, onmessage: _.bind(this.onData, this) });
        };

        WalkModule.prototype.unload = function()
        {
            this.$container.empty();
            this.subscription.close();
        };

        WalkModule.prototype.onData = function(data)
        {
//            var data = {
//                target: [5,4,30],  // x, y, angle
//                current: [8,10,20], // x, y, angle
//                running: true,
//                phase: 3,
//                bodySwingY: 123,
//                bodySwingZ: 321
//            };

            if (data.running)
            {
                this.$runningIndicator.addClass('connected');
                this.$runningIndicator.removeClass('disconnected');
            }
            else
            {
                this.$runningIndicator.removeClass('connected');
                this.$runningIndicator.addClass('disconnected');
            }

            var context = this.context;

            context.clearRect(0, 0, size, size);

            var mid = (size / 2) + 0.5;

            // Cross hairs
            context.strokeStyle = 'rgba(0, 0, 0, 0.3)';
            context.beginPath();
            context.moveTo(mid, 0);
            context.lineTo(mid, size);
            context.moveTo(0, mid);
            context.lineTo(size, mid);
            context.stroke();

            //
            // Angles
            //

            // Target
            context.strokeStyle = 'rgba(121, 36, 133, 0.3)';
            context.beginPath();
            context.lineCap = 'round';
            context.lineWidth = 20;
            context.arc(mid, mid, size*0.4, -Math.PI/2, -Math.PI/2 + (data.target[2]*Math.PI/180), data.target[2] < 0);
            context.stroke();

            // Current
            context.strokeStyle = 'rgba(121, 36, 133, 1)';
            context.beginPath();
            context.lineCap = 'round';
            context.lineWidth = 9;
            context.arc(mid, mid, size*0.4, -Math.PI/2, -Math.PI/2 + (data.current[2]*Math.PI/180), data.current[2] < 0);
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

        return WalkModule;
    }
);

/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/FieldLinePlotter',
        'scripts/app/Protocols',
        'scripts/app/Constants',
        'scripts/app/DataProxy',
        'scripts/app/util/Dragger'
    ],
    function(FieldLinePlotter, Protocols, Constants, DataProxy, Dragger)
    {
        'use strict';

        // A simple module, with a full screen canvas as its element

        var World2dModule = function()
        {
            this.$canvas = $('<canvas></canvas>');
            this.canvas = this.$canvas.get(0);
            this.$container = $('<div></div>');
            this.$hoverInfo = $('<div></div>', {'class': 'hover-info'});

            this.$container.append(this.$canvas)
                           .append(this.$hoverInfo);

            this.bindEvents();

            /////

            this.title = '2d world';
            this.moduleClass = 'field';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        World2dModule.prototype.bindEvents = function()
        {
            Dragger.bind(this.canvas, {
                isRelative: true,
                move: function(dx, dy)
                {
                    this.fieldCenterX += dx;
                    this.fieldCenterY += dy;
                    this.draw();
                }.bind(this)
            });

            this.$canvas.on('mousewheel', function (event)
            {
                event.preventDefault();
                // TODO zoom relative to the position of the mouse pointer, rather than (0,0)
                this.scale += event.originalEvent.wheelDelta / 20;
                this.scale = Math.max(this.minScale, this.scale);
                this.draw();
            }.bind(this));

            this.$canvas.on('mousemove', function (event)
            {
                var x = (event.offsetX - this.fieldCenterX) / this.scale,
                    y = (event.offsetY - this.fieldCenterY) / this.scale;
                this.$hoverInfo.text(x.toFixed(2) + ', ' + y.toFixed(2));
            }.bind(this));

            this.$canvas.on('mouseleave', function() { this.$hoverInfo.text(''); }.bind(this));
        };

        World2dModule.prototype.load = function()
        {
            this.worldFrameSubscription = DataProxy.subscribe(Protocols.worldFrameState, { json: true, onmessage: _.bind(this.onWorldFrameData, this) });

            // TODO only subscribe if use checks a box
            this.particleSubscription   = DataProxy.subscribe(Protocols.particleState,   { json: true, onmessage: _.bind(this.onParticleData, this) });
        };

        World2dModule.prototype.unload = function()
        {
            this.worldFrameSubscription.close();
            this.particleSubscription.close();
        };

        World2dModule.prototype.onWorldFrameData = function(data)
        {
            this.ballPosition = data.ball;
            this.lineSegments = [];

            _.each(data.lines, function (line)
            {
                var p1 = { x: line[0], y: line[1], z: line[2] };
                var p2 = { x: line[3], y: line[4], z: line[5] };
                this.lineSegments.push({ p1: p1, p2: p2 });
            }.bind(this));

            this.draw(); // TODO only draw worldFrameData, on its own canvas
        };

        World2dModule.prototype.onParticleData = function(data)
        {
            this.particles = data;
            this.draw(); // TODO only draw particles, on their own canvas
        };

        World2dModule.prototype.onResized = function(width, height)
        {
            this.canvas.width = width;
            this.canvas.height = height;
            this.fieldCenterX = width/2;
            this.fieldCenterY = height/2;
            this.scale = Math.min(
                width / (Constants.fieldX + 2*Constants.outerMarginMinimum),
                height / (Constants.fieldY + 2*Constants.outerMarginMinimum));
            this.minScale = this.scale * 0.8; // can't zoom out too far

            this.draw();
        };

        World2dModule.prototype.draw = function()
        {
            var options = {
                    scale: this.scale,
                    goalStrokeStyle: 'yellow',
                    groundFillStyle: '#008800',
                    lineStrokeStyle: '#ffffff',
                    particleStyle: 'cyan',
                    particleSize: 3,
                    fieldCenter: { x: this.fieldCenterX, y: this.fieldCenterY }
                },
                context = this.canvas.getContext('2d');

            FieldLinePlotter.start(context, options);
            FieldLinePlotter.drawFieldLines(context, options);
            FieldLinePlotter.drawGoals(context, options);

            if (this.lineSegments && this.lineSegments.length)
                FieldLinePlotter.drawLineSegments(context, options, this.lineSegments, 1, '#0000ff');

            if (this.ballPosition)
                FieldLinePlotter.drawBall(context, options, this.ballPosition);

            if (this.particles)
                FieldLinePlotter.drawParticles(context, options, this.particles);

            FieldLinePlotter.end(context);
        };

        return World2dModule;
    }
);
/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'FieldLinePlotter',
        'Protocols',
        'Constants',
        'DataProxy',
        'util/Dragger',
        'util/MouseEventUtil'
    ],
    function(FieldLinePlotter, Protocols, Constants, DataProxy, Dragger, MouseEventUtil)
    {
        'use strict';

        var World2dModule = function()
        {
            this.$container = $('<div></div>');

            /////

            this.title = '2d world';
            this.id = '2d';
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

            this.$cameraCanvas.on('mousewheel', function (event)
            {
                event.preventDefault();
                // TODO zoom relative to the position of the mouse pointer, rather than (0,0)
                this.scale += event.originalEvent.wheelDelta / 20;
                this.scale = Math.max(this.minScale, this.scale);
                this.draw();
            }.bind(this));

            this.$cameraCanvas.on('mousemove', function (event)
            {
                MouseEventUtil.polyfill(event);
                var x = (event.offsetX - this.fieldCenterX) / this.scale,
                    y = (event.offsetY - this.fieldCenterY) / this.scale;
                this.$hoverInfo.text(x.toFixed(2) + ', ' + y.toFixed(2));
            }.bind(this));

            this.$cameraCanvas.on('mouseleave', function() { this.$hoverInfo.text(''); }.bind(this));
        };

        World2dModule.prototype.load = function()
        {
            this.$cameraCanvas = $('<canvas></canvas>');
            this.canvas = this.$cameraCanvas.get(0);
            this.$hoverInfo = $('<div></div>', {'class': 'hover-info'});

            this.$container.append(this.$cameraCanvas)
                           .append(this.$hoverInfo);

            this.bindEvents();

            this.worldFrameSubscription = DataProxy.subscribe(Protocols.worldFrameState, { json: true, onmessage: _.bind(this.onWorldFrameData, this) });

            // TODO only subscribe if use checks a box
            this.particleSubscription   = DataProxy.subscribe(Protocols.particleState,   { json: true, onmessage: _.bind(this.onParticleData, this) });
        };

        World2dModule.prototype.unload = function()
        {
            this.$container.empty();
            this.worldFrameSubscription.close();
            this.particleSubscription.close();
        };

        World2dModule.prototype.onWorldFrameData = function(data)
        {
            this.agentPosition = data.pos;
            this.ballPosition = data.ball;
            this.visibleFieldPoly = data['visible-field-poly'];
            this.lineSegments = [];
            this.goalPositions = [];

            _.each(data.lines, function (line)
            {
                var p1 = { x: line[0], y: line[1]/*, z: line[2]*/ };
                var p2 = { x: line[3], y: line[4]/*, z: line[5]*/ };
                this.lineSegments.push({ p1: p1, p2: p2 });
            }.bind(this));

            _.each(data.goals, function (goalPos)
            {
                this.goalPositions.push({ x: goalPos[0], y: goalPos[1] });
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
            var fieldLengthX = (Constants.fieldX + 2 * Constants.outerMarginMinimum);
            var fieldLengthY = (Constants.fieldY + 2 * Constants.outerMarginMinimum);
            var ratio = fieldLengthX / fieldLengthY;

            this.canvas.width = width;
            this.canvas.height = width / ratio;
            this.fieldCenterX = width/2;
            this.fieldCenterY = (width / ratio)/2;
            this.scale = Math.min(
                width / fieldLengthX,
                (width / ratio) / fieldLengthY);
            this.minScale = this.scale * 0.5; // can't zoom out too far

            this.draw();
        };

        World2dModule.prototype.draw = function()
        {
            var options = {
                    // TODO replace this scale with a transform on the canvas instead
                    scale: this.scale,
                    goalStrokeStyle: 'yellow',
                    groundFillStyle: '#008800',
                    lineStrokeStyle: '#ffffff',
                    visibleFieldPolyLineWidth: 1,
                    visibleFieldPolyStrokeStyle: '#0000ff',
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

            if (this.particles)
                FieldLinePlotter.drawParticles(context, options, this.particles);

            if (this.agentPosition)
                FieldLinePlotter.drawAgentPosition(context, options, this.agentPosition);

            if (this.visibleFieldPoly)
                FieldLinePlotter.drawVisibleFieldPoly(context, options, this.visibleFieldPoly);

            if (this.ballPosition)
                FieldLinePlotter.drawBall(context, options, this.ballPosition);

            if (this.goalPositions) {
                options.goalStrokeStyle = 'blue';
                FieldLinePlotter.drawGoalPosts(context, options, this.goalPositions);
            }

            FieldLinePlotter.end(context);
        };

        return World2dModule;
    }
);
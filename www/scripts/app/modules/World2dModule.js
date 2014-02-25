/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'FieldLinePlotter',
        'Protocols',
        'Constants',
        'ControlBuilder',
        'DataProxy',
        'HeadControls',
        'util/Dragger',
        'util/mouse',
        'util/Geometry'
    ],
    function(FieldLinePlotter, Protocols, Constants, ControlBuilder, DataProxy, HeadControls, Dragger, mouse, Geometry)
    {
        'use strict';

        var Transform = Geometry.Transform;

        var World2dModule = function()
        {
            this.$container = $('<div></div>');

            /////

            this.title = '2d world';
            this.id = 'world-2d';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];

            this.needsRender = false;
        };

        World2dModule.prototype.bindEvents = function()
        {
            Dragger.bind(this.canvas, {
                isRelative: true,
                move: function(dx, dy)
                {
                    this.transform = new Transform()
                        .translate(dx, dy)
                        .multiply(this.transform);
                    this.needsRender = true;
                }.bind(this)
            });

            this.$canvas.on('mousewheel', function (event)
            {
                event.preventDefault();
                var scale = Math.pow(1.1, event.originalEvent.wheelDelta / 80);
                this.transform = new Transform()
                    .translate(event.offsetX, event.offsetY)
                    .scale(scale, scale)
                    .translate(-event.offsetX, -event.offsetY)
                    .multiply(this.transform);
                this.needsRender = true;
            }.bind(this));

            this.$canvas.on('mousemove', function (event)
            {
                mouse.polyfill(event);
                var p = this.transform.clone().invert().transformPoint(event.offsetX, event.offsetY);
                this.hoverInfo.textContent = p.x.toFixed(2) + ', ' + p.y.toFixed(2);
            }.bind(this));

            this.$canvas.on('mouseleave', function() { this.hoverInfo.textContent = ''; }.bind(this));
        };

        World2dModule.prototype.load = function()
        {
            this.transform = new Transform().scale(1, -1);

            this.$canvas = $('<canvas></canvas>');
            this.canvas = this.$canvas.get(0);
            this.hoverInfo = document.createElement('div');
            this.hoverInfo.className = 'hover-info';

            var localiserControlContainer = document.createElement('div');
            localiserControlContainer.className = 'localiser-controls';
            ControlBuilder.actions('localiser', localiserControlContainer);

            this.$container.append(this.$canvas)
                           .append(new HeadControls().element)
                           .append(localiserControlContainer)
                           .append(this.hoverInfo);

            this.bindEvents();

            this.worldFrameSubscription = DataProxy.subscribe(Protocols.worldFrameState, { json: true, onmessage: _.bind(this.onWorldFrameData, this) });

            // TODO only subscribe if user checks a box
            this.particleSubscription   = DataProxy.subscribe(Protocols.particleState,   { json: true, onmessage: _.bind(this.onParticleData, this) });

            this.stopAnimation = false;
            this.needsRender = true;
            this.animate();
        };

        World2dModule.prototype.unload = function()
        {
            this.stopAnimation = true;
            this.$container.empty();
            this.worldFrameSubscription.close();
            this.particleSubscription.close();
        };

        World2dModule.prototype.onWorldFrameData = function(data)
        {
            this.agentPosition = data.pos;
            this.ballPosition = data.ball;
            this.visibleFieldPoly = data['visible-field-poly'];
            this.observedLineSegments = [];
            this.goalPositions = [];

            _.each(data.lines, function (line)
            {
                var p1 = { x: line[0], y: line[1]/*, z: line[2]*/ };
                var p2 = { x: line[3], y: line[4]/*, z: line[5]*/ };
                this.observedLineSegments.push({ p1: p1, p2: p2 });
            }.bind(this));

            _.each(data.goals, function (goalPos)
            {
                this.goalPositions.push({ x: goalPos[0], y: goalPos[1] });
            }.bind(this));

            this.needsRender = true; // TODO only draw worldFrameData, on its own canvas
        };

        World2dModule.prototype.onParticleData = function(data)
        {
            this.particles = data.particles;
            this.needsRender = true; // TODO only draw particles, on their own canvas
        };

        World2dModule.prototype.onResized = function(width, height)
        {
            var fieldLengthX = (Constants.fieldX + 2 * Constants.outerMarginMinimum);
            var fieldLengthY = (Constants.fieldY + 2 * Constants.outerMarginMinimum);
            var ratio = fieldLengthX / fieldLengthY;

            this.canvas.width = width;
            this.canvas.height = width / ratio;

            var scale = Math.min(
                width / fieldLengthX,
                (width / ratio) / fieldLengthY);
            this.transform = new Transform()
                .scale(scale, -scale)
                .translate(fieldLengthX/2, -fieldLengthY/2);
            this.needsRender = true;
        };

        World2dModule.prototype.animate = function()
        {
            if (this.stopAnimation)
                return;

            window.requestAnimationFrame(this.animate.bind(this));

            if (!this.needsRender)
                return;

            var scale = this.transform.getScale(),
                options = {
                    goalStrokeStyle: 'yellow',
                    groundFillStyle: '#008800',
                    lineStrokeStyle: '#ffffff',
                    visibleFieldPolyLineWidth: 1/scale,
                    visibleFieldPolyStrokeStyle: '#0000ff',
                    particleHue: 240,
                    particleSize: Math.max(0.015, 2/scale),
                    ballFillStyle: 'red'
                },
                context = this.canvas.getContext('2d');

            this.transform.applyTo(context);

            FieldLinePlotter.drawField(context, options);
            FieldLinePlotter.drawFieldLines(context, options);
            FieldLinePlotter.drawGoals(context, options);

            if (this.observedLineSegments && this.observedLineSegments.length)
                FieldLinePlotter.drawLineSegments(context, options, this.observedLineSegments, 0.02, '#000088');

            if (this.agentPosition)
                FieldLinePlotter.drawAgentPosition(context, options, this.agentPosition);

            if (this.particles)
                FieldLinePlotter.drawParticles(context, options, this.particles);

            if (this.visibleFieldPoly)
                FieldLinePlotter.drawVisibleFieldPoly(context, options, this.visibleFieldPoly);

            if (this.ballPosition)
                FieldLinePlotter.drawBall(context, options, this.ballPosition);

            if (this.goalPositions) {
                options.goalStrokeStyle = '#FF5800';
                FieldLinePlotter.drawGoalPosts(context, options, this.goalPositions);
            }

            this.needsRender = false;
        };

        return World2dModule;
    }
);
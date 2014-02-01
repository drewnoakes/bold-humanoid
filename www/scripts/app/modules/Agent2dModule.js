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
        'util/Dragger',
        'util/MouseEventUtil',
        'util/Geometry'
    ],
    function(FieldLinePlotter, Protocols, Constants, ControlBuilder, DataProxy, Dragger, MouseEventUtil, Geometry)
    {
        'use strict';

        var Transform = Geometry.Transform;

        var Agent2dModule = function()
        {
            this.$container = $('<div></div>');

            /////

            this.title = '2d agent';
            this.id = 'agent-2d';
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

        Agent2dModule.prototype.bindEvents = function()
        {
            this.$canvas.on('mousewheel', function (event)
            {
                event.preventDefault();
                this.scale *= Math.pow(1.1, event.originalEvent.wheelDelta / 80);
                this.scale = Math.max(20, this.scale);
                this.transform = new Transform()
                    .translate(this.canvas.width/2, this.canvas.height/2)
                    .scale(this.scale, -this.scale);
                this.needsRender = true;
            }.bind(this));

            this.$canvas.on('mousemove', function (event)
            {
                MouseEventUtil.polyfill(event);
                var p = this.transform.clone().invert().transformPoint(event.offsetX, event.offsetY);
                this.$hoverInfo.text(p.x.toFixed(2) + ', ' + p.y.toFixed(2));
            }.bind(this));

            this.$canvas.on('mouseleave', function() { this.$hoverInfo.text(''); }.bind(this));
        };

        Agent2dModule.prototype.load = function()
        {
            this.transform = new Transform();

            this.$canvas = $('<canvas></canvas>');
            this.canvas = this.$canvas.get(0);
            this.$hoverInfo = $('<div></div>', {'class': 'hover-info'});

            var headControlsContainer = document.createElement('div');
            ControlBuilder.action('head-module.move-down', headControlsContainer);
            ControlBuilder.action('head-module.move-up', headControlsContainer);
            ControlBuilder.action('head-module.move-left', headControlsContainer);
            ControlBuilder.action('head-module.move-right', headControlsContainer);
            ControlBuilder.action('head-module.move-home', headControlsContainer);
            ControlBuilder.action('head-module.move-zero', headControlsContainer);

            this.$container.append(this.$canvas)
                           .append(this.$hoverInfo)
                           .append(headControlsContainer);

            this.bindEvents();

            this.agentFrameSubscription = DataProxy.subscribe(Protocols.agentFrameState, { json: true, onmessage: _.bind(this.onAgentFrameData, this) });

            this.stopAnimation = false;
            this.needsRender = true;
            this.animate();
        };

        Agent2dModule.prototype.unload = function()
        {
            this.stopAnimation = true;
            this.$container.empty();
            this.agentFrameSubscription.close();
        };

        Agent2dModule.prototype.onAgentFrameData = function(data)
        {
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

            this.needsRender = true; // TODO only draw agentFrameData, on its own canvas
        };

        Agent2dModule.prototype.onResized = function(width, height)
        {
            this.canvas.width = width;
            this.canvas.height = height;

            this.scale = Math.min(width / 12, height / 12);
            this.transform = new Transform()
                .translate(this.canvas.width/2, this.canvas.height/2)
                .scale(this.scale, -this.scale);
            this.needsRender = true;
        };

        Agent2dModule.prototype.animate = function()
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
                    visibleFieldPolyStrokeStyle: '#0000ff'
                },
                context = this.canvas.getContext('2d');

            this.transform.applyTo(context);

            FieldLinePlotter.drawField(context, options);

            var maxDistance = Math.sqrt(
                Math.pow(Constants.fieldX + 2*Constants.outerMarginMinimum, 2) +
                Math.pow(Constants.fieldY + 2*Constants.outerMarginMinimum, 2));

            context.strokeStyle = 'white';
            context.lineWidth = 0.5/this.scale;
            context.beginPath();
            context.moveTo(0, maxDistance);
            context.lineTo(0, -maxDistance);
            context.moveTo(-maxDistance, 0);
            context.lineTo(maxDistance, 0);
            context.stroke();

            for (var r = 1; r < maxDistance; r++) {
                context.beginPath();
                context.arc(0, 0, r, 0, Math.PI*2);
                context.stroke();
            }

            if (this.observedLineSegments && this.observedLineSegments.length)
                FieldLinePlotter.drawLineSegments(context, options, this.observedLineSegments, 1, '#0000ff');

            if (this.visibleFieldPoly)
                FieldLinePlotter.drawVisibleFieldPoly(context, options, this.visibleFieldPoly);

            if (this.ballPosition)
                FieldLinePlotter.drawBall(context, options, this.ballPosition);

            if (this.goalPositions)
                FieldLinePlotter.drawGoalPosts(context, options, this.goalPositions);

            this.needsRender = false;
        };

        return Agent2dModule;
    }
);
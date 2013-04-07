/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/FieldLinePlotter',
        'scripts/app/Constants'
    ],
    function(FieldLinePlotter, Constants)
    {
        'use strict';

        // A simple module, with a full screen canvas as its element

        var FieldMapModule = function()
        {
            this.$canvas = $('<canvas></canvas>');
            this.canvas = this.$canvas.get(0);

            this.bindEvents();

            /////

            this.title = 'field';
            this.moduleClass = 'field';
            this.panes = [
                {
                    title: 'main',
                    element: this.canvas,
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        FieldMapModule.prototype.bindEvents = function()
        {
            var self = this;
            this.$canvas.on('mousewheel', function (event)
            {
                // TODO zoom around the mouse pointer, rather than (0,0)
                self.scale += event.originalEvent.wheelDelta / 20;
                self.scale = Math.max(self.minScale, self.scale);
                self.draw();
            });

            var isMouseDown = false, dragStartX, dragStartY;
            this.$canvas.on('mousedown', function(event)
            {
                isMouseDown = true;
                dragStartX = event.screenX;
                dragStartY = event.screenY;
                self.draw();
            });

            this.$canvas.on('mouseup', function() {
                isMouseDown = false;
            });

            this.$canvas.on('mousemove', function(event) {
                if (isMouseDown) {
                    var dx = event.screenX - dragStartX,
                        dy = event.screenY - dragStartY;
                    dragStartX = event.screenX;
                    dragStartY = event.screenY;
                    self.fieldCenterX += dx;
                    self.fieldCenterY += dy;
                    self.draw();
                }
            });
        };

        FieldMapModule.prototype.load = function()
        {};

        FieldMapModule.prototype.unload = function()
        {};

        FieldMapModule.prototype.onResized = function(width, height)
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

        FieldMapModule.prototype.draw = function()
        {
            var options = {
                    scale: this.scale,
                    goalStrokeStyle: 'yellow',
                    groundFillStyle: '#008800',
                    lineStrokeStyle: '#ffffff',
                    fieldCenter: { x: this.fieldCenterX, y: this.fieldCenterY }
                },
                context = this.canvas.getContext('2d');

            FieldLinePlotter.start(context, options);
            FieldLinePlotter.drawLines(context, options);
            FieldLinePlotter.drawGoals(context, options);
            FieldLinePlotter.end(context);
        };

        return FieldMapModule;
    }
);
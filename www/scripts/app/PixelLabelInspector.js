/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlClient',
        'util/CanvasExtensions'
    ],
    function(ControlClient)
    {
        'use strict';

        var PixelLabelInspector = function(width, height)
        {
            this.canvas = document.createElement('canvas');
            this.canvas.className = 'pixel-label-inspector';
            this.canvas.width = width;
            this.canvas.height = height;
            this.context = this.canvas.getContext('2d');

            this.labels = {};
            this.ready = false;

            this.labels.ball  = {hue:{}, saturation:{}, value:{}, colour:'red'};
            this.labels.line  = {hue:{}, saturation:{}, value:{}, colour:'white'};
            this.labels.goal  = {hue:{}, saturation:{}, value:{}, colour:'yellow'};
            this.labels.field = {hue:{}, saturation:{}, value:{}, colour:'green'};

            ControlClient.withData('vision/lut', function(controls)
            {
                _.each(controls, function (control)
                {
                    switch (control.name)
                    {
                        case 'Ball Hue Min': this.labels.ball.hue.min = control.value; break;
                        case 'Ball Hue Max': this.labels.ball.hue.max = control.value; break;
                        case 'Ball Sat Min': this.labels.ball.saturation.min = control.value; break;
                        case 'Ball Sat Max': this.labels.ball.saturation.max = control.value; break;
                        case 'Ball Val Min': this.labels.ball.value.min = control.value; break;
                        case 'Ball Val Max': this.labels.ball.value.max = control.value; break;

                        case 'Line Hue Min': this.labels.line.hue.min = control.value; break;
                        case 'Line Hue Max': this.labels.line.hue.max = control.value; break;
                        case 'Line Sat Min': this.labels.line.saturation.min = control.value; break;
                        case 'Line Sat Max': this.labels.line.saturation.max = control.value; break;
                        case 'Line Val Min': this.labels.line.value.min = control.value; break;
                        case 'Line Val Max': this.labels.line.value.max = control.value; break;

                        case 'Goal Hue Min': this.labels.goal.hue.min = control.value; break;
                        case 'Goal Hue Max': this.labels.goal.hue.max = control.value; break;
                        case 'Goal Sat Min': this.labels.goal.saturation.min = control.value; break;
                        case 'Goal Sat Max': this.labels.goal.saturation.max = control.value; break;
                        case 'Goal Val Min': this.labels.goal.value.min = control.value; break;
                        case 'Goal Val Max': this.labels.goal.value.max = control.value; break;

                        case 'Field Hue Min': this.labels.field.hue.min = control.value; break;
                        case 'Field Hue Max': this.labels.field.hue.max = control.value; break;
                        case 'Field Sat Min': this.labels.field.saturation.min = control.value; break;
                        case 'Field Sat Max': this.labels.field.saturation.max = control.value; break;
                        case 'Field Val Min': this.labels.field.value.min = control.value; break;
                        case 'Field Val Max': this.labels.field.value.max = control.value; break;
                    }
                }.bind(this));

                this.ready = true;

            }.bind(this));
        };

        PixelLabelInspector.prototype.highlightHsv = function(hsv)
        {
            this.hsv = hsv;
            this.draw();
        };

        PixelLabelInspector.prototype.draw = function()
        {
            var context = this.context;

            context.fillStyle = '#000';
            context.clear();

            context.beginPath();

            var gutterWidth = 25,
                barHeight = 4,
                barSpacing = 2,
                componentSpacing = 2,
                components = ['hue', 'saturation', 'value'],
                space = this.canvas.width - gutterWidth;

            var y = 0;
            for (var component = 0; component < 3; component++)
            {
                if (component != 0)
                {
                    // Draw horizontal separator line
                    context.lineWidth = 1;
                    context.strokeStyle = 'rgba(0,0,0,0.3)';
                    context.dashedLine(0, Math.floor(y) + 0.5, this.canvas.width, y, [1,1]);
                    context.stroke();
                    y++;
                }

                // Text label
                context.fillStyle = 'black';
                context.fillText('HSV'[component], 8, y + 16);

                var startY = y;

                y += componentSpacing;

                _.each(_.values(this.labels), function (label)
                {
                    var labelData = label[components[component]],
                        min = labelData.min,
                        max = labelData.max;

                    if (min < 0)
                        min = 0;

                    if (max > 255)
                        max = 255;

                    var drawComponentRange = function(y, minValue, maxValue)
                    {
                        var minRatio = minValue/255,
                            maxRatio = maxValue/255,
                            minX = minRatio*space,
                            maxX = maxRatio*space;

                        context.fillStyle = label.colour;
                        context.fillRect(gutterWidth + minX, y, maxX - minX, barHeight);
                    };

                    if (min < max)
                    {
                        drawComponentRange(y, min, max);
                    }
                    else
                    {
                        // Deal with the fact that hue wraps around
                        drawComponentRange(y, min, 255);
                        drawComponentRange(y, 0, max);
                    }

                    y += barHeight + barSpacing;

                }.bind(this));

                y += componentSpacing;

                if (this.hsv)
                {
                    var hoverRatio = this.hsv['hsv'[component]],
                        hoverX = Math.floor(gutterWidth + hoverRatio*space) + 0.5;

                    context.strokeStyle = 'rgba(0,0,0,0.7)';
                    context.moveTo(hoverX, startY);
                    context.lineTo(hoverX, y);
                    context.stroke();
                }
            }
        };

        PixelLabelInspector.prototype.setVisible = function(visible)
        {
            this.canvas.style.display = visible ? 'block' : 'none';
        };

        return PixelLabelInspector;
    }
);
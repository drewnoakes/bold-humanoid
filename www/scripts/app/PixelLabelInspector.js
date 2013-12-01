/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlClient',
        'HsvRange',
        'util/CanvasExtensions'
    ],
    function(ControlClient, HsvRange)
    {
        'use strict';

        var PixelLabelInspector = function(canvas, width, height)
        {
            this.canvas = canvas;
            this.canvas.width = width;
            this.canvas.height = height;
            this.canvas.style.width = width;
            this.canvas.style.height = height;
            this.context = this.canvas.getContext('2d');

            this.ready = false;

            ControlClient.withSettings('vision.pixel-labels', function(settings)
            {
                var ballSetting, goalSetting, fieldSetting, lineSetting;

                _.each(settings, function (setting)
                {
                    switch (setting.path)
                    {
                        case 'vision.pixel-labels.ball':  ballSetting  = setting; break;
                        case 'vision.pixel-labels.goal':  goalSetting  = setting; break;
                        case 'vision.pixel-labels.field': fieldSetting = setting; break;
                        case 'vision.pixel-labels.line':  lineSetting  = setting; break;
                    }
                }.bind(this));

                this.settings = [ballSetting, goalSetting, fieldSetting, lineSetting];
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
                componentNames = ['hue', 'sat', 'val'],
                space = this.canvas.width - gutterWidth;

            var y = 0;
            for (var componentIndex = 0; componentIndex < 3; componentIndex++)
            {
                if (componentIndex != 0)
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
                context.fillText('HSV'[componentIndex], 8, y + 16);

                var componentName = componentNames[componentIndex];

                var startY = y;

                y += componentSpacing;

                _.each(this.settings, function(setting)
                {
                    var val = setting.value;
                    var labelData = val[componentName],
                        min = labelData[0],
                        max = labelData[1];

                    console.assert(min >= 0);
                    console.assert(max <= 255);

                    if (min < 0)
                        min = 0;

                    if (max > 255)
                        max = 255;

                    var labelColour = HsvRange.calculateColour(val);

                    var drawComponentRange = function(y, minValue, maxValue)
                    {
                        var minRatio = minValue/255,
                            maxRatio = maxValue/255,
                            minX = minRatio*space,
                            maxX = maxRatio*space;

                        context.fillStyle = labelColour;
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
                    var hoverRatio = this.hsv['hsv'[componentIndex]],
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
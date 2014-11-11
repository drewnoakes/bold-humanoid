/**
 * @author Drew Noakes http://drewnoakes.com
 */

import HsvRange = require ('../HsvRange');
import color = require ('../color');
import control = require ('../control');
import Setting = require('Setting');
import canvasUtil = require('util/canvas');

class PixelLabelInspector
{
    private context: CanvasRenderingContext2D;
    private ready: boolean = false;
    private hsv: color.Hsv;
    private settings: Setting[];

    constructor(private canvas: HTMLCanvasElement, width: number, height: number)
    {
        this.canvas.width = width;
        this.canvas.height = height;
        this.canvas.style.width = width + 'px';
        this.canvas.style.height = height + 'px';
        this.context = this.canvas.getContext('2d');

        control.withSettings('vision.pixel-labels', settings =>
        {
            var ballSetting, goalSetting, fieldSetting, lineSetting, cyanSetting, magentaSetting;

            _.each(settings, setting =>
            {
                switch (setting.path) {
                    case 'vision.pixel-labels.ball':
                        ballSetting = setting;
                        break;
                    case 'vision.pixel-labels.goal':
                        goalSetting = setting;
                        break;
                    case 'vision.pixel-labels.field':
                        fieldSetting = setting;
                        break;
                    case 'vision.pixel-labels.line':
                        lineSetting = setting;
                        break;
                    case 'vision.pixel-labels.cyan':
                        cyanSetting = setting;
                        break;
                    case 'vision.pixel-labels.magenta':
                        magentaSetting = setting;
                        break;
                }
            });

            this.settings = [ballSetting, goalSetting, fieldSetting, lineSetting, cyanSetting, magentaSetting];
            this.ready = true;
        });
    }

    public highlightHsv(hsv: color.Hsv)
    {
        this.hsv = hsv;
        this.draw();
    }

    public draw()
    {
        var context = this.context;

        context.fillStyle = '#000';
        canvasUtil.clear(context);

        context.beginPath();

        var gutterWidth = 25,
            barHeight = 4,
            barSpacing = 2,
            componentSpacing = 2,
            componentNames = ['hue', 'sat', 'val'],
            space = this.canvas.width - gutterWidth;

        var y = 0;
        for (var componentIndex = 0; componentIndex < 3; componentIndex++) {
            if (componentIndex != 0) {
                // Draw horizontal separator line
                context.beginPath();
                context.lineWidth = 1;
                context.strokeStyle = 'rgba(0,0,0,0.3)';
                canvasUtil.dashedLine(context, 0, Math.floor(y) + 0.5, this.canvas.width, y, [1, 1]);
                context.stroke();
                y++;
            }

            // Text label
            context.fillStyle = 'black';
            context.fillText('HSV'[componentIndex], 8, y + 16);

            var componentName = componentNames[componentIndex];

            var startY = y;

            y += componentSpacing;

            _.each(this.settings, setting =>
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

                var drawComponentRange = (y, minValue, maxValue) =>
                {
                    var minRatio = minValue / 255,
                        maxRatio = maxValue / 255,
                        minX = minRatio * space,
                        maxX = maxRatio * space;

                    context.fillStyle = labelColour;
                    context.fillRect(gutterWidth + minX, y, maxX - minX, barHeight);
                };

                if (min < max) {
                    drawComponentRange(y, min, max);
                }
                else {
                    // Deal with the fact that hue wraps around
                    drawComponentRange(y, min, 255);
                    drawComponentRange(y, 0, max);
                }

                y += barHeight + barSpacing;

            });

            y += componentSpacing;

            if (this.hsv) {
                var hoverRatio = this.hsv['HSV'[componentIndex]],
                    hoverX = Math.floor(gutterWidth + hoverRatio * space) + 0.5;

                context.beginPath();
                context.strokeStyle = 'rgba(0,0,0,0.7)';
                context.moveTo(hoverX, startY);
                context.lineTo(hoverX, y);
                context.stroke();
            }
        }
    }

    public setVisible(visible)
    {
        // TODO client passes in canvas -- why don't they take care of this?
        this.canvas.style.display = visible ? 'block' : 'none';
    }
}

export = PixelLabelInspector;

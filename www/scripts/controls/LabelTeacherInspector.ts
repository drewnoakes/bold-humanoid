import HsvRange = require ('../HsvRange');
import color = require ('../color');
import control = require ('../control');
import Setting = require('Setting');
import canvasUtil = require('util/canvas');

class LabelTeacherInspector
{
    private context: CanvasRenderingContext2D;
    public hsvRange: any;
    public hsvDist: any;
    public sigmaRange: number;

    constructor(private canvas: HTMLCanvasElement, width: number, height: number)
    {
        this.canvas.width = width;
        this.canvas.height = height;
        this.canvas.style.width = width + 'px';
        this.canvas.style.height = height + 'px';
        this.context = this.canvas.getContext('2d');

        control.withSetting('label-teacher.sigma-range', (setting: Setting) =>
                            {
                                setting.track((val: number) =>
                                              {
                                                  this.sigmaRange = val;
                                                  this.draw();
                                              });
                            }
                           );

    }

    public draw()
    {
        if (!this.hsvRange)
            return;

        var context = this.context;

        context.fillStyle = '#000';
        canvasUtil.clear(context);

        context.beginPath();

        var gutterWidth = 25,
            barHeight = 8,
            barSpacing = 4,
            componentSpacing = 4,
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

            var componentName = componentNames[componentIndex];

            var labelData = this.hsvRange[componentName],
                min = labelData[0],
                max = labelData[1];

            // Text label
            context.fillStyle = 'black';
            context.fillText('HSV'[componentIndex], 8, y + 12);

            y += componentSpacing;

            var labelColour = HsvRange.calculateColour(this.hsvRange);

            var drawComponentRange = (y, minValue, maxValue) =>
            {
                var minRatio = minValue / 255,
                    maxRatio = maxValue / 255,
                    minX = minRatio * space,
                    maxX = maxRatio * space;

                context.fillStyle = labelColour;
                context.fillRect(gutterWidth + minX, y, maxX - minX, barHeight);
            };

            var min = this.hsvRange[componentName][0];
            var max = this.hsvRange[componentName][1];

            if (min < max) {
                drawComponentRange(y, Math.max(min, 0), Math.min(max, 255));
            }
            else {
                // Deal with the fact that hue wraps around
                drawComponentRange(y, min, 255);
                drawComponentRange(y, 0, max);
            }

            var drawSigmaRange = (y , minValue, maxValue) =>
            {
                var minRatio = minValue / 255,
                    maxRatio = maxValue / 255,
                    minX = minRatio * space,
                    maxX = maxRatio * space;

                context.moveTo(gutterWidth + minX, y);
                context.lineTo(gutterWidth + maxX, y);
            };

            var mean = this.hsvDist[componentName][0],
                sigma = this.hsvDist[componentName][1],
                sigmaMin = mean - this.sigmaRange * sigma,
                sigmaMax = mean + this.sigmaRange * sigma,
                
                meanRatio = this.hsvDist[componentName][0] / 255,
                meanX = gutterWidth + meanRatio * space;
            
            context.beginPath();
            context.lineWidth = 1;
            context.strokeStyle = 'rgba(0,0,0,0.9)';
            context.moveTo(meanX, y - .5 * barSpacing);
            context.lineTo(meanX, y + barHeight + .5 *barSpacing);

            var sigmaY = y + .5 * barHeight;
            drawSigmaRange(sigmaY, Math.max(0, sigmaMin), Math.min(255, sigmaMax));
            
            if (componentName == 'hue')
            {
                if (sigmaMin < 0)
                    drawSigmaRange(sigmaY, 255 + sigmaMin, 255);
                if (sigmaMax > 255)
                    drawSigmaRange(sigmaY, 0, sigmaMax - 255);
            }
            else
            {
                drawSigmaRange(y + .5 * barHeight, Math.max(0, sigmaMin), Math.min(255, sigmaMax));
            }
            context.stroke();

            y += barHeight + barSpacing;
        }
    }

    public setVisible(visible)
    {
        // TODO client passes in canvas -- why don't they take care of this?
        this.canvas.style.display = visible ? 'block' : 'none';
    }
}

export = LabelTeacherInspector;

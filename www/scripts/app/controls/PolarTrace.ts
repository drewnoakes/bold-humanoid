/**
 * @author Drew Noakes http://drewnoakes.com
 */

import geometry = require('../util/geometry');

class PolarTrace
{
    private length: number = 200;
    private lineStyle: string = '#44C425';
    private axisStyle: string = 'rgba(255,255,255,0.5)';
    private ringStyle: string = 'rgba(255,255,255,0.5)';
    private maxValue: number = 1.5;
    private dotSize: number = 6;
    private ringValues: number[] = [0.5, 1, 1.5];
    private fadeRate: number = 0.02;

    private scale: number;

    private overlayCanvas: HTMLCanvasElement;
    private seriesCanvas: HTMLCanvasElement;
    private seriesContext: CanvasRenderingContext2D;

    private lastPosition: geometry.IPoint2;

    public element: HTMLDivElement;

    constructor()
    {
        this.scale = this.length / 2 / this.maxValue;
        this.seriesCanvas = document.createElement('canvas');
        this.seriesCanvas.width = this.seriesCanvas.height = this.length;
        this.seriesContext = this.seriesCanvas.getContext('2d');

        this.overlayCanvas = document.createElement('canvas');
        this.overlayCanvas.style.position = 'absolute';
        this.overlayCanvas.style.left = '0';
        this.overlayCanvas.width = this.overlayCanvas.height = this.length;

        this.renderOverlay();

        this.element = document.createElement('div');
        this.element.className = 'polar-trace';
        this.element.style.width = this.length + 'px';
        this.element.style.height = this.length + 'px';
        this.element.appendChild(this.seriesCanvas);
        this.element.appendChild(this.overlayCanvas);

        this.seriesContext.fillStyle = '#000000';
        this.seriesContext.fillRect(0, 0, this.length, this.length);
    }

    public renderOverlay()
    {
        var overlayContext = this.overlayCanvas.getContext('2d');

        // draw axes
        overlayContext.translate(this.length / 2, this.length / 2);
        overlayContext.strokeStyle = this.axisStyle;
        overlayContext.lineWidth = 1;
        overlayContext.moveTo(0.5, -this.length / 2);
        overlayContext.lineTo(0.5, this.length / 2);
        overlayContext.moveTo(-this.length / 2, 0.5);
        overlayContext.lineTo(this.length / 2, 0.5);
        overlayContext.stroke();

        // draw rings
        overlayContext.strokeStyle = this.ringStyle;
        overlayContext.lineWidth = 0.5;
        for (var i = 0; i < this.ringValues.length; i++) {
            var ringValue = this.ringValues[i],
                radius = this.scale * ringValue;
            overlayContext.beginPath();
            overlayContext.arc(0, 0, radius, 0, 2 * Math.PI, false);
            overlayContext.stroke();
        }
    }

    public addValue(xValue: number, yValue: number)
    {
        var context = this.seriesContext;

        context.save();
        context.fillStyle = "rgba(0, 0, 0, " + this.fadeRate + ")";
        context.fillRect(0, 0, this.length, this.length);

        context.translate(this.length / 2, this.length / 2);

        var x = xValue * this.scale,
            y = yValue * this.scale;

        // draw line segment
        if (this.lastPosition) {
            context.strokeStyle = this.lineStyle;
            context.lineWidth = this.dotSize;
            context.beginPath();
            context.moveTo(this.lastPosition.x, this.lastPosition.y);
            context.lineTo(x, y);
            context.stroke();
        }

        this.lastPosition = {x: x, y: y};

        context.restore();
    }
}

export = PolarTrace;

/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        //noinspection UnnecessaryLocalVariableJS
        var PolarTrace = function ()
        {
            this.length = 200;
            this.lineStyle = '#44C425';
            this.axisStyle = 'rgba(255,255,255,0.5)';
            this.ringStyle = 'rgba(255,255,255,0.5)';
            this.maxValue = 1.5;
            this.dotSize = 6;
            this.ringValues = [0.5, 1, 1.5];
            this.fadeRate = 0.02;
            this.scale = this.length / 2 / this.maxValue;
            this.$seriesCanvas = $('<canvas></canvas>');
            this.seriesCanvas = this.$seriesCanvas.get(0);
            this.seriesCanvas.width = this.seriesCanvas.height = this.length;
            this.seriesContext = this.seriesCanvas.getContext('2d');

            this.overlayCanvas = $('<canvas></canvas>').css({position: 'absolute', left: '0'}).get(0);
            this.overlayCanvas.width = this.overlayCanvas.height = this.length;

            this.renderOverlay();

            this.element = $('<div></div>', {'class': 'polar-trace'})
                .css({width: this.length, height: this.length})
                .append(this.seriesCanvas)
                .append(this.overlayCanvas)
                .get(0);

            this.seriesContext.fillStyle = '#000000';
            this.seriesContext.fillRect(0, 0, this.length, this.length);
        };

        PolarTrace.prototype.renderOverlay = function ()
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
        };

        PolarTrace.prototype.addValue = function (xValue, yValue)
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
        };

        return PolarTrace;
    }
);
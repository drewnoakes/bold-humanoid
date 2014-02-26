/**
 * @author Drew Noakes http://drewnoakes.com
 */

var context2DPrototype = CanvasRenderingContext2D.prototype;

context2DPrototype.clear =
    context2DPrototype.clear ||
        function(preserveTransform)
        {
            if (preserveTransform) {
                this.save();
                this.setTransform(1, 0, 0, 1, 0, 0);
            }

            this.clearRect(0, 0, this.canvas.width, this.canvas.height);

            if (preserveTransform) {
                this.restore();
            }
        };

context2DPrototype.ellipse =
    context2DPrototype.ellipse ||
        function(x, y, width, height)
        {
            this.save();
            this.translate(x - width/2, y - height/2);
            this.scale(width, height);
            this.arc(0, 0, 1, 0, 2 * Math.PI, false);
            this.restore();
        };

context2DPrototype.circle =
    context2DPrototype.circle ||
        function(x, y, radius)
        {
            this.arc(x, y, radius, 0, 2 * Math.PI, false);
        };

context2DPrototype.noisyCircle =
    context2DPrototype.noisyCircle ||
        function(x, y, radius, stepCount, angleNoise, radiusNoise, rotation)
        {
            rotation = typeof(rotation) !== "number" ? 0 : rotation;
            angleNoise = typeof(angleNoise) !== "number" ? 0.2 : angleNoise;
            radiusNoise = typeof(radiusNoise) !== "number" ? 0.2 : radiusNoise;

            var maxAngleNoise = angleNoise * (2 * Math.PI / stepCount) / 2;
            var maxRadiusNoise = radiusNoise * radius;

            for (var step = 0; step < stepCount; step++) {
                var angle = rotation + step/(stepCount-1) * Math.PI * 2;

                angle += ((Math.random()*2) - 1) * maxAngleNoise;
                var r = radius + ((Math.random()*2)-1) * maxRadiusNoise;

                if (step == stepCount - 1)
                    this.closePath();
                else
                    this.lineTo(
                        x + r * Math.cos(angle),
                        y + r * Math.sin(angle));
            }
        };

context2DPrototype.drawSpiral =
    context2DPrototype.drawSpiral ||
        function(centerX, centerY, stepCount, loopCount, innerDistance, loopSpacing, rotation)
        {
            var stepSize = 2 * Math.PI / stepCount,
                endAngle = 2 * Math.PI * loopCount,
                finished = false;

            this.beginPath();

            for (var angle = 0; !finished; angle += stepSize) {
                // Ensure that the spiral finishes at the correct place,
                // avoiding any drift introduced by cumulative errors from
                // repeatedly adding floating point numbers.
                if (angle > endAngle) {
                    angle = endAngle - (endAngle % stepSize);
                    finished = true;
                }

                var scalar = innerDistance + loopSpacing * angle,
                    rotatedAngle = angle + rotation,
                    x = centerX + scalar * Math.cos(rotatedAngle),
                    y = centerY + scalar * Math.sin(rotatedAngle);

                this.lineTo(x, y);
            }

            this.stroke();
        };

if (context2DPrototype.lineTo) {
    context2DPrototype.dashedLine =
        context2DPrototype.dashedLine ||
            function(x, y, x2, y2, da)
            {
                da = da || [10, 5];
                this.save();
                var dx = (x2 - x),
                    dy = (y2 - y),
                    len = Math.sqrt(dx * dx + dy * dy),
                    rot = Math.atan2(dy, dx);
                this.translate(x, y);
                this.moveTo(0, 0);
                if (rot !== 0)
                    this.rotate(rot);
                var dc = da.length;
                var di = 0,
                    draw = true;
                x = 0;
                while (len > x) {
                    x += da[di++ % dc];
                    if (x > len) x = len;
                    draw
                        ? this.lineTo(x, 0)
                        : this.moveTo(x, 0);
                    draw = !draw;
                }
                this.restore();
            };
}

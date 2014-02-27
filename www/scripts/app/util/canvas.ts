/**
 * @author Drew Noakes http://drewnoakes.com
 */

export function clear(context: CanvasRenderingContext2D, preserveTransform: boolean = false)
{
    if (preserveTransform) {
        context.save();
        context.setTransform(1, 0, 0, 1, 0, 0);
    }

    context.clearRect(0, 0, context.canvas.width, context.canvas.height);

    if (preserveTransform) {
        context.restore();
    }
}

export function ellipse(context: CanvasRenderingContext2D, x: number, y: number, width: number, height: number)
{
    context.save();
    context.translate(x - width/2, y - height/2);
    context.scale(width, height);
    context.arc(0, 0, 1, 0, 2 * Math.PI, false);
    context.restore();
}

export function circle(context: CanvasRenderingContext2D, x: number, y: number, radius: number)
{
    context.arc(x, y, radius, 0, 2 * Math.PI, false);
}

export function noisyCircle(context: CanvasRenderingContext2D, x: number, y: number, radius: number, stepCount: number, angleNoise?: number, radiusNoise?: number, rotation?: number)
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
            context.closePath();
        else
            context.lineTo(
                x + r * Math.cos(angle),
                y + r * Math.sin(angle));
    }
}

export function drawSpiral(context: CanvasRenderingContext2D, centerX: number, centerY: number, stepCount: number, loopCount: number, innerDistance: number, loopSpacing: number, rotation: number)
{
    var stepSize = 2 * Math.PI / stepCount,
        endAngle = 2 * Math.PI * loopCount,
        finished = false;

    context.beginPath();

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

        context.lineTo(x, y);
    }

    context.stroke();
}

export function dashedLine(context: CanvasRenderingContext2D, x: number, y: number, x2: number, y2: number, da: number[])
{
    da = da || [10, 5];
    context.save();
    var dx = (x2 - x),
        dy = (y2 - y),
        len = Math.sqrt(dx * dx + dy * dy),
        rot = Math.atan2(dy, dx);
    context.translate(x, y);
    context.moveTo(0, 0);
    if (rot !== 0)
        context.rotate(rot);
    var dc = da.length;
    var di = 0,
        draw = true;
    x = 0;
    while (len > x) {
        x += da[di++ % dc];
        if (x > len) x = len;
        draw
            ? context.lineTo(x, 0)
            : context.moveTo(x, 0);
        draw = !draw;
    }
    context.restore();
}

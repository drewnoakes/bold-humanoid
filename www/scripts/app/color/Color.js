/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'color/RgbColor'
    ],
    function(
        RgbColor
    )
    {
        return {
            blend: function(from, to, ratio)
            {
                if (ratio <= 0)
                    return from;
                if (ratio >= 1)
                    return to;

                var from1 = from.getValue(0);
                var from2 = from.getValue(1);
                var from3 = from.getValue(2);

                var r = from1 + ratio * (to.getValue(0) - from1);
                var g = from2 + ratio * (to.getValue(1) - from2);
                var b = from3 + ratio * (to.getValue(2) - from3);

                return new from.ColorType(r, g, b);
            },

            fromContext2d: function(context, x, y)
            {
                var pixelData = context.getImageData(x, y, 1, 1).data;

                var r = pixelData[0] / 255;
                var g = pixelData[1] / 255;
                var b = pixelData[2] / 255;

                return new RgbColor(r, g, b);
            }
        };
    }
);
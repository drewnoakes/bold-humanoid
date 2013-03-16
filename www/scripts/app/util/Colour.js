/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        //noinspection UnnecessaryLocalVariableJS
        var Colour =
        {
            rgbToHsv: function (rgb)
            {
                var r = rgb.r,
                    g = rgb.g,
                    b = rgb.b;

                var max = Math.max(r, g, b),
                    min = Math.min(r, g, b),
                    d = max - min;

                var hsv = {};
                if (max == min) {
                    // achromatic
                    hsv.h = 0;
                } else {
                    var h;
                    switch (max) {
                        case r:
                            h = (g - b) / d + (g < b ? 6 : 0);
                            break;
                        case g:
                            h = (b - r) / d + 2;
                            break;
                        case b:
                            h = (r - g) / d + 4;
                            break;
                    }
                    h /= 6;
                    hsv.h = h;
                }

                hsv.s = max == 0 ? 0 : d / max;
                hsv.v = max;

                return hsv;
            }
        };

        return Colour;
    }
);
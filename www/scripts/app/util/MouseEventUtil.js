/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        return {
            polyfill: function(e)
            {
                if (typeof(e.offsetX) !== 'undefined' && typeof(e.offsetY) !== 'undefined')
                    return;

                var target = e.target || e.srcElement,
                    rect = target.getBoundingClientRect();

                e.offsetX = e.clientX - rect.left;
                e.offsetY = e.clientY - rect.top;
            }
        };
    }
);
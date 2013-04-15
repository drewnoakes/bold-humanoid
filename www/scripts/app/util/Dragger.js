/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        //noinspection UnnecessaryLocalVariableJS
        var Dragger = {
            bind: function(element, functions)
            {
                var onMouseDownPosition = new THREE.Vector2();

                var onMouseMove = function(event)
                {
                    event.preventDefault();
                    var dx = event.clientX - onMouseDownPosition.x,
                        dy = event.clientY - onMouseDownPosition.y;
                    if (functions.move) {
                        functions.move(dx, dy);
                    }
                    if (functions.isRelative) {
                        onMouseDownPosition.x = event.clientX;
                        onMouseDownPosition.y = event.clientY;
                    }
                };

                var onMouseUp = function (event)
                {
                    event.preventDefault();
                    window.removeEventListener('mouseup', onMouseUp, false);
                    window.removeEventListener('mousemove', onMouseMove, false);
                    if (functions.stop) {
                        functions.stop();
                    }
                };

                element.addEventListener('mousedown', function(event)
                {
                    event.preventDefault();
                    if (functions.start) {
                        functions.start();
                    }
                    onMouseDownPosition.x = event.clientX;
                    onMouseDownPosition.y = event.clientY;
                    window.addEventListener('mouseup', onMouseUp, false);
                    window.addEventListener('mousemove', onMouseMove, false);
                }, false);

            }
        };

        return Dragger;
    }
);
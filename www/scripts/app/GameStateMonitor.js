/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/DataProxy',
        'scripts/app/Protocols'
    ],
    function(DataProxy, Protocols)
    {
        'use strict';

        //noinspection UnnecessaryLocalVariableJS
        var GameStateMonitor = function()
        {
            var subscription = DataProxy.subscribe(
                Protocols.gameState,
                {
                    json: true,
                    onmessage: function (data)
                    {
                        // TODO actually use object structure, rather than populating the text with JSON
                        $('#secondsRemaining').text(data);
                    }
                }
            );

            // TODO retain subscription and potentially cancel?
        };

        return GameStateMonitor;
    }
);
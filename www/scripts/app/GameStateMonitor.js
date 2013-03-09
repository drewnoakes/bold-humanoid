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
        //noinspection UnnecessaryLocalVariableJS
        var GameStateMonitor = function()
        {
            var subscription = DataProxy.subscribe(
                Protocols.gameState,
                {
                    onmessage: function (msg)
                    {
                        $('#secondsRemaining').text(msg.data);
                    }
                }
            );

            // TODO retain subscription and potentially cancel?
        };

        return GameStateMonitor;
    }
);
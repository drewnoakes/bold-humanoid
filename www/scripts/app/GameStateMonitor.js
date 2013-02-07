define(
    [
        'scripts/app/WebSocketFactory'
    ],
    function(WebSocketFactory)
    {
        //noinspection UnnecessaryLocalVariableJS
        var GameStateMonitor = function()
        {
            var socket = WebSocketFactory.open("game-state-protocol");
            socket.onmessage = function (msg)
            {
                $('#secondsRemaining').text(msg.data);
            }
        };

        return GameStateMonitor;
    }
);
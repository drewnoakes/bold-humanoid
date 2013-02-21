define(
    [
        'scripts/app/WebSocketFactory'
    ],
    function(WebSocketFactory)
    {
        var protocol = 'camera-protocol';

        var socket = WebSocketFactory.open(protocol);
    }
);
require(
    [
        'scripts/app/FieldMap',
        'scripts/app/GameStateMonitor',
        'scripts/app/WebSocketFactory',
        'scripts/app/StreamingCharts'
    ],
    function(FieldMap, GameStateMonitor, WebSocketFactory, StreamingCharts)
    {
        WebSocketFactory.toString();

        new FieldMap($('#field-map').get(0));

        new GameStateMonitor();
    }
);
require(
    [
        'scripts/app/FieldMap',
        'scripts/app/GameStateMonitor',
        'scripts/app/WebSocketFactory',
        'scripts/app/StreamingCharts',
        'scripts/app/Model'
    ],
    function(FieldMap, GameStateMonitor, WebSocketFactory)
    {
//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

        WebSocketFactory.toString();

        new FieldMap($('#field-map').get(0));

        new GameStateMonitor();
    }
);
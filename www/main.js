require(
    [
        'scripts/app/FieldMap',
        'scripts/app/GameStateMonitor',
        'scripts/app/Camera.js',
        'scripts/app/StreamingCharts',
        'scripts/app/TimingModule',
        'scripts/app/Model'
    ],
    function(FieldMap, GameStateMonitor)
    {
//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

        new FieldMap($('#field-map').get(0));

        new GameStateMonitor();
    }
);
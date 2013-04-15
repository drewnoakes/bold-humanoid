require(
    [
        'scripts/app/ModuleHost',
        'scripts/app/modules/FieldMapModule',
        'scripts/app/modules/TimingModule',
        'scripts/app/modules/ModelModule',
        'scripts/app/modules/CameraModule',
        'scripts/app/modules/SensorModule',
        'scripts/app/modules/StateModule',
        'scripts/app/GameStateMonitor'
    ],
    function(ModuleHost, FieldMapModule, TimingModule, ModelModule, CameraModule, SensorModule, StateModule, GameStateMonitor)
    {
//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

        var moduleHost = new ModuleHost();

        moduleHost.add(new CameraModule());
        moduleHost.add(new FieldMapModule());
        moduleHost.add(new ModelModule());
        moduleHost.add(new TimingModule());
        moduleHost.add(new SensorModule());
        moduleHost.add(new StateModule());

        moduleHost.load();

        new GameStateMonitor();
    }
);
require(
    [
        'ControlClient',
        'ModuleHost',
        'modules/CameraModule',
        'modules/CommsModule',
        'modules/GameStateModule',
        'modules/IMUModule',
        'modules/LocaliserModule',
        'modules/MotionTimingModule',
        'modules/OptionTreeModule',
        'modules/StateDumpModule',
        'modules/ThinkTimingModule',
        'modules/WalkModule',
        'modules/World2dModule',
        'modules/World3dModule'
    ],
    function(ControlClient, ModuleHost, CameraModule, CommsModule, GameStateModule, IMUModule, LocaliserModule, MotionTimingModule, OptionTreeModule, StateDumpModule, ThinkTimingModule, WalkModule, World2dModule, World3dModule)
    {
//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

        var moduleHost = new ModuleHost('#header-module-links');

        moduleHost.register(new CameraModule());
        moduleHost.register(new World3dModule());
        moduleHost.register(new World2dModule());
        moduleHost.register(new ThinkTimingModule());
        moduleHost.register(new MotionTimingModule());
        moduleHost.register(new LocaliserModule());
        moduleHost.register(new WalkModule());
        moduleHost.register(new CommsModule());
        moduleHost.register(new IMUModule());
        moduleHost.register(new OptionTreeModule());
        moduleHost.register(new GameStateModule());
        moduleHost.register(new StateDumpModule());

        moduleHost.load();

        ControlClient.connect();
    }
);
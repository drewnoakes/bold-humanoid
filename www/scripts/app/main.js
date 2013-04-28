require(
    [
        'ModuleHost',
        'modules/World2dModule',
        'modules/TimingModule',
        'modules/World3dModule',
        'modules/CameraModule',
        'modules/OptionTreeModule',
        'modules/IMUModule',
        'modules/StateDumpModule',
        'modules/LocaliserModule',
        'modules/WalkModule',
        'modules/GameStateModule',
        'ControlClient'
    ],
    function(ModuleHost, World2dModule, TimingModule, World3dModule, CameraModule, OptionTreeModule, IMUModule, StateDumpModule, LocaliserModule, WalkModule, GameStateModule, ControlClient)
    {
//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

        var moduleHost = new ModuleHost('#header-module-links');

        moduleHost.register(new CameraModule());
        moduleHost.register(new World3dModule());
        moduleHost.register(new World2dModule());
        moduleHost.register(new TimingModule());
        moduleHost.register(new LocaliserModule());
        moduleHost.register(new WalkModule());
        moduleHost.register(new IMUModule());
        moduleHost.register(new OptionTreeModule());
        moduleHost.register(new GameStateModule());
        moduleHost.register(new StateDumpModule());

        moduleHost.load();

        ControlClient.connect();
    }
);
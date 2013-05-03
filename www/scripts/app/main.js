require(
    [
        'ControlClient',
        'ModuleHost',
        'modules/CameraModule',
        'modules/CommsModule',
        'modules/GameStateModule',
        'modules/IMUModule',
        'modules/LocaliserModule',
        'modules/OptionTreeModule',
        'modules/StateDumpModule',
        'modules/TimingModule',
        'modules/WalkModule',
        'modules/World2dModule',
        'modules/World3dModule'
    ],
    function(ControlClient, ModuleHost, CameraModule, CommsModule, GameStateModule, IMUModule, LocaliserModule, OptionTreeModule, StateDumpModule, TimingModule, WalkModule, World2dModule, World3dModule)
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
        moduleHost.register(new CommsModule());
        moduleHost.register(new IMUModule());
        moduleHost.register(new OptionTreeModule());
        moduleHost.register(new GameStateModule());
        moduleHost.register(new StateDumpModule());

        moduleHost.load();

        ControlClient.connect();
    }
);
require(
    [
        'scripts/app/ModuleHost',
        'scripts/app/modules/World2dModule',
        'scripts/app/modules/TimingModule',
        'scripts/app/modules/World3dModule',
        'scripts/app/modules/CameraModule',
        'scripts/app/modules/OptionTreeModule',
        'scripts/app/modules/IMUModule',
        'scripts/app/modules/StateDumpModule',
        'scripts/app/modules/LocaliserModule',
        'scripts/app/ControlClient',
        'scripts/app/GameStateMonitor'
    ],
    function(ModuleHost, World2dModule, TimingModule, World3dModule, CameraModule, OptionTreeModule, IMUModule, StateDumpModule, LocaliserModule, ControlClient, GameStateMonitor)
    {
//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

        var moduleHost = new ModuleHost('#header-module-links');

        moduleHost.register(new CameraModule());
        moduleHost.register(new World3dModule());
        moduleHost.register(new World2dModule());
        moduleHost.register(new TimingModule());
        moduleHost.register(new LocaliserModule());
        moduleHost.register(new IMUModule());
        moduleHost.register(new OptionTreeModule());
        moduleHost.register(new StateDumpModule());

        moduleHost.load();

        new GameStateMonitor();

        ControlClient.connect();
    }
);
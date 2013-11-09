require(
    [
        'ControlClient',
        'ModuleHost',
        'modules/MotionScriptModule',
        'modules/CameraModule',
        'modules/CommsModule',
        'modules/GameStateModule',
        'modules/HardwareModule',
        'modules/HistogramModule',
        'modules/IMUModule',
        'modules/LocaliserModule',
        'modules/MotionTimingModule',
        'modules/OptionTreeModule',
        'modules/StateDumpModule',
        'modules/ThinkTimingModule',
        'modules/VoiceModule',
        'modules/WalkModule',
        'modules/World2dModule',
        'modules/World3dModule'
    ],
    function(ControlClient, ModuleHost, MotionScriptModule, CameraModule, CommsModule, GameStateModule, HardwareModule, HistogramModule, IMUModule, LocaliserModule, MotionTimingModule, OptionTreeModule, StateDumpModule, ThinkTimingModule, VoiceModule, WalkModule, World2dModule, World3dModule)
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
        moduleHost.register(new HardwareModule());
        moduleHost.register(new HistogramModule());
        moduleHost.register(new IMUModule());
        moduleHost.register(new OptionTreeModule());
        moduleHost.register(new GameStateModule());
        moduleHost.register(new MotionScriptModule());
        moduleHost.register(new VoiceModule());
        moduleHost.register(new StateDumpModule());

        moduleHost.load();

        ControlClient.connect();
    }
);
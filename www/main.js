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
        'scripts/app/GameStateMonitor'
    ],
    function(ModuleHost, World2dModule, TimingModule, World3dModule, CameraModule, OptionTreeModule, IMUModule, StateDumpModule, GameStateMonitor)
    {
//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

        var moduleHost = new ModuleHost();

        moduleHost.add(new CameraModule());
        moduleHost.add(new World3dModule());
        moduleHost.add(new TimingModule());
        moduleHost.add(new IMUModule());
        moduleHost.add(new World2dModule());
        moduleHost.add(new OptionTreeModule());
        moduleHost.add(new StateDumpModule());

        moduleHost.load();

        new GameStateMonitor();

        $('#module-container').sortable(); //.disableSelection();
    }
);
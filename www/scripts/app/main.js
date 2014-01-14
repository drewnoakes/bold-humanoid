/**
 * @author Drew Noakes http://drewnoakes.com
 */
require(
    [
        'ControlClient',
        'ModuleHost',
        'Constants',
        'modules/MotionScriptModule',
        'modules/CameraModule',
        'modules/CommsModule',
        'modules/ConfigModule',
        'modules/GameStateModule',
        'modules/HardwareModule',
        'modules/HistogramModule',
        'modules/IMUModule',
        'modules/OrientationModule',
        'modules/LocaliserModule',
        'modules/MotionTimingModule',
        'modules/OptionTreeModule',
        'modules/StateDumpModule',
        'modules/ThinkTimingModule',
        'modules/TrajectoryModule',
        'modules/VoiceModule',
        'modules/WalkModule',
        'modules/Agent2dModule',
        'modules/VisionModule',
        'modules/World2dModule',
        'modules/World3dModule'
    ],
    function(ControlClient, ModuleHost, Constants, MotionScriptModule, CameraModule, CommsModule, ConfigModule, GameStateModule, HardwareModule, HistogramModule, IMUModule, OrientationModule, LocaliserModule, MotionTimingModule, OptionTreeModule, StateDumpModule, ThinkTimingModule, TrajectoryModule, VoiceModule, WalkModule, Agent2dModule, VisionModule, World2dModule, World3dModule)
    {
//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

        var loadUi = function (settings)
        {
            Constants.update(settings);

            var moduleHost = new ModuleHost('#header-module-links');

            moduleHost.register(new CameraModule());
            moduleHost.register(new VisionModule());
            moduleHost.register(new World3dModule());
            moduleHost.register(new World2dModule());
            moduleHost.register(new Agent2dModule());
            moduleHost.register(new ThinkTimingModule());
            moduleHost.register(new MotionTimingModule());
            moduleHost.register(new LocaliserModule());
            moduleHost.register(new WalkModule());
            moduleHost.register(new CommsModule());
            moduleHost.register(new HardwareModule());
            moduleHost.register(new HistogramModule());
            moduleHost.register(new IMUModule());
            moduleHost.register(new OrientationModule());
            moduleHost.register(new OptionTreeModule());
            moduleHost.register(new GameStateModule());
            moduleHost.register(new MotionScriptModule());
            moduleHost.register(new TrajectoryModule());
            moduleHost.register(new VoiceModule());
            moduleHost.register(new StateDumpModule());
            moduleHost.register(new ConfigModule());

            $('#module-container').hide().fadeIn();
            $('#loading-indicator').fadeOut(function ()
            {
                $(this).remove();
            });

            moduleHost.load();
        };

        ControlClient.withSettings('', loadUi);

        var onerror = function ()
        {
            // Allow manual override. Useful when developing Round Table when no agent
            // is available.
            if (window.location.search.indexOf('forceload') !== -1)
            {
                loadUi();
                return;
            }

            $('#loading-indicator').find('h1').text('No Connection');
            $('#bouncer').fadeOut(function() { $(this).remove() });
        };

        ControlClient.connect(onerror);
    }
);
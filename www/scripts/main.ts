/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts"/>
/// <reference path="../libs/jquery.d.ts"/>

import control = require('control');
import constants = require('constants');
import data = require('data');
import ModuleHost = require('controls/ModuleHost');
import Setting = require('Setting');
import util = require('util');

import Agent2dModule = require('modules/Agent2dModule');
import AudioModule = require('modules/AudioModule');
import AnimatorModule = require('modules/AnimatorModule');
import CameraModule = require('modules/CameraModule');
import CommsModule = require('modules/CommsModule');
import ConfigModule = require('modules/ConfigModule');
import DrawbridgeModule = require('modules/DrawbridgeModule');
import GameStateModule = require('modules/GameStateModule');
import HardwareModule = require('modules/HardwareModule');
import HistogramModule = require('modules/HistogramModule');
import IMUModule = require('modules/IMUModule');
import LoadModule = require('modules/LoadModule');
import LocaliserModule = require('modules/LocaliserModule');
import LogModule = require('modules/LogModule');
import MotionScriptModule = require('modules/MotionScriptModule');
import MotionTimingModule = require('modules/MotionTimingModule');
import OptionTreeModule = require('modules/OptionTreeModule');
import OrientationModule = require('modules/OrientationModule');
import StateDumpModule = require('modules/StateDumpModule');
import TeamModule = require('modules/TeamModule');
import ThinkTimingModule = require('modules/ThinkTimingModule');
import TrajectoryModule = require('modules/TrajectoryModule');
import VisionModule = require('modules/VisionModule');
import VoiceModule = require('modules/VoiceModule');
import WalkModule = require('modules/WalkModule');
import World2dModule = require('modules/World2dModule');
import World3dModule = require('modules/World3dModule');

//        if (!WebGLDetector.webgl)
//            WebGLDetector.addGetWebGLMessage();

var loadUi = (settings?: Setting[]) =>
{
    if (settings)
        constants.update(settings);

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
    moduleHost.register(new AudioModule());
    moduleHost.register(new TeamModule());
    moduleHost.register(new CommsModule());
    moduleHost.register(new HardwareModule());
    moduleHost.register(new HistogramModule());
    moduleHost.register(new IMUModule());
    moduleHost.register(new OrientationModule());
    moduleHost.register(new OptionTreeModule());
    moduleHost.register(new GameStateModule());
    moduleHost.register(new MotionScriptModule());
    moduleHost.register(new LoadModule());
    moduleHost.register(new TrajectoryModule());
    moduleHost.register(new VoiceModule());
    moduleHost.register(new AnimatorModule());
    moduleHost.register(new DrawbridgeModule());
    moduleHost.register(new ConfigModule());
    moduleHost.register(new StateDumpModule());
    moduleHost.register(new LogModule());

    $('#module-container').hide().fadeIn();
    $('#loading-indicator').fadeOut(function() { $(this).remove(); });

    moduleHost.load();
};

control.withSettings('', loadUi);

var disconnectLink = <HTMLAnchorElement>document.querySelector('div#header-content a.disconnect');
disconnectLink.addEventListener('click', e =>
{
    e.preventDefault();

    if (data.isAllDisconnected())
        data.reconnectAll();
    else
        data.disconnectAll();
});

data.onConnectionChanged(() =>
{
    disconnectLink.textContent = data.isAllDisconnected() ? "reconnect" : "disconnect";
});

var onerror = () =>
{
    // Allow manual override. Useful when developing Round Table when no agent
    // is available.
    if (window.location.search.indexOf('forceload') !== -1)
    {
        loadUi();
        return;
    }

    $('#loading-indicator').find('h1').text('No Connection');
    $('#bouncer').fadeOut(function() { $(this).remove(); });
};

control.connect(onerror);

document.getElementById('logo').addEventListener('click', e => util.toggleFullScreen());

// Not sure why this copy is required, but it seems to be...
var toggleNightMode = constants.toggleNightMode;
document.querySelector("#header h1").addEventListener('click', e => { toggleNightMode(); });

document.documentElement.classList.add("initialised");

/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        'use strict';

        //noinspection UnnecessaryLocalVariableJS

        var Protocols = {
            camera: 'camera-protocol',
            timing: 'timing-protocol',
            // One per state object...
            agentFrameState: 'AgentFrame',
            alarmState: 'Alarm',
            bodyState: 'Body',
            cameraFrameState: 'CameraFrame',
            gameState: 'Game',
            hardwareState: 'Hardware'
        };

        return Protocols;
    }
);
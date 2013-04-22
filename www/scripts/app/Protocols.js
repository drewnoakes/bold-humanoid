/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        'use strict';

        var Protocols = {
            camera: 'camera-protocol',
            timing: 'timing-protocol',
            control: 'control-protocol',

            // One per StateObject...
            agentFrameState: 'AgentFrame',
            alarmState: 'Alarm',
            bodyState: 'Body',
            cameraFrameState: 'CameraFrame',
            gameState: 'Game',
            hardwareState: 'Hardware',
            optionTreeState: 'OptionTree',
            particleState: 'Particle',
            worldFrameState: 'WorldFrame'
        };

        // TODO populate this from the server somehow, but must only list those using JSON encoding
        Protocols.allStates = [
            Protocols.agentFrameState,
            Protocols.alarmState,
            Protocols.cameraFrameState,
            Protocols.bodyState,
            Protocols.gameState,
            Protocols.hardwareState,
            Protocols.optionTreeState,
            Protocols.particleState,
            Protocols.worldFrameState
        ];

        return Protocols;
    }
);
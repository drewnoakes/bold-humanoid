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
            control: 'control-protocol',

            // One per StateObject...
            agentFrameState: 'AgentFrame',
            alarmState: 'Alarm',
            ambulatorState: 'Ambulator',
            bodyState: 'Body',
            cameraFrameState: 'CameraFrame',
            debug: 'Debug',
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
            Protocols.ambulatorState,
            Protocols.cameraFrameState,
            Protocols.bodyState,
            Protocols.debug,
            Protocols.gameState,
            Protocols.hardwareState,
            Protocols.optionTreeState,
            Protocols.particleState,
            Protocols.worldFrameState
        ];

        return Protocols;
    }
);
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
            ambulatorState: 'Ambulator',
            bodyState: 'Body',
            bodyControlState: 'BodyControl',
            cameraFrameState: 'CameraFrame',
            debug: 'Debug',
            gameState: 'Game',
            hardwareState: 'Hardware',
            labelCount: 'LabelCount',
            motionTask: 'MotionTask',
            motionTiming: 'MotionTiming',
            optionTreeState: 'OptionTree',
            orientationState: 'Orientation',
            particleState: 'Particle',
            staticHardware: 'StaticHardware',
            thinkTiming: 'ThinkTiming',
            worldFrameState: 'WorldFrame'
        };

        // TODO populate this from the server somehow, but must only list those using JSON encoding
        Protocols.allStates = [
            Protocols.agentFrameState,
            Protocols.ambulatorState,
            Protocols.cameraFrameState,
            Protocols.bodyState,
            Protocols.bodyControlState,
            Protocols.debug,
            Protocols.gameState,
            Protocols.hardwareState,
            Protocols.labelCount,
            Protocols.motionTask,
            Protocols.motionTiming,
            Protocols.optionTreeState,
            Protocols.orientationState,
            Protocols.particleState,
            Protocols.staticHardware,
            Protocols.thinkTiming,
            Protocols.worldFrameState
        ];

        return Protocols;
    }
);
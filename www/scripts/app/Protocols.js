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

            // One per StateObject...
            agentFrameState: 'AgentFrame',
            alarmState: 'Alarm',
            bodyState: 'Body',
            cameraFrameState: 'CameraFrame',
            gameState: 'Game',
            hardwareState: 'Hardware',
            optionTreeState: 'OptionTree',
            worldFrameState: 'WorldFrame',

            // TODO populate this from the server somehow, but must only list those using JSON encoding
            allStates: [
                'AgentFrame',
                'Alarm',
                'CameraFrame',
                'Body',
                'Game',
                'Hardware',
                'OptionTree',
                'WorldFrame'
            ]
        };

        return Protocols;
    }
);
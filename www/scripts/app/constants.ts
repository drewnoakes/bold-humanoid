/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />
/// <reference path="../libs/three.d.ts" />

import geometry = require('util/geometry');
import math = require('util/math');
import Setting = require('Setting');
import Trackable = require('util/Trackable');

// All lengths are in metres

// These values are defaults that will be overwritten by data from the server once connected

export var fieldX = 6.0;
export var fieldY = 4.0;
export var goalX = 0.5;
export var goalY = 1.5;
export var goalZ = 0.8;
export var goalPostDiameter = 0.1;
export var goalAreaX = 0.6;
export var goalAreaY = 2.2;
export var penaltyMarkDistance = 1.8;
export var circleDiameter = 1.2;
export var lineWidth = 0.05;
export var penaltyLineLength = 0.1;
export var outerMarginMinimum = 0.7;
export var ballDiameter = 0.1;
export var footHeight = 0.0335;
export var cameraFovHorizontalDegrees = 58;
export var cameraFovVerticalDegrees = 46;
export var cameraImageWidth = 640;
export var cameraImageHeight = 480;

export var minDiagonalFieldDistance = Math.sqrt(
    Math.pow(fieldX + 2 * outerMarginMinimum, 2) +
    Math.pow(fieldY + 2 * outerMarginMinimum, 2));

export var ballRadius = ballDiameter / 2;

export enum TeamColour
{
    Unknown,
    Cyan,
    Magenta
}

export var teamColour = TeamColour.Unknown;

export var playerDiameter = 0.35;

export function update(settings: Setting[])
{
    _.each(settings, (setting: Setting) =>
    {
        switch (setting.path)
        {
            case 'world.ball-diameter':
                ballDiameter = setting.value;
                ballRadius = setting.value / 2;
                break;
            case 'world.circle-diameter': circleDiameter = setting.value; break;
            case 'world.field-size-x': fieldX = setting.value; break;
            case 'world.field-size-y': fieldY = setting.value; break;
            case 'world.goal-area-size-x': goalAreaX = setting.value; break;
            case 'world.goal-area-size-y': goalAreaY = setting.value; break;
            case 'world.goal-post-diameter': goalPostDiameter = setting.value; break;
            case 'world.goal-size-x': goalX = setting.value; break;
            case 'world.goal-size-y': goalY = setting.value; break;
            case 'world.goal-size-z': goalZ = setting.value; break;
            case 'world.line-width': lineWidth = setting.value; break;
            case 'world.outer-margin-minimum': outerMarginMinimum = setting.value; break;
            case 'world.penalty-line-length': penaltyLineLength = setting.value; break;
            case 'world.penalty-mark-distance': penaltyMarkDistance = setting.value; break;

            case 'camera.field-of-view.horizontal-degrees': cameraFovHorizontalDegrees = setting.value; break;
            case 'camera.field-of-view.vertical-degrees': cameraFovVerticalDegrees = setting.value; break;
            case 'camera.image-width': cameraImageWidth = setting.value; break;
            case 'camera.image-height': cameraImageHeight = setting.value; break;

            case 'team-colour': teamColour = <TeamColour>setting.value; break;
        }
    });
}

export var webSocketPort = 8080;

export var jointIds = {
    shoulderPitchRight: 1,
    shoulderPitchLeft: 2,
    shoulderRollRight: 3,
    shoulderRollLeft: 4,
    elbowRight: 5,
    elbowLeft: 6,
    hipYawRight: 7,
    hipYawLeft: 8,
    hipRollRight: 9,
    hipRollLeft: 10,
    hipPitchRight: 11,
    hipPitchLeft: 12,
    kneeRight: 13,
    kneeLeft: 14,
    anklePitchRight: 15,
    anklePitchLeft: 16,
    ankleRollRight: 17,
    ankleRollLeft: 18,
    headPan: 19,
    headTilt: 20
};

export var jointNiceNames = {
    1:  "Shoulder Pitch Right",
    2:  "Shoulder Pitch Left",
    3:  "Shoulder Roll Right",
    4:  "Shoulder Roll Left",
    5:  "Elbow Right",
    6:  "Elbow Left",
    7:  "Hip Yaw Right",
    8:  "Hip Yaw Left",
    9:  "Hip Roll Right",
    10: "Hip Roll Left",
    11: "Hip Pitch Right",
    12: "Hip Pitch Left",
    13: "Knee Right",
    14: "Knee Left",
    15: "Ankle Pitch Right",
    16: "Ankle Pitch Left",
    17: "Ankle Roll Right",
    18: "Ankle Roll Left",
    19: "Head Pan",
    20: "Head Tilt"
}

export var jointMotionFileNames = {
    1:  "shoulder-pitch-r",
    2:  "shoulder-pitch-l",
    3:  "shoulder-roll-r",
    4:  "shoulder-roll-l",
    5:  "elbow-r",
    6:  "elbow-l",
    7:  "hip-yaw-r",
    8:  "hip-yaw-l",
    9:  "hip-roll-r",
    10: "hip-roll-l",
    11: "hip-pitch-r",
    12: "hip-pitch-l",
    13: "knee-r",
    14: "knee-l",
    15: "ankle-pitch-r",
    16: "ankle-pitch-l",
    17: "ankle-roll-r",
    18: "ankle-roll-l",
    19: "head-pan",
    20: "head-tilt"
}

export var jointPairMotionFileNames = {
    1:  "shoulder-pitch",
    3:  "shoulder-roll",
    5:  "elbow",
    7:  "hip-yaw",
    9:  "hip-roll",
    11: "hip-pitch",
    13: "knee",
    15: "ankle-pitch",
    17: "ankle-roll"
}

export function isJointBaseOfPair(jointId: number)
{
    return jointPairMotionFileNames[jointId] != null;
}

export function getPairJointId(jointId: number)
{
    console.assert(jointId > 0 && jointId < 19);
    return jointId + (jointId % 2 == 0 ? -1 : 0);
}

export var jointIdNumbers = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20];

export var neckOffsetZ     = .026 + .0505; // OP calculated from spec
export var neckOffsetX     = .013;         // OP calculated from spec
export var shoulderOffsetX = .013;         // OP calculated from spec
export var shoulderOffsetY = .082;         // OP spec
export var shoulderOffsetZ = .026;         // OP calculated from spec
export var handOffsetX     = .058;
export var handOffsetZ     = .0159;
export var upperArmLength  = .060;         // OP spec
export var lowerArmLength  = .129;         // OP spec
export var hipOffsetY      = .072 / 2.0;   // OP spec
export var hipOffsetZ      = .096;         // OP calculated from spec
export var hipOffsetX      = .008;         // OP calculated from spec
export var thighLength     = .0930;        // OP spec
export var tibiaLength     = .0930;        // OP spec
export var footHeight      = .0335;        // OP spec
export var kneeOffsetX     = .0;           // OP

// export var dThigh = sqrt(thighLength*thighLength + kneeOffsetX*kneeOffsetX);
// export var aThigh = atan(kneeOffsetX/thighLength);
// export var dTibia = sqrt(tibiaLength*tibiaLength + kneeOffsetX*kneeOffsetX);
// export var aTibia = atan(kneeOffsetX/tibiaLength);

// export var cameraRangeVertical = (46.0/180) * Math.PI; // 46 degrees from top to bottom
// export var cameraRangeHorizontal = (60.0/180) * Math.PI; // 60 degrees from left to right

// NOTE forehead-camera geometry is not included

// TODO define the offsets/rotations/etc below in terms of measurements from constants?


export var cameraOffsetInHead = new THREE.Vector3(0, 0.0332, 0.0344);

export interface IBodyPart
{
    name: string;
    geometryPath: string;
    offset?: geometry.IPoint3;
    rotationAxis?: THREE.Euler;
    rotationOrigin?: number;
    creaseAngle?: number;
    jointId?: number;

    children?: IBodyPart[];
}

export var bodyStructure: IBodyPart = {
    name: 'torso',
    geometryPath: 'models/darwin/darwin-body.json',
    children: [
        {
            name: 'neck',
            geometryPath: 'models/darwin/darwin-neck.json',
            offset: { x: 0, y: 0, z: 0.0505 },
            rotationAxis: new THREE.Euler(0, 0, 1),
            jointId: jointIds.headPan,
            children: [
                {
                    name: 'head',
                    geometryPath: 'models/darwin/darwin-head.json',
                    creaseAngle: 1.00,
                    rotationAxis: new THREE.Euler(1, 0, 0),
                    rotationOrigin: math.degToRad(-40),
                    jointId: jointIds.headTilt,
                    children: [
                        {
                            name: 'eye-led',
                            creaseAngle: 0.52,
                            geometryPath: 'models/darwin/darwin-eye-led.json',
                            offset: { x: 0, y: 0, z: 0 }
                        },
                        {
                            name: 'forehead-led',
                            geometryPath: 'models/darwin/darwin-forehead-led.json',
                            offset: { x: 0, y: 0, z: 0 }
                        }
                    ]
                }
            ]
        },
        {
            name: 'shoulder-left',
            geometryPath: 'models/darwin/darwin-shoulder-left.json',
            offset: { x: -0.082, y: 0, z: 0 },
            rotationAxis: new THREE.Euler(-1, 0, 0),
            jointId: jointIds.shoulderPitchLeft,
            children: [
                {
                    name: 'arm-upper-left',
                    geometryPath: 'models/darwin/darwin-arm-upper-left.json',
                    offset: { x: 0, y: 0, z: -0.016 },
                    rotationAxis: new THREE.Euler(0, -1, 0),
                    rotationOrigin: -Math.PI / 4,
                    jointId: jointIds.shoulderRollLeft,
                    children: [
                        {
                            name: 'arm-lower-left',
                            geometryPath: 'models/darwin/darwin-arm-lower-left.json',
                            offset: { x: 0, y: 0.016, z: -0.06 },
                            rotationAxis: new THREE.Euler(-1, 0, 0),
                            rotationOrigin: -Math.PI / 2,
                            jointId: jointIds.elbowLeft
                        }
                    ]
                }
            ]
        },
        {
            name: 'shoulder-right',
            geometryPath: 'models/darwin/darwin-shoulder-right.json',
            offset: { x: 0.082, y: 0, z: 0 },
            rotationAxis: new THREE.Euler(1, 0, 0),
            jointId: jointIds.shoulderPitchRight,
            children: [
                {
                    name: 'arm-upper-right',
                    geometryPath: 'models/darwin/darwin-arm-upper-right.json',
                    offset: { x: 0, y: 0, z: -0.016 },
                    rotationAxis: new THREE.Euler(0, -1, 0),
                    rotationOrigin: Math.PI / 4,
                    jointId: jointIds.shoulderRollRight,
                    children: [
                        {
                            name: 'arm-lower-right',
                            geometryPath: 'models/darwin/darwin-arm-lower-right.json',
                            offset: { x: 0, y: 0.016, z: -0.06 },
                            rotationAxis: new THREE.Euler(1, 0, 0),
                            rotationOrigin: Math.PI / 2,
                            jointId: jointIds.elbowRight
                        }
                    ]
                }
            ]
        },
        {
            name: 'pelvis-left-yaw',
            geometryPath: 'models/darwin/darwin-pelvis-yaw-left.json',
            offset: { x: -0.037, y: -0.005, z: -0.1222 },
            rotationAxis: new THREE.Euler(0, 0, -1),
            jointId: jointIds.hipYawLeft,
            children: [
                {
                    name: 'pelvis-left',
                    geometryPath: 'models/darwin/darwin-pelvis-left.json',
                    offset: { x: 0, y: 0, z: 0 },
                    rotationAxis: new THREE.Euler(0, -1, 0),
                    jointId: jointIds.hipRollLeft,
                    children: [
                        {
                            name: 'leg-upper-left',
                            geometryPath: 'models/darwin/darwin-leg-upper-left.json',
                            offset: { x: 0, y: 0, z: 0 },
                            rotationAxis: new THREE.Euler(1, 0, 0),
                            jointId: jointIds.hipPitchLeft,
                            children: [
                                {
                                    name: 'leg-lower-left',
                                    geometryPath: 'models/darwin/darwin-leg-lower-left.json',
                                    offset: { x: 0, y: 0, z: -0.093 },
                                    rotationAxis: new THREE.Euler(1, 0, 0),
                                    jointId: jointIds.kneeLeft,
                                    children: [
                                        {
                                            name: 'ankle-left',
                                            geometryPath: 'models/darwin/darwin-ankle-left.json',
                                            offset: { x: 0, y: 0, z: -0.093 },
                                            rotationAxis: new THREE.Euler(-1, 0, 0),
                                            jointId: jointIds.anklePitchLeft,
                                            children: [
                                                {
                                                    name: 'foot-left',
                                                    geometryPath: 'models/darwin/darwin-foot-left.json',
                                                    offset: { x: 0, y: 0, z: 0 },
                                                    rotationAxis: new THREE.Euler(0, 1, 0),
                                                    jointId: jointIds.ankleRollLeft
                                                }
                                            ]
                                        }
                                    ]
                                }
                            ]
                        }
                    ]
                }
            ]
        },
        {
            name: 'pelvis-right-yaw',
            geometryPath: 'models/darwin/darwin-pelvis-yaw-right.json',
            offset: { x: 0.037, y: -0.005, z: -0.1222 },
            rotationAxis: new THREE.Euler(0, 0, -1),
            jointId: jointIds.hipYawRight,
            children: [
                {
                    name: 'pelvis-right',
                    geometryPath: 'models/darwin/darwin-pelvis-right.json',
                    offset: { x: 0, y: 0, z: 0 },
                    rotationAxis: new THREE.Euler(0, -1, 0),
                    jointId: jointIds.hipRollRight,
                    children: [
                        {
                            name: 'leg-upper-right',
                            geometryPath: 'models/darwin/darwin-leg-upper-right.json',
                            offset: { x: 0, y: 0, z: 0 },
                            rotationAxis: new THREE.Euler(-1, 0, 0),
                            jointId: jointIds.hipPitchRight,
                            children: [
                                {
                                    name: 'leg-lower-right',
                                    geometryPath: 'models/darwin/darwin-leg-lower-right.json',
                                    offset: { x: 0, y: 0, z: -0.093 },
                                    rotationAxis: new THREE.Euler(-1, 0, 0),
                                    jointId: jointIds.kneeRight,
                                    children: [
                                        {
                                            name: 'ankle-right',
                                            geometryPath: 'models/darwin/darwin-ankle-right.json',
                                            offset: { x: 0, y: 0, z: -0.093 },
                                            rotationAxis: new THREE.Euler(1, 0, 0),
                                            jointId: jointIds.anklePitchRight,
                                            children: [
                                                {
                                                    name: 'foot-right',
                                                    geometryPath: 'models/darwin/darwin-foot-right.json',
                                                    offset: { x: 0, y: 0, z: 0 },
                                                    rotationAxis: new THREE.Euler(0, 1, 0),
                                                    jointId: jointIds.ankleRollRight
                                                }
                                            ]
                                        }
                                    ]
                                }
                            ]
                        }
                    ]
                }
            ]
        }
    ]
};

export var protocols = {
    camera: 'camera-protocol',
    control: 'control-protocol',

    // One per StateObject...
    agentFrameState: 'AgentFrame',
    balanceState: 'Balance',
    bodyState: 'Body',
    behaviourControlState: 'BehaviourControl',
    bodyControlState: 'BodyControl',
    cameraFrameState: 'CameraFrame',
    drawingState: 'Drawing',
    debugState: 'Debug',
    gameState: 'Game',
    hardwareState: 'Hardware',
    labelCountState: 'LabelCount',
    motionTaskState: 'MotionTask',
    motionTimingState: 'MotionTiming',
    odometryState: 'Odometry',
    optionTreeState: 'OptionTree',
    teamState: 'Team',
    orientationState: 'Orientation',
    particleState: 'Particle',
    staticHardwareState: 'StaticHardware',
    stationaryMapState: 'StationaryMap',
    thinkTimingState: 'ThinkTiming',
    walkState: 'Walk',
    worldFrameState: 'WorldFrame'
};

// TODO populate this from the server somehow, but must only list those using JSON encoding

export var allStateProtocols = [
    protocols.agentFrameState,
    protocols.balanceState,
    protocols.behaviourControlState,
    protocols.bodyControlState,
    protocols.bodyState,
    protocols.cameraFrameState,
    protocols.debugState,
    protocols.drawingState,
    protocols.gameState,
    protocols.hardwareState,
    protocols.labelCountState,
    protocols.motionTaskState,
    protocols.motionTimingState,
    protocols.odometryState,
    protocols.optionTreeState,
    protocols.orientationState,
    protocols.particleState,
    protocols.staticHardwareState,
    protocols.stationaryMapState,
    protocols.teamState,
    protocols.thinkTimingState,
    protocols.walkState,
    protocols.worldFrameState
];

var getQueryStringParameterByName = name =>
{
    name = name.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
    var regexS = "[\\?&]" + name + "=([^&#]*)";
    var regex = new RegExp(regexS);
    var results = regex.exec(window.location.search);
    if (results == null)
        return null;
    else
        return decodeURIComponent(results[1].replace(/\+/g, " "));
};

var getWebSocketUrl = () =>
{
    var host = getQueryStringParameterByName("host");

    if (host == null) {
        // Use the current page's host
        var u = document.URL;
        if (u.substring(0, 4) === "http")
            u = u.substr(7);
        if (u.indexOf(":") != -1)
            u = u.substring(0, u.indexOf(":"));
        host = u.split('/')[0];
    }

    return "ws://" + host + ":" + webSocketPort;
};

export var webSocketUrl = getWebSocketUrl();

export var isNightModeActive = new Trackable<boolean>(window.localStorage["night-mode"] === "true");
isNightModeActive.track(value =>
{
    window.localStorage.setItem("night-mode", value.toString());
    if (value)
        document.documentElement.classList.add("night-mode");
    else
        document.documentElement.classList.remove("night-mode");
});

export function toggleNightMode()
{
    isNightModeActive.setValue(!isNightModeActive.getValue());
}

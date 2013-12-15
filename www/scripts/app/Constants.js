/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function()
    {
        'use strict';

        // all lengths are in metres
        var Constants = {
            fieldX: 6.0,
            fieldY: 4.0,
            goalX: 0.5,
            goalY: 1.5,
            goalZ: 0.8,
            goalPostDiameter: 0.1,
            goalAreaX: 0.6,
            goalAreaY: 2.2,
            penaltyMarkDistance: 1.8,
            circleDiameter: 1.2,
            lineWidth:0.05,
            penaltyLineLength: 0.1,
            outerMarginMinimum: 0.7,
            ballDiameter: 0.067, // according to Wikipedia
            footHeight: 0.0335,
            cameraFovHorizontalDegrees: 58,
            cameraFovVerticalDegrees: 46
        };

        Constants.ballRadius = Constants.ballDiameter / 2;

        Constants.update = function(settings)
        {
            _.each(settings, function(setting) {
                switch (setting.path)
                {
                    case 'world.ball-diameter':
                        Constants.ballDiameter = setting.value;
                        Constants.ballRadius = setting.value / 2;
                        break;
                    case 'world.circle-diameter': Constants.circleDiameter = setting.value; break;
                    case 'world.field-size-x': Constants.fieldX = setting.value; break;
                    case 'world.field-size-y': Constants.fieldY = setting.value; break;
                    case 'world.goal-area-size-x': Constants.goalAreaX = setting.value; break;
                    case 'world.goal-area-size-y': Constants.goalAreaY = setting.value; break;
                    case 'world.goal-post-diameter': Constants.goalPostDiameter = setting.value; break;
                    case 'world.goal-size-x': Constants.goalX = setting.value; break;
                    case 'world.goal-size-y': Constants.goalY = setting.value; break;
                    case 'world.goal-size-z': Constants.goalZ = setting.value; break;
                    case 'world.line-width': Constants.lineWidth = setting.value; break;
                    case 'world.outer-margin-minimum': Constants.outerMarginMinimum = setting.value; break;
                    case 'world.penalty-line-length': Constants.penaltyLineLength = setting.value; break;
                    case 'world.penalty-mark-distance': Constants.penaltyMarkDistance = setting.value; break;
                    case 'camera.field-of-view.horizontal-degrees.': Constants.cameraFovHorizontalDegrees = setting.value; break;
                    case 'camera.field-of-view.vertical-degrees.': Constants.cameraFovVerticalDegrees = setting.value; break;
                }
            });
        };

        Constants.webSocketPort = 8080;

        Constants.jointNames = {
            1: 'shoulderForwardRight',
            2: 'shoulderForwardLeft',
            3: 'shoulderOutwardRight',
            4: 'shoulderOutwardLeft',
            5: 'elbowRight',
            6: 'elbowLeft',
            7: 'legTurnRight',
            8: 'legTurnLeft',
            9: 'legOutRight',
            10: 'legOutLeft',
            11: 'legForwardRight',
            12: 'legForwardLeft',
            13: 'kneeRight',
            14: 'kneeLeft',
            15: 'footForwardRight',
            16: 'footForwardLeft',
            17: 'footOutRight',
            18: 'footOutLeft',
            19: 'headPan',
            20: 'headTilt'
        };

        var neckOffsetZ     = .026 + .0505; // OP calculated from spec
        var neckOffsetX     = .013;         // OP calculated from spec
        var shoulderOffsetX = .013;         // OP calculated from spec
        var shoulderOffsetY = .082;         // OP spec
        var shoulderOffsetZ = .026;         // OP calculated from spec
        var handOffsetX     = .058;
        var handOffsetZ     = .0159;
        var upperArmLength  = .060;         // OP spec
        var lowerArmLength  = .129;         // OP spec
        var hipOffsetY      = .072 / 2.0;   // OP spec
        var hipOffsetZ      = .096;         // OP calculated from spec
        var hipOffsetX      = .008;         // OP calculated from spec
        var thighLength     = .0930;        // OP spec
        var tibiaLength     = .0930;        // OP spec
        var footHeight      = .0335;        // OP spec
        var kneeOffsetX     = .0;           // OP

//        var dThigh = sqrt(thighLength*thighLength + kneeOffsetX*kneeOffsetX);
//        var aThigh = atan(kneeOffsetX/thighLength);
//        var dTibia = sqrt(tibiaLength*tibiaLength + kneeOffsetX*kneeOffsetX);
//        var aTibia = atan(kneeOffsetX/tibiaLength);

//        var cameraRangeVertical = (46.0/180) * Math.PI; // 46 degrees from top to bottom
//        var cameraRangeHorizontal = (60.0/180) * Math.PI; // 60 degrees from left to right

        Constants.jointIds = {
            shoulderForwardRight: 1,
            shoulderForwardLeft: 2,
            shoulderOutwardRight: 3,
            shoulderOutwardLeft: 4,
            elbowRight: 5,
            elbowLeft: 6,
            legTurnRight: 7,
            legTurnLeft: 8,
            legOutRight: 9,
            legOutLeft: 10,
            legForwardRight: 11,
            legForwardLeft: 12,
            kneeRight: 13,
            kneeLeft: 14,
            footForwardRight: 15,
            footForwardLeft: 16,
            footOutRight: 17,
            footOutLeft: 18,
            headPan: 19,
            headTilt: 20
        };

        // NOTE forehead-camera geometry is not included
        // TODO define the offsets/rotations/etc below in terms of measurements from Constants?

        Constants.cameraOffsetInHead = new THREE.Vector3(0, 0.0329074, 0.0359816);

        Constants.bodyStructure = {
            name: 'torso',
            geometryPath: 'models/darwin/darwin-body.json',
            children: [
                {
                    name: 'neck',
                    geometryPath: 'models/darwin/darwin-neck.json',
                    offset: { x: 0, y: 0, z: 0.051 },
                    rotationAxis: new THREE.Vector3(0, 0, 1),
                    jointId: Constants.jointIds.headPan,
                    children: [
                        {
                            name: 'head',
                            geometryPath: 'models/darwin/darwin-head.json',
                            creaseAngle: 0.52,
                            rotationAxis: new THREE.Vector3(1, 0, 0),
                            rotationOrigin: -Math.PI / 4,// TODO the 3D model has a rotation that should not be applied in the kinematic model
                            jointId: Constants.jointIds.headTilt,
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
                    name: 'pelvis-left-yaw',
                    geometryPath: 'models/darwin/darwin-pelvis-yaw-left.json',
                    offset: { x: -0.037, y: -0.005, z: -0.1222 },
                    rotationAxis: new THREE.Vector3(0, 0, -1),
                    jointId: Constants.jointIds.legTurnLeft,
                    children: [
                        {
                            name: 'pelvis-left',
                            geometryPath: 'models/darwin/darwin-pelvis-left.json',
                            offset: { x: 0, y: 0, z: 0 },
                            rotationAxis: new THREE.Vector3(0, -1, 0),
                            jointId: Constants.jointIds.legOutLeft,
                            children: [
                                {
                                    name: 'leg-upper-left',
                                    geometryPath: 'models/darwin/darwin-leg-upper-left.json',
                                    offset: { x: 0, y: 0, z: 0 },
                                    rotationAxis: new THREE.Vector3(1, 0, 0),
                                    jointId: Constants.jointIds.legForwardLeft,
                                    children: [
                                        {
                                            name: 'leg-lower-left',
                                            geometryPath: 'models/darwin/darwin-leg-lower-left.json',
                                            offset: { x: 0, y: 0, z: -0.093 },
                                            rotationAxis: new THREE.Vector3(1, 0, 0),
                                            jointId: Constants.jointIds.kneeLeft,
                                            children: [
                                                {
                                                    name: 'ankle-left',
                                                    geometryPath: 'models/darwin/darwin-ankle-left.json',
                                                    offset: { x: 0, y: 0, z: -0.093 },
                                                    rotationAxis: new THREE.Vector3(-1, 0, 0),
                                                    jointId: Constants.jointIds.footForwardLeft,
                                                    children: [
                                                        {
                                                            name: 'foot-left',
                                                            geometryPath: 'models/darwin/darwin-foot-left.json',
                                                            offset: { x: 0, y: 0, z: 0 },
                                                            rotationAxis: new THREE.Vector3(0, 1, 0),
                                                            jointId: Constants.jointIds.footOutLeft
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
                    rotationAxis: new THREE.Vector3(0, 0, -1),
                    jointId: Constants.jointIds.legTurnRight,
                    children: [
                        {
                            name: 'pelvis-right',
                            geometryPath: 'models/darwin/darwin-pelvis-right.json',
                            offset: { x: 0, y: 0, z: 0 },
                            rotationAxis: new THREE.Vector3(0, -1, 0),
                            jointId: Constants.jointIds.legOutRight,
                            children: [
                                {
                                    name: 'leg-upper-right',
                                    geometryPath: 'models/darwin/darwin-leg-upper-right.json',
                                    offset: { x: 0, y: 0, z: 0 },
                                    rotationAxis: new THREE.Vector3(-1, 0, 0),
                                    jointId: Constants.jointIds.legForwardRight,
                                    children: [
                                        {
                                            name: 'leg-lower-right',
                                            geometryPath: 'models/darwin/darwin-leg-lower-right.json',
                                            offset: { x: 0, y: 0, z: -0.093 },
                                            rotationAxis: new THREE.Vector3(-1, 0, 0),
                                            jointId: Constants.jointIds.kneeRight,
                                            children: [
                                                {
                                                    name: 'ankle-right',
                                                    geometryPath: 'models/darwin/darwin-ankle-right.json',
                                                    offset: { x: 0, y: 0, z: -0.093 },
                                                    rotationAxis: new THREE.Vector3(1, 0, 0),
                                                    jointId: Constants.jointIds.footForwardRight,
                                                    children: [
                                                        {
                                                            name: 'foot-right',
                                                            geometryPath: 'models/darwin/darwin-foot-right.json',
                                                            offset: { x: 0, y: 0, z: 0 },
                                                            rotationAxis: new THREE.Vector3(0, 1, 0),
                                                            jointId: Constants.jointIds.footOutRight
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
                    name: 'shoulder-left',
                    geometryPath: 'models/darwin/darwin-shoulder-left.json',
                    offset: { x: -0.082, y: 0, z: 0 },
                    rotationAxis: new THREE.Vector3(-1, 0, 0),
                    jointId: Constants.jointIds.shoulderForwardLeft,
                    children: [
                        {
                            name: 'arm-upper-left',
                            geometryPath: 'models/darwin/darwin-arm-upper-left.json',
                            offset: { x: 0, y: 0, z: -0.016 },
                            rotationAxis: new THREE.Vector3(0, -1, 0),
                            rotationOrigin: -Math.PI / 4,
                            jointId: Constants.jointIds.shoulderOutwardLeft,
                            children: [
                                {
                                    name: 'arm-lower-left',
                                    geometryPath: 'models/darwin/darwin-arm-lower-left.json',
                                    offset: { x: 0, y: 0.016, z: -0.06 },
                                    rotationAxis: new THREE.Vector3(-1, 0, 0),
                                    rotationOrigin: -Math.PI / 2,
                                    jointId: Constants.jointIds.elbowLeft
                                }
                            ]
                        }
                    ]
                },
                {
                    name: 'shoulder-right',
                    geometryPath: 'models/darwin/darwin-shoulder-right.json',
                    offset: { x: 0.082, y: 0, z: 0 },
                    rotationAxis: new THREE.Vector3(1, 0, 0),
                    jointId: Constants.jointIds.shoulderForwardRight,
                    children: [
                        {
                            name: 'arm-upper-right',
                            geometryPath: 'models/darwin/darwin-arm-upper-right.json',
                            offset: { x: 0, y: 0, z: -0.016 },
                            rotationAxis: new THREE.Vector3(0, -1, 0),
                            rotationOrigin: Math.PI / 4,
                            jointId: Constants.jointIds.shoulderOutwardRight,
                            children: [
                                {
                                    name: 'arm-lower-right',
                                    geometryPath: 'models/darwin/darwin-arm-lower-right.json',
                                    offset: { x: 0, y: 0.016, z: -0.06 },
                                    rotationAxis: new THREE.Vector3(1, 0, 0),
                                    rotationOrigin: Math.PI / 2,
                                    jointId: Constants.jointIds.elbowRight
                                }
                            ]
                        }
                    ]
                }
            ]
        };

        return Constants;
    }
);

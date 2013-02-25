/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function()
    {
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
            ballDiameter: 0.067 // according to Wikipedia
        };

        var jointNames = {
            1: 'shoulderForwardRight',
            2: 'shoulderForwardLeft',
            3: 'shoulderOutwardRight',
            4: 'shoulderOutwardLeft',
            5: 'elbowRight',
            6: 'elbowLeft',
            7: 'legTurnRight',
            8: 'legTurnLeft',
            9: 'legForwardRight',
            10: 'legForwardLeft',
            11: 'legOutRight',
            12: 'legOutLeft',
            13: 'kneeRight',
            14: 'kneeLeft',
            15: 'footOutRight',
            16: 'footOutLeft',
            17: 'footForwardRight',
            18: 'footForwardLeft',
            19: 'headPan',
            20: 'headTilt'
        };

        Constants.jointIds = {
            shoulderForwardRight: 1,
            shoulderForwardLeft: 2,
            shoulderOutwardRight: 3,
            shoulderOutwardLeft: 4,
            elbowRight: 5,
            elbowLeft: 6,
            legTurnRight: 7,
            legTurnLeft: 8,
            legForwardRight: 9,
            legForwardLeft: 10,
            legOutRight: 11,
            legOutLeft: 12,
            kneeRight: 13,
            kneeLeft: 14,
            footOutRight: 15,
            footOutLeft: 16,
            footForwardRight: 17,
            footForwardLeft: 18,
            headPan: 19,
            headTilt: 20
        };

        // TODO add forehead-camera geometry
        // TODO define the offsets/rotations/etc below in terms of Constants

        Constants.bodyStructure = {
            name: 'body',
            geometryPath: 'models/darwin/darwin-body.json',
            children: [
                {
                    name: 'neck',
                    geometryPath: 'models/darwin/darwin-neck.json',
                    offset: { x: 0, y: 0.051 },
                    rotationAxis: 'y',
                    jointId: Constants.jointIds.headPan,
                    children: [
                        {
                            name: 'head',
                            geometryPath: 'models/darwin/darwin-head.json',
                            creaseAngle: 0.52,
//                            offset: { x: 0, y: 0.0 },
                            rotationAxis: 'x', // TODO should be -1, 0, 0
                            jointId: Constants.jointIds.headTilt,
                            children: [
                                {
                                    name: 'eye-led',
                                    creaseAngle: 0.52,
                                    geometryPath: 'models/darwin/darwin-eye-led.json',
                                    offset: { x: 0, y: 0 }
                                },
                                {
                                    name: 'forehead-led',
                                    geometryPath: 'models/darwin/darwin-forehead-led.json',
                                    offset: { x: 0, y: 0 }
                                }
                            ]
                        }
                    ]
                },
                {
                    name: 'pelvis-left-yaw',
                    geometryPath: 'models/darwin/darwin-pelvis-yaw-left.json',
                    offset: { x: 0.037, y: -0.1222, z: -0.005 },
                    rotationAxis: 'y', // TODO should be 0, -1, 0
                    jointId: Constants.jointIds.legTurnLeft,
                    children: [
                        {
                            name: 'pelvis-left',
                            geometryPath: 'models/darwin/darwin-pelvis-left.json',
                            offset: { x: 0, y: 0, z: 0 },
                            rotationAxis: 'z', // TODO should be 0, 0, -1
                            jointId: Constants.jointIds.legOutLeft,
                            children: [
                                {
                                    name: 'leg-upper-left',
                                    geometryPath: 'models/darwin/darwin-leg-upper-left.json',
                                    offset: { x: 0, y: 0, z: 0 },
                                    rotationAxis: 'x', // TODO should be -1, 0, 0
                                    jointId: Constants.jointIds.legForwardLeft,
                                    children: [
                                        {
                                            name: 'leg-lower-left',
                                            geometryPath: 'models/darwin/darwin-leg-lower-left.json',
                                            offset: { x: 0, y: -0.093, z: 0 },
                                            rotationAxis: 'x', // TODO should be -1, 0, 0
                                            jointId: Constants.jointIds.kneeLeft,
                                            children: [
                                                {
                                                    name: 'ankle-left',
                                                    geometryPath: 'models/darwin/darwin-ankle-left.json',
                                                    offset: { x: 0, y: -0.093, z: 0 },
                                                    rotationAxis: 'x', // TODO should be 1, 0, 0
                                                    jointId: Constants.jointIds.footForwardLeft,
                                                    children: [
                                                        {
                                                            name: 'foot-left',
                                                            geometryPath: 'models/darwin/darwin-foot-left.json',
                                                            offset: { x: 0, y: 0, z: 0 },
                                                            rotationAxis: 'z', // TODO should be 0, 0, 1
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
                    offset: { x: -0.037, y: -0.1222, z: -0.005 },
                    rotationAxis: 'y', // TODO should be 0, -1, 0
                    jointId: Constants.jointIds.legTurnRight,
                    children: [
                        {
                            name: 'pelvis-right',
                            geometryPath: 'models/darwin/darwin-pelvis-right.json',
                            offset: { x: 0, y: 0, z: 0 },
                            rotationAxis: 'z', // TODO should be 0, 0, -1
                            jointId: Constants.jointIds.legOutRight,
                            children: [
                                {
                                    name: 'leg-upper-right',
                                    geometryPath: 'models/darwin/darwin-leg-upper-right.json',
                                    offset: { x: 0, y: 0, z: 0 },
                                    rotationAxis: 'x', // TODO should be 1, 0, 0
                                    jointId: Constants.jointIds.legForwardRight,
                                    children: [
                                        {
                                            name: 'leg-lower-right',
                                            geometryPath: 'models/darwin/darwin-leg-lower-right.json',
                                            offset: { x: 0, y: -0.093, z: 0 },
                                            rotationAxis: 'x', // TODO should be 1, 0, 0
                                            jointId: Constants.jointIds.kneeRight,
                                            children: [
                                                {
                                                    name: 'ankle-right',
                                                    geometryPath: 'models/darwin/darwin-ankle-right.json',
                                                    offset: { x: 0, y: -0.093, z: 0 },
                                                    rotationAxis: 'x', // TODO should be -1, 0, 0
                                                    jointId: Constants.jointIds.footForwardRight,
                                                    children: [
                                                        {
                                                            name: 'foot-right',
                                                            geometryPath: 'models/darwin/darwin-foot-right.json',
                                                            offset: { x: 0, y: 0, z: 0 },
                                                            rotationAxis: 'z', // TODO should be 0, 0, 1
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
                    offset: { x: 0.082, y: 0, z: 0 },
                    rotationAxis: 'x', // 1 0 0
                    jointId: Constants.jointIds.shoulderForwardLeft,
                    children: [
                        {
                            name: 'arm-upper-left',
                            geometryPath: 'models/darwin/darwin-arm-upper-left.json',
                            offset: { x: 0, y: -0.016, z: 0 },
                            rotationAxis: 'z', // TODO should be 0 0 -1 -0.7854
                            jointId: Constants.jointIds.shoulderOutwardLeft,
                            children: [
                                {
                                    name: 'arm-lower-left',
                                    geometryPath: 'models/darwin/darwin-arm-lower-left.json',
                                    offset: { x: 0, y: -0.06, z: 0.016 },
                                    rotationAxis: 'x', // TODO should be 1 0 0 -1.5708
                                    jointId: Constants.jointIds.elbowLeft
                                }
                            ]
                        }
                    ]
                },
                {
                    name: 'shoulder-right',
                    geometryPath: 'models/darwin/darwin-shoulder-right.json',
                    offset: { x: -0.082, y: 0, z: 0 },
                    rotationAxis: 'x', // -1 0 0
                    jointId: Constants.jointIds.shoulderForwardRight,
                    children: [
                        {
                            name: 'arm-upper-right',
                            geometryPath: 'models/darwin/darwin-arm-upper-right.json',
                            offset: { x: 0, y: -0.016, z: 0 },
                            rotationAxis: 'z', // TODO should be 0 0 -1 0.7854
                            jointId: Constants.jointIds.shoulderOutwardRight,
                            children: [
                                {
                                    name: 'arm-lower-right',
                                    geometryPath: 'models/darwin/darwin-arm-lower-right.json',
                                    offset: { x: 0, y: -0.06, z: 0.016 },
                                    rotationAxis: 'x', // TODO should be -1 0 0 1.5708
                                    jointId: Constants.jointIds.elbowRight
                                }
                            ]
                        }
                    ]
                }
            ]
        };

        return Constants;

        // TODO allow rotationAxis to be a vector rather than an axis, or else additionally specify the direction

        /*

         double neckOffsetZ     = .shoulderOffsetZ + .0505; // OP calculated from spec
         double neckOffsetX     = .013;         // OP calculated from spec
         double shoulderOffsetX = .013;         // OP calculated from spec
         double shoulderOffsetY = .082;         // OP spec
         double shoulderOffsetZ = .026;         // OP calculated from spec
         double handOffsetX     = .058;
         double handOffsetZ     = .0159;
         double upperArmLength  = .060;         // OP spec
         double lowerArmLength  = .129;         // OP spec
         double hipOffsetY      = .072 / 2.0;   // OP spec
         double hipOffsetZ      = .096;         // OP calculated from spec
         double hipOffsetX      = .008;         // OP calculated from spec
         double thighLength     = .0930;        // OP spec
         double tibiaLength     = .0930;        // OP spec
         double footHeight      = .0335;        // OP spec
         double kneeOffsetX     = .0;           // OP

         double dThigh = sqrt(thighLength*thighLength + kneeOffsetX*kneeOffsetX);
         double aThigh = atan(kneeOffsetX/thighLength);
         double dTibia = sqrt(tibiaLength*tibiaLength + kneeOffsetX*kneeOffsetX);
         double aTibia = atan(kneeOffsetX/tibiaLength);

         double cameraRangeVertical = (46.0/180) * Math.PI; // 46 degrees from top to bottom
         double cameraRangeHorizontal = (60.0/180) * Math.PI; // 60 degrees from left to right

         */
    }
);
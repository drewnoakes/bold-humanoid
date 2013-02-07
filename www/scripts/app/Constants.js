define(
    [
    ],
    function()
    {
        //noinspection UnnecessaryLocalVariableJS

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

        return Constants;

        // TODO allow rotationAxis to be a vector rather than an axis

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

         var jointById = {
         1: shoulderForwardRight,
         2: shoulderForwardLeft,
         3: shoulderOutwardRight,
         4: shoulderOutwardLeft,
         5: elbowRight,
         6: elbowLeft,
         7: legTurnRight,
         8: legTurnLeft,
         9: legForwardRight,
         10: legForwardLeft,
         11: legOutRight,
         12: legOutLeft,
         13: kneeRight,
         14: kneeLeft,
         15: footOutRight,
         16: footOutLeft,
         17: footForwardRight,
         18: footForwardLeft,
         19: headPan,
         20: headTilt
         };
         */
    }
);
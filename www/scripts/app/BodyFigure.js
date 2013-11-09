/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'Constants'
    ],
    function (Constants)
    {
        'use strict';

        var bodyTemplate = Handlebars.compile($('#body-figure-template').html());

        var BodyFigure = function ()
        {
            var shoulderOffset = 60,
                elbowOffset = 65,
                hipOffset = 40,
                kneeOffset = 45,
                footOffset = 50;

            // TODO rename Constants.jointIds to use consistent pitch/roll/yaw naming

            var bodyData = {
                joints: [
                    { id: Constants.jointIds.headTilt, name: 'Head Tilt', row: 0, group: 0, xOffset: 0 },
                    { id: Constants.jointIds.headPan,  name: 'Head Pan', row: 1, group: 0, xOffset: 0 },

                    { id: Constants.jointIds.shoulderForwardLeft,  name: 'Left Shoulder Pitch', row: 2, group: 1, xOffset: shoulderOffset },
                    { id: Constants.jointIds.shoulderForwardRight, name: 'Right Shoulder Pitch', row: 2, group: 1, xOffset: -shoulderOffset },
                    { id: Constants.jointIds.shoulderOutwardLeft,  name: 'Left Shoulder Roll', row: 3, group: 1, xOffset: shoulderOffset },
                    { id: Constants.jointIds.shoulderOutwardRight, name: 'Right Shoulder Roll', row: 3, group: 1, xOffset: -shoulderOffset },

                    { id: Constants.jointIds.elbowLeft,  name: 'Left Elbow',  row: 4, group: 2, xOffset: elbowOffset },
                    { id: Constants.jointIds.elbowRight, name: 'Right Elbow', row: 4, group: 2, xOffset: -elbowOffset },

                    { id: Constants.jointIds.legTurnLeft,     name: 'Left Hip Yaw', row: 5, group: 3, xOffset: hipOffset },
                    { id: Constants.jointIds.legTurnRight,    name: 'Right Hip Yaw', row: 5, group: 3, xOffset: -hipOffset },
                    { id: Constants.jointIds.legOutLeft,      name: 'Left Hip Roll', row: 6, group: 3, xOffset: hipOffset },
                    { id: Constants.jointIds.legOutRight,     name: 'Right Hip Roll', row: 6, group: 3, xOffset: -hipOffset },
                    { id: Constants.jointIds.legForwardLeft,  name: 'Left Hip Pitch', row: 7, group: 3, xOffset: hipOffset },
                    { id: Constants.jointIds.legForwardRight, name: 'Right Hip Pitch', row: 7, group: 3, xOffset: -hipOffset },

                    { id: Constants.jointIds.kneeLeft,  name: 'Left Knee', row: 8, group: 4, xOffset: kneeOffset },
                    { id: Constants.jointIds.kneeRight, name: 'Right Knee', row: 8, group: 4, xOffset: -kneeOffset },

                    { id: Constants.jointIds.footForwardLeft,  name: 'Left Ankle Pitch', row: 9, group: 5, xOffset: footOffset },
                    { id: Constants.jointIds.footForwardRight, name: 'Right Ankle Pitch', row: 9, group: 5, xOffset: -footOffset },
                    { id: Constants.jointIds.footOutLeft,      name: 'Left Ankle Roll', row: 10, group: 5, xOffset: footOffset },
                    { id: Constants.jointIds.footOutRight,     name: 'Right Ankle Roll', row: 10, group: 5, xOffset: -footOffset }
                ],
                containerHeight: 0,
                containerWidth: 0
            };

            // layout constants
            var blockHeight = 20,
                blockWidth = 70,
                blockSpacing = 25, // from top-to-top, not bottom-to-top
                groupSpacing = 6,
                center = 70;

            _.each(bodyData.joints, function (joint)
            {
                joint.y = groupSpacing * joint.group + blockSpacing * joint.row;
                joint.x = center + joint.xOffset;
                joint.height = blockHeight;
                joint.width = blockWidth;

                bodyData.containerWidth = Math.max(bodyData.containerWidth, joint.x + joint.width);
                bodyData.containerHeight = Math.max(bodyData.containerHeight, joint.y + joint.height);
            });

            var bodyHtml = bodyTemplate(bodyData);
            var doc = $('<div></div>').html(bodyHtml).get(0);
            this.element = doc.children[0];

            this.jointElementById = [undefined];

            for (var i = 1; i <= 20; i++) {
                var jointBlock = this.element.querySelector("div.joint[data-joint-id='" + i + "']");
                this.jointElementById.push(jointBlock);
            }
        };

        BodyFigure.prototype.getJointElement = function(jointId)
        {
            return this.jointElementById[jointId];
        };

        return BodyFigure;
    }
);
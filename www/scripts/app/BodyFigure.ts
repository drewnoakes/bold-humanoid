/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />

import constants = require('constants');
import DOMTemplate = require('DOMTemplate');
import util = require('util');

var bodyTemplate = new DOMTemplate('body-figure-template');

interface JointDetail
{
    id: number;
    name: string;
    row: number;
    group: number;
    xOffset: number;
    x?: number;
    y?: number;
    height?: number;
    width?: number;
}

class BodyFigure
{
    public element: HTMLDivElement;

    private jointElementById: HTMLDivElement[];

    public hoverJointId: util.Trackable<number> = new util.Trackable<number>();
    public selectedJointIds: util.Trackable<number[]> = new util.Trackable<number[]>([]);

    constructor(options: any)
    {
        var shoulderOffset = 60,
            elbowOffset = 65,
            hipOffset = 40,
            kneeOffset = 45,
            footOffset = 50;

        options = _.extend<any,any,any,any,any>({}, {hasHover: false, hasSelection: false}, options);

        // TODO rename constants.jointIds to use consistent pitch/roll/yaw naming

        var bodyData = {
            joints: <JointDetail[]>[
                { id: constants.jointIds.headTilt, name: 'Head Tilt', row: 0, group: 0, xOffset: 0 },
                { id: constants.jointIds.headPan,  name: 'Head Pan', row: 1, group: 0, xOffset: 0 },

                { id: constants.jointIds.shoulderForwardLeft,  name: 'Left Shoulder Pitch', row: 2, group: 1, xOffset: shoulderOffset },
                { id: constants.jointIds.shoulderForwardRight, name: 'Right Shoulder Pitch', row: 2, group: 1, xOffset: -shoulderOffset },
                { id: constants.jointIds.shoulderOutwardLeft,  name: 'Left Shoulder Roll', row: 3, group: 1, xOffset: shoulderOffset },
                { id: constants.jointIds.shoulderOutwardRight, name: 'Right Shoulder Roll', row: 3, group: 1, xOffset: -shoulderOffset },

                { id: constants.jointIds.elbowLeft,  name: 'Left Elbow',  row: 4, group: 2, xOffset: elbowOffset },
                { id: constants.jointIds.elbowRight, name: 'Right Elbow', row: 4, group: 2, xOffset: -elbowOffset },

                { id: constants.jointIds.legTurnLeft,     name: 'Left Hip Yaw', row: 5, group: 3, xOffset: hipOffset },
                { id: constants.jointIds.legTurnRight,    name: 'Right Hip Yaw', row: 5, group: 3, xOffset: -hipOffset },
                { id: constants.jointIds.legOutLeft,      name: 'Left Hip Roll', row: 6, group: 3, xOffset: hipOffset },
                { id: constants.jointIds.legOutRight,     name: 'Right Hip Roll', row: 6, group: 3, xOffset: -hipOffset },
                { id: constants.jointIds.legForwardLeft,  name: 'Left Hip Pitch', row: 7, group: 3, xOffset: hipOffset },
                { id: constants.jointIds.legForwardRight, name: 'Right Hip Pitch', row: 7, group: 3, xOffset: -hipOffset },

                { id: constants.jointIds.kneeLeft,  name: 'Left Knee', row: 8, group: 4, xOffset: kneeOffset },
                { id: constants.jointIds.kneeRight, name: 'Right Knee', row: 8, group: 4, xOffset: -kneeOffset },

                { id: constants.jointIds.footForwardLeft,  name: 'Left Ankle Pitch', row: 9, group: 5, xOffset: footOffset },
                { id: constants.jointIds.footForwardRight, name: 'Right Ankle Pitch', row: 9, group: 5, xOffset: -footOffset },
                { id: constants.jointIds.footOutLeft,      name: 'Left Ankle Roll', row: 10, group: 5, xOffset: footOffset },
                { id: constants.jointIds.footOutRight,     name: 'Right Ankle Roll', row: 10, group: 5, xOffset: -footOffset }
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

        this.element = <HTMLDivElement>bodyTemplate.create(bodyData);

        this.jointElementById = [undefined]; // pad index zero

        this.visitJoints((jointId, jointDiv) =>
        {
            if (options.hasHover) {
                jointDiv.addEventListener('mouseenter', () => this.hoverJointId.setValue(jointId));
                jointDiv.addEventListener('mouseleave', () => this.hoverJointId.setValue(undefined));
            }

            if (options.hasSelection) {
                jointDiv.addEventListener('click', () =>
                {
                    if (jointDiv.classList.contains('selected')) {
                        jointDiv.classList.remove('selected');
                        this.selectedJointIds.setValue(_.filter(this.selectedJointIds.getValue(), d => d !== jointId));
                    } else {
                        jointDiv.classList.add('selected');
                        this.selectedJointIds.getValue().push(jointId);
                        this.selectedJointIds.triggerChange();
                    }
                });
            }

            this.jointElementById.push(jointDiv);

        });
    }

    public getJointElement(jointId)
    {
        return this.jointElementById[jointId];
    }

    public visitJoints(callback: (jointId:number, jointDiv:HTMLDivElement) => void)
    {
        _.each(_.range(1, 21), function(jointId)
        {
            var jointDiv = <HTMLDivElement>this.element.querySelector("div.joint[data-joint-id='" + jointId + "']");
            callback(jointId, jointDiv);
        }.bind(this));
    }
}

export = BodyFigure;

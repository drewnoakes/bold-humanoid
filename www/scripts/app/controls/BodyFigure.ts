/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import constants = require('constants');
import DOMTemplate = require('DOMTemplate');
import Trackable = require('util/Trackable');

var bodyTemplate = new DOMTemplate('body-figure-template');

interface JointDetail
{
    id: number;
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

    public hoverJointId: Trackable<number> = new Trackable<number>();
    public selectedJointIds: Trackable<number[]> = new Trackable<number[]>([]);

    constructor(options?: any)
    {
        var shoulderOffset = 60,
            elbowOffset = 65,
            hipOffset = 40,
            kneeOffset = 45,
            footOffset = 50;

        options = _.extend<any,any,any,any,any>({}, {hasHover: false, hasSelection: false}, options);

        var bodyData = {
            joints: <JointDetail[]>[
                { id: constants.jointIds.headTilt,           row: 0,  group: 0, xOffset: 0 },
                { id: constants.jointIds.headPan,            row: 1,  group: 0, xOffset: 0 },

                { id: constants.jointIds.shoulderPitchLeft,  row: 2,  group: 1, xOffset: shoulderOffset },
                { id: constants.jointIds.shoulderPitchRight, row: 2,  group: 1, xOffset: -shoulderOffset },
                { id: constants.jointIds.shoulderRollLeft,   row: 3,  group: 1, xOffset: shoulderOffset },
                { id: constants.jointIds.shoulderRollRight,  row: 3,  group: 1, xOffset: -shoulderOffset },

                { id: constants.jointIds.elbowLeft,          row: 4,  group: 2, xOffset: elbowOffset },
                { id: constants.jointIds.elbowRight,         row: 4,  group: 2, xOffset: -elbowOffset },

                { id: constants.jointIds.hipYawLeft,         row: 5,  group: 3, xOffset: hipOffset },
                { id: constants.jointIds.hipYawRight,        row: 5,  group: 3, xOffset: -hipOffset },
                { id: constants.jointIds.hipRollLeft,        row: 6,  group: 3, xOffset: hipOffset },
                { id: constants.jointIds.hipRollRight,       row: 6,  group: 3, xOffset: -hipOffset },
                { id: constants.jointIds.hipPitchLeft,       row: 7,  group: 3, xOffset: hipOffset },
                { id: constants.jointIds.hipPitchRight,      row: 7,  group: 3, xOffset: -hipOffset },

                { id: constants.jointIds.kneeLeft,           row: 8,  group: 4, xOffset: kneeOffset },
                { id: constants.jointIds.kneeRight,          row: 8,  group: 4, xOffset: -kneeOffset },

                { id: constants.jointIds.anklePitchLeft,     row: 9,  group: 5, xOffset: footOffset },
                { id: constants.jointIds.anklePitchRight,    row: 9,  group: 5, xOffset: -footOffset },
                { id: constants.jointIds.ankleRollLeft,      row: 10, group: 5, xOffset: footOffset },
                { id: constants.jointIds.ankleRollRight,     row: 10, group: 5, xOffset: -footOffset }
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

        _.each(bodyData.joints, joint =>
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
                        this.selectedJointIds.setValue(_.filter<number>(this.selectedJointIds.getValue(), d => d !== jointId));
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

    public getJointElement(jointId: number)
    {
        return this.jointElementById[jointId];
    }

    public visitJoints(callback: (jointId:number, jointDiv:HTMLDivElement) => void)
    {
        _.each<number>(_.range(1, 21), jointId =>
        {
            var jointDiv = <HTMLDivElement>this.element.querySelector("div.joint[data-joint-id='" + jointId + "']");
            callback(jointId, jointDiv);
        });
    }
}

export = BodyFigure;

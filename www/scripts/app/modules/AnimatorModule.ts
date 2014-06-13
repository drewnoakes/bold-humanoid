/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/d3.d.ts" />

import control = require('control');
import constants = require('constants');
import DOMTemplate = require('DOMTemplate');
import geometry = require('util/geometry');
import Module = require('Module');
import scripts = require('scripts');
import Trackable = require('util/Trackable');

function getJointValue(values: scripts.JointValues, jointId: number, defaultValue?: number): number
{
    var v = values[constants.jointMotionFileNames[jointId]];

    // If not found (and not a head joint) then look for a value specified as half a pair
    if (v == null && jointId < 19)
    {
        v = values[constants.jointPairMotionFileNames[constants.getPairJointId(jointId)]];

        // TODO need to mirror value here if from a pair and it's the left side joint!!!

        if (v == null && defaultValue != null)
            return defaultValue;
    }

    console.assert(v != null);

    return  v
}

function buildValueArray(values: scripts.JointValues, defaultValue?: number): number[]
{
    return _.map(constants.jointIdNumbers, jointId => getJointValue(values, jointId, defaultValue));
}

function toViewModel(script: scripts.MotionScript): IScriptViewModel
{
    return {
        name: script.name,
        controlsHead: script.controlsHead,
        controlsArms: script.controlsArms,
        controlsLegs: script.controlsLegs,
        stages: _.map(script.stages, (stage: scripts.Stage) => ({
            repeat: stage.repeat || 0,
            speed: stage.speed || 0,
            pGains: buildValueArray(stage.pGains || {}, 32),
            keyFrames: _.map(stage.keyFrames, (keyFrame: scripts.KeyFrame) => ({
                pauseCycles: keyFrame.pauseCycles || 0,
                moveCycles: keyFrame.moveCycles,
                values: buildValueArray(keyFrame.values)
            }))
        }))
    }
}

interface IKeyFrameViewModel
{
    pauseCycles: number;
    moveCycles: number;
    values: number[];
}

interface IStageViewModel
{
    repeat: number;
    speed: number;
    pGains: number[];
    keyFrames: IKeyFrameViewModel[];
}

interface IScriptViewModel
{
    name: string;
    controlsHead: boolean;
    controlsArms: boolean;
    controlsLegs: boolean;
    stages: IStageViewModel[];
}

var animatorTemplate = new DOMTemplate('animator-template');

class AnimatorModule extends Module
{
    constructor()
    {
        super('animator', 'animator', {fullScreen: true});
    }

    public load()
    {
        var content = <HTMLElement>animatorTemplate.create();
        this.element.appendChild(content);

        /*
          <h2 class="script-name"></h2>
          <div class="timeline-container">
            <ul class="joint-names"></ul>
            <ul class="stages"></ul>
          </div>
        */

        // Populate the joint names once
        d3.select(this.element)
            .select("ul.joint-names")
            .selectAll("li")
            .data(constants.jointIdNumbers)
            .enter()
            .append("li")
            .text(jointId => constants.jointNiceNames[jointId]);

        // Function to bind the view model to the stages element
        var update = (script: IScriptViewModel) =>
        {
            // Set the header directly
            d3.select(this.element).select("h2.script-name").text(script.name);

            /*
              <ul class="stages">
                <li>
                  <ul class="gains">
                    <li>32</li>
                    ...
                  </ul>
                  <ul class="key-frames">
                    <li>
                      <ul class="values">
                        <li>1670</li>
                        ...
                      </ul>
                      <div class="move-cycles">25</div>
                      <div class="pause-cycles">0</div>
                    </li>
                  </ul>
                </li>
              </ul>
            */

            //////// STAGES

            var stages = d3.select(this.element)
                .select("ul.stages")
                .selectAll("li")
                .data(script.stages);

            var enteredStages = stages.enter()
                .append("li")
                .classed("stage", true);
            enteredStages.append("ul").classed("gains", true);
            enteredStages.append("ul").classed("key-frames", true);

            stages.exit().remove();

            //////// GAINS

            var gains = stages.select("ul.gains")
                .selectAll("li")
                .data(stage => stage.pGains);

            gains.enter()
                .append("li");

            gains.text(d => d.toString());

            gains.exit().remove();

            //////// KEY FRAMES

            var keyFrames = stages.select("ul.key-frames")
                .selectAll("li")
                .data((stage: IStageViewModel) => stage.keyFrames);

            var enteredKeyFrames = keyFrames.enter()
                .append("li")
                .classed("key-frame", true);

            enteredKeyFrames.append("ul").classed("values", true);
            enteredKeyFrames.append("div").classed("move-cycles", true);
            enteredKeyFrames.append("div").classed("pause-cycles", true);

            keyFrames.exit().remove();

            var values = keyFrames.select("ul.values")
                .selectAll("li")
                .data((keyFrame: IKeyFrameViewModel) => keyFrame.values)
                .enter()
                .append("li").text(d => d);
        };

        update(toViewModel(scripts.allMotionScripts[2]));

        this.intervalHandler = window.setInterval(() => update(toViewModel(scripts.allMotionScripts[Math.floor(Math.random() * scripts.allMotionScripts.length)])), 5000);
    }

    private intervalHandler: number;

    public unload()
    {
        window.clearInterval(this.intervalHandler);
    }
}

export = AnimatorModule;

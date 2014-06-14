/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/d3.d.ts" />

import control = require('control');
import constants = require('constants');
import DOMTemplate = require('DOMTemplate');
import geometry = require('util/geometry');
import math = require('util/math');
import Module = require('Module');
import scripts = require('scripts');
import Trackable = require('util/Trackable');

var MIN_VALUE: number    = 0x0000;
var CENTER_VALUE: number = 0x0800; // 2048
var MAX_VALUE: number    = 0x0FFF; // 4095

var RATIO_VALUE2DEGS: number =  360.0 / 4096.0;
var RATIO_DEGS2VALUE: number = 4096.0 / 360.0;

var RATIO_VALUE2RADS: number = (2*Math.PI) / 4096.0;
var RATIO_RADS2VALUE: number = 4096.0 / (2*Math.PI);

function getMirrorValue(value: number) { return clampValue(MAX_VALUE + 1 - value); }
function getMirrorAngle(angle: number) { return -angle; }
function degs2Value(angle: number) { return math.clamp(Math.round(angle*RATIO_DEGS2VALUE), -CENTER_VALUE, MAX_VALUE - CENTER_VALUE) + CENTER_VALUE; }
function value2Degs(value: number) { return (value - CENTER_VALUE)*RATIO_VALUE2DEGS; }
function rads2Value(angle: number) { return math.clamp(Math.round(angle*RATIO_RADS2VALUE), -CENTER_VALUE, MAX_VALUE - CENTER_VALUE) + CENTER_VALUE; }
function value2Rads(value: number) { return (value - CENTER_VALUE)*RATIO_VALUE2RADS; }
function clampValue(value: number) { return math.clamp(value, 0, MAX_VALUE); }

// http://unixpapa.com/js/key.html
var KEY_TAB = 9,
    KEY_ENTER = 13,
    KEY_ESC = 27,
    KEY_PAGEUP = 33,
    KEY_PAGEDOWN = 34,
    KEY_END = 35,
    KEY_HOME = 36,
    KEY_LEFT = 37,
    KEY_UP = 38,
    KEY_RIGHT = 39,
    KEY_DOWN = 40;

var DEFAULT_PGAIN = 32;

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

    return constants.isJointBaseOfPair(jointId) ? v : getMirrorValue(v);
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
            pGains: buildValueArray(stage.pGains || {}, DEFAULT_PGAIN),
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

var animatorTemplate = DOMTemplate.forText(
    '<div>' +
      '<h2 class="script-name"></h2>' +
      '<div class="timeline-container">' +
        '<ul class="joint-names"></ul>' +
        '<ul class="stages"></ul>' +
      '</div>' +
    '</div>');

class AnimatorModule extends Module
{
    constructor()
    {
        super('animator', 'animator', {fullScreen: true});
    }

    private update(script: IScriptViewModel)
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

        var t = this;
        values.on('click', function(data: any, index: number) { t.onEditValue(this); }, true);
    }

    private onEditValue(element: HTMLLIElement)
    {
        console.log(element);
        var textbox = document.createElement('input');
        textbox.value = element.textContent;
        element.textContent = null;
        element.appendChild(textbox);

        textbox.addEventListener('keydown', e =>
        {
            if (e.keyCode === KEY_ENTER)
            {

            }
        });
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

        this.update(toViewModel(scripts.allMotionScripts[2]));

        this.element.tabIndex = 0; // allow element to have keyboard focus
        this.element.addEventListener('keydown', e =>
        {
            console.dir(e);
            if (e.keyCode === KEY_LEFT) {
                console.log('left');
            }
            else if (e.keyCode === KEY_UP) {
                console.log('up');
            }
            else if (e.keyCode === KEY_RIGHT) {
                console.log('right');
            }
            else if (e.keyCode === KEY_DOWN) {
                console.log('down');
            }
        }, true);

        this.intervalHandler = window.setInterval(() => this.update(toViewModel(scripts.allMotionScripts[Math.floor(Math.random() * scripts.allMotionScripts.length)])), 5000);
    }

    private intervalHandler: number;

    public unload()
    {
        window.clearInterval(this.intervalHandler);
    }
}

export = AnimatorModule;

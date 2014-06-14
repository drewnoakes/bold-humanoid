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

function getJointValue(values: scripts.JointValues, jointId: number): number
{
    var v = values[constants.jointMotionFileNames[jointId]];

    if (v != null)
        return v;

    if (jointId > 18)
        throw new Error("Values for head joints must be specified");

    var pairBaseId = constants.getPairJointId(jointId);
    var pairName = constants.jointPairMotionFileNames[pairBaseId];

    v = values[pairName];

    if (v == null)
        throw new Error("No joint data specified for joint ID " + jointId);

    return jointId == pairBaseId
        ? v
        : getMirrorValue(v);
}

var DEFAULT_PGAIN = 32;

function getJointGain(gains: scripts.JointValues, jointId: number): number
{
    if (gains == null)
        return DEFAULT_PGAIN;

    var v = gains[constants.jointMotionFileNames[jointId]];

    if (v != null)
        return v;

    if (jointId > 18)
        return DEFAULT_PGAIN;

    var pairBaseId = constants.getPairJointId(jointId);
    var pairName = constants.jointPairMotionFileNames[pairBaseId];

    v = gains[pairName];

    return v == null ? DEFAULT_PGAIN : v;
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
            pGains: _.map(constants.jointIdNumbers, jointId => getJointGain(stage.pGains, jointId)),
            keyFrames: _.map(stage.keyFrames, (keyFrame: scripts.KeyFrame) => ({
                pauseCycles: keyFrame.pauseCycles || 0,
                moveCycles: keyFrame.moveCycles,
                values: _.map(constants.jointIdNumbers, jointId => getJointValue(keyFrame.values, jointId))
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

    private buildUI(script: IScriptViewModel)
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
            .append("li")
            .attr("taboffset", "0")
            .attr("data-type", "gain");

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
            .append("li")
            .attr("data-type", "value")
            .text(d => d);

        var t = this;
        values.on('click', function(data: any, index: number) { t.setFocus(this); }, true);
        values.on('dblclick', function(data: any, index: number) { t.startEdit(this); }, true);
    }

    private clearFocus(): void
    {
        _.each(this.element.querySelectorAll('li.focussed'), (element: HTMLElement) => element.classList.remove('focussed'));
    }

    private setFocus(element: HTMLLIElement): void
    {
        this.clearFocus();
        element.classList.add('focussed');
        this.element.focus();
    }

    private editTextBox: HTMLInputElement = document.createElement('input');
    private editElement: HTMLLIElement;

    private startEdit(element: HTMLLIElement)
    {
        this.stopEdit();
        this.clearFocus();

        this.editElement = element;

        this.editTextBox.value = element.textContent;
        element.textContent = null;
        element.appendChild(this.editTextBox);
        this.editTextBox.focus();
        this.editTextBox.setSelectionRange(0, this.editTextBox.value.length);

        this.editTextBox.addEventListener('keydown', e =>
        {
            switch (e.keyCode)
            {
                case KEY_ENTER:
                {
                    e.preventDefault();
                    e.stopImmediatePropagation();

                    var text = this.editTextBox.value;
                    element.removeChild(this.editTextBox);
                    element.textContent = text;
                    this.setFocus(element);
                    break;
                }
                case KEY_ESC:
                {
                    e.preventDefault();
                    e.stopImmediatePropagation();

                    this.stopEdit();
                }
            }
        });
    }

    private stopEdit()
    {

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
            .text(jointId => constants.jointNiceNames[jointId])
            .on('keydown', this.onKeyDown.bind(this));

        this.buildUI(toViewModel(scripts.allMotionScripts[2]));

        this.element.tabIndex = 0; // allow element to have keyboard focus
        this.element.addEventListener('keydown', this.onKeyDown.bind(this), true);

//        this.intervalHandler = window.setInterval(() => this.buildUI(toViewModel(scripts.allMotionScripts[Math.floor(Math.random() * scripts.allMotionScripts.length)])), 5000);
    }

    private onKeyDown(e: KeyboardEvent)
    {
        console.dir(e);

        switch (e.keyCode)
        {
            case KEY_ESC:
                console.log('KEY_ESC');
                this.clearFocus();
                break;
            case KEY_LEFT:
                console.log('KEY_LEFT');
                break;
            case KEY_UP:
                console.log('KEY_UP');
                break;
            case KEY_RIGHT:
                console.log('KEY_RIGHT');
                break;
            case KEY_DOWN:
                console.log('KEY_DOWN');
                break;
        }
    }

//    private intervalHandler: number;

    public unload()
    {
//        window.clearInterval(this.intervalHandler);
    }
}

export = AnimatorModule;

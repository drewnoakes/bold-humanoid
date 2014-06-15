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
        stages: _.map(script.stages, (stage: scripts.Stage, stageIndex: number) => ({
            index: stageIndex,
            repeat: stage.repeat || 0,
            speed: stage.speed || 0,
            pGains: _.map(constants.jointIdNumbers, jointId => getJointGain(stage.pGains, jointId)),
            keyFrames: _.map(stage.keyFrames, (keyFrame: scripts.KeyFrame, keyFrameIndex: number) => ({
                index: keyFrameIndex,
                pauseCycles: keyFrame.pauseCycles || 0,
                moveCycles: keyFrame.moveCycles,
                values: _.map(constants.jointIdNumbers, jointId => getJointValue(keyFrame.values, jointId))
            }))
        }))
    }
}

interface IKeyFrameViewModel
{
    index: number;
    pauseCycles: number;
    moveCycles: number;
    values: number[];
}

interface IStageViewModel
{
    index: number;
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

enum ValueType
{
    Gain,
    MotorValue
}

interface ValueData
{
    type: ValueType;
    stageIndex: number;
    jointId: number;
    keyFrameIndex?: number; // only if type == MotorValue
}

function findParent(element: HTMLElement, className: string)
{
    while ((element = <HTMLElement>element.parentElement) != null)
    {
        if (element.classList.contains(className))
            return element;
    }
    return null;
}

class AnimatorModule extends Module
{
    private editTextBox: HTMLInputElement = document.createElement('input');
    private focusElement: HTMLLIElement;
    private script: IScriptViewModel;

    constructor()
    {
        super('animator', 'animator', {fullScreen: true});

        this.editTextBox.addEventListener('click', e => { e.stopPropagation(); });
        this.editTextBox.addEventListener('dblclick', e => { e.stopPropagation(); });

        this.editTextBox.addEventListener('keydown', e =>
        {
            // When editing, disallow key events from bubbling to parent(s)
            e.stopPropagation();

            switch (e.keyCode)
            {
                case KEY_ENTER:
                {
                    console.assert(this.focusElement === this.editTextBox.parentElement);

                    // TODO apply the edit to the view model and rebuild UI instead of just copying text
                    this.focusElement.textContent = this.editTextBox.value;

                    this.setFocus(this.focusElement);
                    this.stopEdit();
                    break;
                }
                case KEY_ESC:
                {
                    this.stopEdit();
                    break;
                }
                case KEY_LEFT:
                {
                    if (this.editTextBox.selectionStart === this.editTextBox.selectionEnd && this.editTextBox.selectionStart === 0)
                        e.preventDefault();
                    // TODO move focussed item left, if possible
                    break;
                }
                case KEY_RIGHT:
                {
                    if (this.editTextBox.selectionStart === this.editTextBox.selectionEnd &&
                        this.editTextBox.selectionStart === this.editTextBox.value.length)
                        e.preventDefault();
                    // TODO move focussed item right, if possible
                    break;
                }
                case KEY_UP:
                {
                    // TODO move focussed item up, if possible
                    break;
                }
                case KEY_DOWN:
                {
                    // TODO move focussed item down, if possible
                    break;
                }
            }
        });
    }

    private getValueData(element: HTMLLIElement): ValueData
    {
        var valueData: ValueData = {
            type: element.dataset["type"] === "gain" ? ValueType.Gain : ValueType.MotorValue,
            stageIndex: parseInt(findParent(element, "stage").dataset["index"]),
            jointId: parseInt(element.dataset["jointid"])

        };

        if (valueData.type === ValueType.MotorValue)
            valueData.keyFrameIndex = parseInt(findParent(element, "key-frame").dataset["index"]);

        return  valueData;
    }

    private getValueElement(valueData: ValueData): HTMLLIElement
    {
        return valueData.type === ValueType.Gain
            ? <HTMLLIElement>this.element.querySelector('li.stage[data-index="' + valueData.stageIndex + '"] li[data-type="gain"][data-jointid="' + valueData.jointId + '"]')
            : <HTMLLIElement>this.element.querySelector('li.stage[data-index="' + valueData.stageIndex + '"] li.key-frame[data-index="' + valueData.keyFrameIndex + '"] li[data-type="value"][data-jointid="' + valueData.jointId + '"]');
    }

    private buildUI()
    {
        var focussedValueData = this.focusElement != null ? this.getValueData(this.focusElement) : null;

        var t = this;

        // Set the header directly
        d3.select(this.element).select("h2.script-name").text(this.script.name);

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
            .data(this.script.stages);

        var enteredStages = stages.enter()
            .append("li")
            .classed("stage", true)
            .attr("data-index", (d, i) => i.toString());

        enteredStages.append("ul").classed("gains", true);
        enteredStages.append("ul").classed("key-frames", true);

        stages.exit().remove();

        //////// GAINS

        var gains = stages.select("ul.gains")
            .selectAll("li")
            .data(stage => stage.pGains);

        gains.enter()
            .append("li")
            .attr("data-type", "gain")
            .attr("data-jointid", (d, i) => (i + 1).toString())
            .on('click', function(data: any, index: number) { t.setFocus(this); })
            .on('dblclick', function(data: any, index: number) { t.startEdit(this); });


        gains.text(d => d.toString());

        gains.exit().remove();

        //////// KEY FRAMES

        var keyFrames = stages.select("ul.key-frames")
            .selectAll("li")
            .data((stage: IStageViewModel) => stage.keyFrames);

        var enteredKeyFrames = keyFrames.enter()
            .append("li")
            .classed("key-frame", true)
            .attr("data-index", d => d.index);

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
            .attr("data-jointid", (d, i) => (i + 1).toString())
            .text(d => d)
            .on('click', function(data: any, index: number) { t.setFocus(this); })
            .on('dblclick', function(data: any, index: number) { t.startEdit(this); });

        if (focussedValueData != null)
            this.setFocus(this.getValueElement(focussedValueData));
    }

    private clearFocus(): void
    {
        _.each(this.element.querySelectorAll('li.focussed'), (element: HTMLElement) => element.classList.remove('focussed'));
        this.focusElement = null;
    }

    private setFocus(element: HTMLLIElement): void
    {
        if (this.focusElement === element)
            return;

        if (this.focusElement)
            this.clearFocus();

        // Store a reference to it
        this.focusElement = element;

        // Mark it as focussed
        this.focusElement.classList.add('focussed');

        // Focus the outer module-level element, where we trap keyboard events
        this.element.focus();
    }

    private startEdit(element: HTMLLIElement)
    {
        this.stopEdit();
        this.setFocus(element);

        this.editTextBox.value = element.textContent;
        element.textContent = null;
        element.appendChild(this.editTextBox);
        this.editTextBox.focus();
        this.editTextBox.setSelectionRange(0, this.editTextBox.value.length);
    }

    private stopEdit()
    {
        // TODO implement!!!
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

        this.script = toViewModel(scripts.allMotionScripts[2]);
        this.buildUI();

        this.element.tabIndex = 0; // allow element to have keyboard focus
        this.element.addEventListener('keydown', this.onKeyDown.bind(this));

//        this.intervalHandler = window.setInterval(() => this.buildUI(toViewModel(scripts.allMotionScripts[Math.floor(Math.random() * scripts.allMotionScripts.length)])), 5000);
    }

    private moveFocus(deltaX: number, deltaY: number)
    {
        var li: HTMLLIElement;

        // Up/down
        if (deltaY !== 0)
        {
            var d = this.getValueData(this.focusElement);
            d.jointId += deltaY;
            li = this.getValueElement(d);
        }

        if (deltaX !== 0)
        {
            var d = this.getValueData(this.focusElement);
            if (deltaX < 0)
            {
                if (d.type === ValueType.MotorValue)
                {
                    if (d.keyFrameIndex === 0)
                    {
                        d.type = ValueType.Gain;
                        delete d.keyFrameIndex;
                        li = this.getValueElement(d);
                    }
                    else
                    {
                        d.keyFrameIndex--;
                        li = this.getValueElement(d);
                    }
                }
                else
                {
                    d.type = ValueType.MotorValue;
                    d.stageIndex--;
                    if (d.stageIndex != -1)
                    {
                        d.keyFrameIndex = this.script.stages[d.stageIndex].keyFrames.length - 1;
                        li = this.getValueElement(d);
                    }
                }
            }
            else
            {
                if (d.type === ValueType.MotorValue)
                {
                    d.keyFrameIndex++;
                    li = this.getValueElement(d);
                    if (li == null)
                    {
                        d.type = ValueType.Gain;
                        d.stageIndex++;
                        delete d.keyFrameIndex;
                        li = this.getValueElement(d);
                    }
                }
                else
                {
                    d.type = ValueType.MotorValue;
                    d.keyFrameIndex = 0;
                    li = this.getValueElement(d);
                }
            }
        }

        if (!li)
            return false;

        this.setFocus(li);
        return true;
    }

    private onKeyDown(e: KeyboardEvent)
    {
        switch (e.keyCode)
        {
            case KEY_ESC:
                console.log('KEY_ESC');
                this.clearFocus();
                break;

            case KEY_LEFT:  if (this.moveFocus(-1,  0)) e.preventDefault(); break;
            case KEY_RIGHT: if (this.moveFocus( 1,  0)) e.preventDefault(); break;
            case KEY_UP:    if (this.moveFocus( 0, -1)) e.preventDefault(); break;
            case KEY_DOWN:  if (this.moveFocus( 0,  1)) e.preventDefault(); break;

            default:
                console.dir(e);
        }
    }

//    private intervalHandler: number;

    public unload()
    {
//        window.clearInterval(this.intervalHandler);
    }
}

export = AnimatorModule;

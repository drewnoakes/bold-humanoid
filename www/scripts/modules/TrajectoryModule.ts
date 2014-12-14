/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/d3.d.ts" />

import constants = require('constants');
import canvas = require('util/canvas');
import Checkbox = require('controls/Checkbox');
import data = require('data');
import dom = require("util/domdomdom");
import BodyFigure = require('controls/BodyFigure');
import ICloseable = require('ICloseable');
import Module = require('Module');
import Select = require('controls/Select');
import state = require('state');
import Trackable = require('util/Trackable');

var chartHeight = 300,
    chartWidth = 430,
    maxDataLength = chartWidth;

var _yScale = d3.scale.linear()
    .range([0, chartHeight])
    .domain([4096, 0]);

var _yScaleMirror = d3.scale.linear()
    .range([0, chartHeight])
    .domain([0, 4096]);

var _yModScale = d3.scale.linear()
    .range([0, chartHeight])
    .domain([100, -100]);

var _yModScaleMirror = d3.scale.linear()
    .range([0, chartHeight])
    .domain([-100, 100]);

enum Mode
{
    All,
    ControlOnly,
    ModulationOnly
}

class TrajectoryModule extends Module
{
    private data: state.BodyControl[] = [];

    private mirrorValues: Trackable<boolean> = new Trackable<boolean>(true);
    private mode: Trackable<Mode> = new Trackable<Mode>(Mode.All);
    private isRecording: boolean = false;
    private skipFirstDatum: boolean = true;

    private subscription: ICloseable;
    private bodyFigure: BodyFigure;
    private recordButton: HTMLElement;
    private canvas: HTMLCanvasElement;

    private yScaleNormal: D3.Scale.LinearScale;
    private yScaleMirror: D3.Scale.LinearScale;

    constructor()
    {
        super('trajectory', 'trajectory');

        this.mirrorValues.onchange(() => this.render());
    }

    public load(width: number)
    {
        this.recordButton = dom('button.record', 'record');
        this.recordButton.addEventListener('click', this.toggleIsRecording.bind(this));

        var modeItems = [
            { value:Mode.All,            text:"Control + Mod" },
            { value:Mode.ControlOnly,    text:"Control only" },
            { value:Mode.ModulationOnly, text:"Modulation only" }
        ];

        var controlContainer =
            dom('div.controls',
                this.recordButton,
                new Select(this.mode, modeItems).element,
                new Checkbox('Mirror values', this.mirrorValues).element);

        var hoverText = dom('div.hover-value');

        this.canvas = document.createElement('canvas');
        this.canvas.width = chartWidth;
        this.canvas.height = chartHeight;

        this.canvas.addEventListener('mousemove', e => hoverText.textContent = Math.round(this.yScaleNormal.invert(e.offsetY)).toString());
        this.canvas.addEventListener('mouseout', e => hoverText.textContent = '');

        this.bodyFigure = new BodyFigure({hasHover: true, hasSelection: true});

        dom(this.element,
            controlContainer,
            hoverText,
            this.canvas,
            this.bodyFigure.element);

        this.bodyFigure.hoverJointId.track(this.render.bind(this));
        this.bodyFigure.selectedJointIds.track(this.render.bind(this));
        this.bodyFigure.visitJoints((jointId, jointDiv) => jointDiv.textContent = jointId.toString());

        this.closeables.add(this.mode.track(mode => {
            if (mode === Mode.ModulationOnly)
            {
                this.yScaleNormal = _yModScale;
                this.yScaleMirror = _yModScaleMirror;
            }
            else
            {
                this.yScaleNormal = _yScale;
                this.yScaleMirror = _yScaleMirror;
            }
            this.render();
        }));
    }

    public unload()
    {
        if (this.subscription)
        {
            this.subscription.close();
            delete this.subscription;
        }

        delete this.canvas;
        delete this.bodyFigure;
        delete this.data;
        delete this.recordButton;
    }

    private onBodyControlState(data: state.BodyControl)
    {
        console.assert(this.data.length < maxDataLength);

        if (this.skipFirstDatum) {
            this.skipFirstDatum = false;
            return;
        }

        this.data.push(data);

        if (this.data.length === maxDataLength) {
            this.toggleIsRecording();
        }
    }

    private toggleIsRecording()
    {
        this.isRecording = !this.isRecording;

        if (this.isRecording) {
            canvas.clear(this.canvas.getContext('2d'));
            this.recordButton.classList.add('recording');
            this.recordButton.textContent = 'recording...';
            this.subscription = new data.Subscription<state.BodyControl>(
                constants.protocols.bodyControlState,
                {
                    onmessage: this.onBodyControlState.bind(this)
                }
            );
            this.skipFirstDatum = true;
            this.data = [];
        } else {
            this.recordButton.classList.remove('recording');
            this.recordButton.textContent = 'record';
            this.subscription.close();
            this.render();
        }
    }

    private render()
    {
        if (this.isRecording || this.data.length === 0)
            return;

        var ctx = this.canvas.getContext('2d');

        var x = d3.scale.linear()
            .range([0, chartWidth])
            .domain([this.data[0].cycle, this.data[this.data.length - 1].cycle]);

        canvas.clear(ctx);

        var selectedJointIds = this.bodyFigure.selectedJointIds.getValue(),
            hoverJointId = this.bodyFigure.hoverJointId.getValue();

        var jointIds = selectedJointIds && selectedJointIds.length ? selectedJointIds : d3.range(1, 21);

        // Draw tick markers to show when samples were taken
        ctx.strokeStyle = '#888';
        _.each(this.data, d =>
        {
            var px = Math.round(x(d.cycle) - 0.5) + 0.5;
            ctx.moveTo(px, chartHeight);
            ctx.lineTo(px, chartHeight - 5);
        });
        ctx.stroke();

        var mode = this.mode.getValue(),
            mirror = this.mirrorValues.getValue();

        var drawLine = jointId =>
        {
            jointId--;
            ctx.beginPath();
            _.each(this.data, d =>
            {
                var y = mirror && jointId % 2 === 0 && jointId !== 20 ? this.yScaleMirror : this.yScaleNormal;
                var v = 0;
                if (mode === Mode.All || mode === Mode.ControlOnly) v += d.joints[jointId].v;
                if (mode === Mode.All || mode === Mode.ModulationOnly) v += d.joints[jointId].m;
                var px = x(d.cycle),
                    py = y(v);
                ctx.lineTo(px, py);
            });
            ctx.stroke();
        };

        ctx.strokeStyle = '#792485';
        _.each(jointIds, jointId =>
        {
            if (hoverJointId === jointId)
                return;
            drawLine(jointId);
        });

        if (hoverJointId > 0) {
            ctx.strokeStyle = '#FFF';
            drawLine(hoverJointId);
        }
    }
}

export = TrajectoryModule;

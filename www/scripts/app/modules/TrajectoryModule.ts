/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/d3.d.ts" />

import constants = require('constants');
import Checkbox = require('controls/Checkbox');
import data = require('data');
import BodyFigure = require('controls/BodyFigure');
import Module = require('Module');
import state = require('state');
import canvas = require('util/canvas');
import Trackable = require('util/Trackable');

var chartHeight = 300,
    chartWidth = 430,
    maxDataLength = chartWidth;

var yScale = d3.scale.linear()
    .range([0, chartHeight])
    .domain([4096, 0]);

var yScaleMirror = d3.scale.linear()
    .range([0, chartHeight])
    .domain([0, 4096]);

interface ICloseable
{
    close();
}

class TrajectoryModule extends Module
{
    private data: state.BodyControl[] = [];

    private mirrorValues: Trackable<boolean> = new Trackable<boolean>(true);
    private isRecording: boolean = false;
    private skipFirstDatum: boolean = true;

    private subscription: ICloseable;
    private bodyFigure: BodyFigure;
    private recordButton: HTMLButtonElement;
    private canvas: HTMLCanvasElement;

    constructor()
    {
        super('trajectory', 'trajectory');

        this.mirrorValues.onchange(() => this.render());
    }

    public load(width: number)
    {
        this.recordButton = document.createElement('button');
        this.recordButton.className = 'record';
        this.recordButton.textContent = 'record';
        this.recordButton.addEventListener('click', this.toggleIsRecording.bind(this));

        var mirrorCheckbox = new Checkbox('Mirror values', this.mirrorValues);

        var controlContainer = document.createElement('div');
        controlContainer.className = 'controls';
        controlContainer.appendChild(this.recordButton);
        controlContainer.appendChild(mirrorCheckbox.element);
        this.element.appendChild(controlContainer);

        var hoverText = document.createElement('div');
        hoverText.className = 'hover-value';
        this.element.appendChild(hoverText);

        this.canvas = document.createElement('canvas');
        this.canvas.width = chartWidth;
        this.canvas.height = chartHeight;
        this.element.appendChild(this.canvas);

        this.canvas.addEventListener('mousemove', e => hoverText.textContent = Math.round(yScale.invert(e.offsetY)).toString());
        this.canvas.addEventListener('mouseout', e => hoverText.textContent = '');

        this.bodyFigure = new BodyFigure({hasHover: true, hasSelection: true});
        this.element.appendChild(this.bodyFigure.element);

        this.bodyFigure.hoverJointId.track(this.render.bind(this));
        this.bodyFigure.selectedJointIds.track(this.render.bind(this));
        this.bodyFigure.visitJoints((jointId, jointDiv) => jointDiv.textContent = jointId.toString());
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

        var drawLine = jointId =>
        {
            jointId--;
            ctx.beginPath();
            _.each(this.data, d =>
            {
                var y = this.mirrorValues.getValue() && jointId % 2 === 0 && jointId !== 20 ? yScaleMirror : yScale;
                var px = x(d.cycle),
                    py = y(d.joints[jointId].v);
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

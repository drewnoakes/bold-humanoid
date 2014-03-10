/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import Animator = require('Animator');
import FieldLinePlotter = require('FieldLinePlotter');
import constants = require('constants');
import data = require('data');
import HeadControls = require('controls/HeadControls');
import mouse = require('util/mouse');
import geometry = require('util/geometry');
import Module = require('Module');
import state = require('state');

class Agent2dModule extends Module
{
    private canvas: HTMLCanvasElement;
    private hoverInfo: HTMLDivElement;
    private transform: geometry.Transform;

    private ballPosition: number[];
    private goalPositions: geometry.IPoint2[];
    private visibleFieldPoly: number[][];
    private occlusionRays: number[][];
    private observedLineSegments: geometry.ILineSegment2[];
    private scale: number;
    private animator: Animator;

    constructor()
    {
        super('agent-2d', '2d agent');

        this.animator = new Animator(this.render.bind(this));
    }

    public load(element: HTMLDivElement)
    {
        this.transform = new geometry.Transform();

        this.canvas = document.createElement('canvas');
        this.hoverInfo = document.createElement('div');
        this.hoverInfo.className = 'hover-info';

        element.appendChild(this.canvas);
        element.appendChild(this.hoverInfo);
        element.appendChild(new HeadControls().element);

        this.canvas.addEventListener('mousewheel', event =>
        {
            event.preventDefault();

            this.scale *= Math.pow(1.1, event.wheelDelta / 80);
            this.scale = Math.max(20, this.scale);
            this.transform = new geometry.Transform()
                .translate(this.canvas.width / 2, this.canvas.height / 2)
                .scale(this.scale, -this.scale);
            this.animator.setRenderNeeded();
        });

        this.canvas.addEventListener('mousemove', event =>
        {
            mouse.polyfill(event);
            var p = this.transform.clone().invert().transformPoint(event.offsetX, event.offsetY);
            this.hoverInfo.textContent = p.x.toFixed(2) + ', ' + p.y.toFixed(2);
        });

        this.canvas.addEventListener('mouseleave', () =>
        {
            this.hoverInfo.textContent = '';
        });

        this.closeables.add(new data.Subscription<state.AgentFrame>(
            constants.protocols.agentFrameState,
            {
                onmessage: this.onAgentFrameData.bind(this)
            }));

        this.closeables.add(() => this.animator.stop());

        this.animator.start();
    }

    private onAgentFrameData(data: state.AgentFrame)
    {
        this.ballPosition = data.ball;
        this.visibleFieldPoly = data.visibleFieldPoly;
        this.occlusionRays = data.occlusionRays;
        this.observedLineSegments = [];
        this.goalPositions = [];

        _.each(data.lines, line =>
        {
            var p1 = { x: line[0], y: line[1]/*, z: line[2]*/ };
            var p2 = { x: line[3], y: line[4]/*, z: line[5]*/ };
            this.observedLineSegments.push({ p1: p1, p2: p2 });
        });

        _.each(data.goals, goalPos => this.goalPositions.push({ x: goalPos[0], y: goalPos[1] }));

        this.animator.setRenderNeeded(); // TODO only draw agentFrameData, on its own canvas
    }

    public onResized(width, height)
    {
        this.canvas.width = width;
        this.canvas.height = height;

        this.scale = Math.min(width / 12, height / 12);
        this.transform = new geometry.Transform()
            .translate(this.canvas.width / 2, this.canvas.height / 2)
            .scale(this.scale, -this.scale);
        this.animator.setRenderNeeded();
    }

    private render()
    {
        var scale = this.transform.getScale(),
            options = {
                lineWidth: 1/scale,
                goalStrokeStyle: 'yellow',
                groundFillStyle: '#008800',
                lineStrokeStyle: '#ffffff',
                visibleFieldPolyLineWidth: 1 / scale,
                visibleFieldPolyStrokeStyle: '#0000ff'
            },
            context = this.canvas.getContext('2d');

        this.transform.applyTo(context);

        FieldLinePlotter.drawField(context, options);

        var maxDistance = Math.sqrt(
            Math.pow(constants.fieldX + 2 * constants.outerMarginMinimum, 2) +
            Math.pow(constants.fieldY + 2 * constants.outerMarginMinimum, 2));

        context.strokeStyle = 'white';
        context.lineWidth = 0.5 / this.scale;
        context.beginPath();
        context.moveTo(0, maxDistance);
        context.lineTo(0, -maxDistance);
        context.moveTo(-maxDistance, 0);
        context.lineTo(maxDistance, 0);
        context.stroke();

        for (var r = 1; r < maxDistance; r++) {
            context.beginPath();
            context.arc(0, 0, r, 0, Math.PI * 2);
            context.stroke();
        }

        if (this.observedLineSegments && this.observedLineSegments.length)
            FieldLinePlotter.drawLineSegments(context, options, this.observedLineSegments, 1, '#0000ff');

        if (this.visibleFieldPoly)
            FieldLinePlotter.drawVisibleFieldPoly(context, options, this.visibleFieldPoly);

        if (this.ballPosition)
            FieldLinePlotter.drawBall(context, options, this.ballPosition);

        if (this.goalPositions)
            FieldLinePlotter.drawGoalPosts(context, options, this.goalPositions);

        if (this.occlusionRays)
            FieldLinePlotter.drawOcclusionRays(context, options, this.occlusionRays);
    }
}

export = Agent2dModule;

/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import constants = require('constants');
import ControlBuilder = require('ControlBuilder');
import data = require('data');
import plotter = require('FieldLinePlotter');
import HeadControls = require('HeadControls');
import interaction = require('interaction');
import mouse = require('util/mouse');
import geometry = require('../util/geometry');
import state = require('state');
import Module = require('Module');

class World2dModule extends Module
{
    private needsRender: boolean = false;
    private stopAnimation: boolean = false;

    private particles: number[][];
    private transform: geometry.Transform;
    private canvas: HTMLCanvasElement;
    private agentPosition: number[];
    private ballPosition: number[];
    private visibleFieldPoly: number[][];
    private observedLineSegments: geometry.ILineSegment2[];
    private goalPositions: geometry.IPoint2[];

    private hoverInfo: HTMLDivElement;

    constructor()
    {
        super('world-2d', '2d world');
    }

    bindEvents()
    {
        new interaction.Dragger(this.canvas, (evt: interaction.IDragEvent) =>
        {
            this.transform = new geometry.Transform()
                .translate(evt.lastDeltaX, evt.lastDeltaY)
                .multiply(this.transform);
            this.needsRender = true;
        });

        this.canvas.addEventListener('mousewheel', event =>
        {
            event.preventDefault();
            var scale = Math.pow(1.1, event.wheelDelta / 80);
            this.transform = new geometry.Transform()
                .translate(event.offsetX, event.offsetY)
                .scale(scale, scale)
                .translate(-event.offsetX, -event.offsetY)
                .multiply(this.transform);
            this.needsRender = true;
        });

        this.canvas.addEventListener('mousemove', event =>
        {
            mouse.polyfill(event);
            var p = this.transform.clone().invert().transformPoint(event.offsetX, event.offsetY);
            this.hoverInfo.textContent = p.x.toFixed(2) + ', ' + p.y.toFixed(2);
        });

        this.canvas.addEventListener('mouseleave', () => this.hoverInfo.textContent = '');
    }

    public load(element: HTMLDivElement)
    {
        this.transform = new geometry.Transform().scale(1, -1);

        this.canvas = document.createElement('canvas');
        this.hoverInfo = document.createElement('div');
        this.hoverInfo.className = 'hover-info';

        var localiserControlContainer = document.createElement('div');
        localiserControlContainer.className = 'localiser-controls';
        ControlBuilder.actions('localiser', localiserControlContainer);

        element.appendChild(this.canvas);
        element.appendChild(new HeadControls().element);
        element.appendChild(localiserControlContainer);
        element.appendChild(this.hoverInfo);

        this.bindEvents();

        this.closeables.add(new data.Subscription<state.WorldFrame>(
            constants.protocols.worldFrameState,
            {
                onmessage: this.onWorldFrameData.bind(this)
            }
        ));

        // TODO only subscribe if user checks a box
        this.closeables.add(new data.Subscription<state.Particle>(
            constants.protocols.particleState, {
                onmessage: this.onParticleData.bind(this)
            }
        ));

        this.stopAnimation = false;
        this.needsRender = true;
        this.animate();
    }

    public unload()
    {
        this.stopAnimation = true;
    }

    public onResized(width: number, height: number)
    {
        var fieldLengthX = (constants.fieldX + 2 * constants.outerMarginMinimum);
        var fieldLengthY = (constants.fieldY + 2 * constants.outerMarginMinimum);
        var ratio = fieldLengthX / fieldLengthY;

        this.canvas.width = width;
        this.canvas.height = width / ratio;

        var scale = Math.min(
            width / fieldLengthX,
            (width / ratio) / fieldLengthY);
        this.transform = new geometry.Transform()
            .scale(scale, -scale)
            .translate(fieldLengthX/2, -fieldLengthY/2);
        this.needsRender = true;
    }

    private animate()
    {
        if (this.stopAnimation)
            return;

        window.requestAnimationFrame(this.animate.bind(this));

        if (!this.needsRender)
            return;

        var scale = this.transform.getScale(),
            options = {
                goalStrokeStyle: 'yellow',
                groundFillStyle: '#008800',
                lineStrokeStyle: '#ffffff',
                visibleFieldPolyLineWidth: 1/scale,
                visibleFieldPolyStrokeStyle: '#0000ff',
                particleHue: 240,
                particleSize: Math.max(0.015, 2/scale),
                ballFillStyle: 'red'
            },
            context = this.canvas.getContext('2d');

        this.transform.applyTo(context);

        plotter.drawField(context, options);
        plotter.drawFieldLines(context, options);
        plotter.drawGoals(context, options);

        if (this.observedLineSegments && this.observedLineSegments.length)
            plotter.drawLineSegments(context, options, this.observedLineSegments, 0.02, '#000088');

        if (this.agentPosition)
            plotter.drawAgentPosition(context, options, this.agentPosition);

        if (this.particles)
            plotter.drawParticles(context, options, this.particles);

        if (this.visibleFieldPoly)
            plotter.drawVisibleFieldPoly(context, options, this.visibleFieldPoly);

        if (this.ballPosition)
            plotter.drawBall(context, options, this.ballPosition);

        if (this.goalPositions) {
            options.goalStrokeStyle = '#FF5800';
            plotter.drawGoalPosts(context, options, this.goalPositions);
        }

        this.needsRender = false;
    }

    private onWorldFrameData(data: state.WorldFrame)
    {
        this.agentPosition = data.pos;
        this.ballPosition = data.ball;
        this.visibleFieldPoly = data['visible-field-poly'];
        this.observedLineSegments = [];
        this.goalPositions = [];

        _.each(data.lines, line =>
        {
            var p1 = { x: line[0], y: line[1]/*, z: line[2]*/ };
            var p2 = { x: line[3], y: line[4]/*, z: line[5]*/ };
            this.observedLineSegments.push({ p1: p1, p2: p2 });
        });

        _.each(data.goals, goalPos =>
        {
            this.goalPositions.push({ x: goalPos[0], y: goalPos[1] });
        });

        this.needsRender = true; // TODO only draw worldFrameData, on its own canvas
    }

    private onParticleData(data: state.Particle)
    {
        this.particles = data.particles;
        this.needsRender = true; // TODO only draw particles, on their own canvas
    }
}

export = World2dModule;

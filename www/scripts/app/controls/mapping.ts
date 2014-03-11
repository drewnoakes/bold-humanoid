/**
 * @author Drew Noakes https://drewnoakes.com
 */

import Animator = require('Animator');
import canvasUtil = require('util/canvas');
import Checkbox = require('controls/Checkbox');
import constants = require('constants');
import data = require('data');
import geometry = require('util/geometry');
import interaction = require('interaction');
import mouse = require('util/mouse');
import plotter = require('plotter');
import state = require('state');
import util = require('util');

export class Map
{
    public hoverPoint: util.Trackable<geometry.IPoint2> = new util.Trackable<geometry.IPoint2>();

    private layers: MapLayer[] = [];

    constructor(private layerContainer: HTMLDivElement, private checkboxContainer: HTMLDivElement, public transform: util.Trackable<geometry.Transform>)
    {
        this.transform.setValue(new geometry.Transform().scale(1, -1));

        new interaction.Dragger(this.layerContainer, (evt: interaction.IDragEvent) =>
        {
            this.transform.setValue(new geometry.Transform()
                .translate(evt.lastDeltaX, evt.lastDeltaY)
                .multiply(this.transform.getValue()));
        });

        this.layerContainer.addEventListener('mousewheel', event =>
        {
            event.preventDefault();
            var scale = Math.pow(1.1, event.wheelDelta / 80);
            this.transform.setValue(new geometry.Transform()
                .translate(event.offsetX, event.offsetY)
                .scale(scale, scale)
                .translate(-event.offsetX, -event.offsetY)
                .multiply(this.transform.getValue()));
        });

        this.layerContainer.addEventListener('mousemove', event =>
        {
            mouse.polyfill(event);
            this.hoverPoint.setValue(this.transform.getValue().clone().invert().transformPoint(event.offsetX, event.offsetY));
        });

        this.layerContainer.addEventListener('mouseleave', () => this.hoverPoint.setValue(null));
    }

    public addLayer(layer: MapLayer)
    {
        this.layerContainer.appendChild(layer.canvas);

        var checkbox = new Checkbox(layer.name, layer.enabled);
        this.checkboxContainer.appendChild(checkbox.element);

        this.layers.push(layer);
    }

    public setPixelSize(width: number, height: number)
    {
        var fieldLengthX = (constants.fieldX + 2 * constants.outerMarginMinimum),
            fieldLengthY = (constants.fieldY + 2 * constants.outerMarginMinimum),
            ratio = fieldLengthX / fieldLengthY;

        // TODO review this approach of forcing the height -- should take advantage of whatever's available
        height = width/ratio;

        this.layerContainer.style.width = width + 'px';
        this.layerContainer.style.height = height + 'px';

        _.each<MapLayer>(this.layers, layer => layer.setPixelSize(width, height));

        var scale = Math.min(
            width / fieldLengthX,
            (width / ratio) / fieldLengthY);

        this.transform.setValue(
            new geometry.Transform()
            .scale(scale, -scale)
            .translate(fieldLengthX/2, -fieldLengthY/2));
    }

    public unload()
    {
        _.each<MapLayer>(this.layers, layer => layer.enabled.setValue(false));

        util.clearChildren(this.layerContainer);
        util.clearChildren(this.checkboxContainer);

        this.layers = [];

        delete this.transform;
    }
}

export class MapLayer
{
    public canvas: HTMLCanvasElement;
    public context: CanvasRenderingContext2D;
    public enabled: util.Trackable<boolean>;

    constructor(public transform: util.Trackable<geometry.Transform>,
                public name: string,
                initiallyEnabled: boolean = true)
    {
        this.canvas = document.createElement('canvas');
        this.context = this.canvas.getContext('2d');

        console.assert(!!this.context);

        this.enabled = new util.Trackable<boolean>(initiallyEnabled);
        this.enabled.track(isEnabled => this.canvas.style.display = isEnabled ? 'block' : 'none');
        this.transform.track(t => t.applyTo(this.context));
    }

    public setPixelSize(width: number, height: number)
    {
        this.canvas.width = width;
        this.canvas.height = height;
        this.context = this.canvas.getContext('2d');
        this.transform.getValue().applyTo(this.context);
    }
}

export class DataLayer<T> extends MapLayer
{
    public data: T;

    constructor(transform: util.Trackable<geometry.Transform>,
                protocolName: string,
                layerName: string,
                render: () => void,
                initiallyEnabled: boolean = true)
    {
        super(transform, layerName, initiallyEnabled);

        var animator = new Animator(render);
        var subscription: data.Subscription<T>;

        this.enabled.track(isEnabled =>
        {
            if (isEnabled)
            {
                subscription = new data.Subscription<T>(
                    protocolName,
                    {
                        onmessage: data =>
                        {
                            this.data = data;
                            animator.setRenderNeeded();
                        }
                    });

                animator.start();
            }
            else
            {
                if (subscription)
                    subscription.close();
                animator.stop();
            }
        });
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export class FieldLineLayer extends MapLayer
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(transform, "Field lines");

        this.transform.track(t =>
        {
            plotter.drawField(this.context, {});
            plotter.drawFieldLines(this.context, {});
            plotter.drawGoals(this.context, {});
        });
    }
}

export class ParticleLayer extends DataLayer<state.Particle>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.particleState,
            "Particles",
            this.render.bind(this));
    }

    private render()
    {
        canvasUtil.clear(this.context, true);

        var scale = this.transform.getValue().getScale();
        var options = {
            particleHue: 240,
            particleSize: Math.max(0.015, 2 / scale)
        };

        if (this.data && this.data.particles)
            plotter.drawParticles(this.context, options, this.data.particles);
    }
}

export class ObservedLineLayer extends DataLayer<state.WorldFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.worldFrameState,
            "Observed lines",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.lines)
                    plotter.drawLineSegments(this.context, this.data.lines, 0.02, '#000088')
            });
    }
}

export class AgentPositionLayer extends DataLayer<state.WorldFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.worldFrameState,
            "Agent position",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.pos)
                    plotter.drawAgentPosition(this.context, {}, this.data.pos)
            });
    }
}

export class VisibleFieldPolyLayer extends DataLayer<state.WorldFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.worldFrameState,
            "Visible area",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.visibleFieldPoly)
                    plotter.drawVisibleFieldPoly(this.context, {visibleFieldPolyLineWidth: 1/this.transform.getValue().getScale()}, this.data.visibleFieldPoly)
            });
    }
}

export class BallPositionLayer extends DataLayer<state.WorldFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.worldFrameState,
            "Ball",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.ball)
                    plotter.drawBall(this.context, {}, this.data.ball)
            });
    }
}

export class ObservedGoalLayer extends DataLayer<state.WorldFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.worldFrameState,
            "Observed goals",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.goals)
                    plotter.drawGoalPosts(this.context, {goalStrokeStyle:'#FF5800'}, this.data.goals)
            });
    }
}

export class OcclusionAreaLayer extends DataLayer<state.WorldFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.worldFrameState,
            "Occlusion area",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.occlusionRays)
                    plotter.drawOcclusionRays(this.context, {lineWidth:1/this.transform.getValue().getScale()}, this.data.occlusionRays)
            });
    }
}

export class TeamLayer extends DataLayer<state.OpenTeam>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.openTeamState,
            "Team",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.teammates)
                    plotter.drawTeammates(this.context, this.data.teammates, this.transform.getValue().getScale());
            });
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export class AgentReferenceLayer extends MapLayer
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(transform, "Agent frame");

        this.transform.track(t =>
        {
            plotter.drawField(this.context, {});

            var maxDistance = constants.minDiagonalFieldDistance;
            var context = this.context;

            context.strokeStyle = 'white';
            context.lineWidth = 0.5 / t.getScale();
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
        });
    }
}

export class AgentObservedLineLayer extends DataLayer<state.AgentFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.agentFrameState,
            "Observed lines",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.lines)
                    plotter.drawLineSegments(this.context, this.data.lines, 0.02, '#000088')
            });
    }
}

export class AgentVisibleFieldPolyLayer extends DataLayer<state.AgentFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.agentFrameState,
            "Visible area",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.visibleFieldPoly)
                    plotter.drawVisibleFieldPoly(this.context, {visibleFieldPolyLineWidth: 1/this.transform.getValue().getScale()}, this.data.visibleFieldPoly)
            });
    }
}

export class AgentBallPositionLayer extends DataLayer<state.AgentFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.agentFrameState,
            "Ball",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.ball)
                    plotter.drawBall(this.context, {}, this.data.ball)
            });
    }
}

export class AgentObservedGoalLayer extends DataLayer<state.AgentFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.agentFrameState,
            "Observed goals",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.goals)
                    plotter.drawGoalPosts(this.context, {goalStrokeStyle:'#FF5800'}, this.data.goals)
            });
    }
}

export class AgentOcclusionAreaLayer extends DataLayer<state.AgentFrame>
{
    constructor(transform: util.Trackable<geometry.Transform>)
    {
        super(
            transform,
            constants.protocols.agentFrameState,
            "Occlusion area",
            () => {
                canvasUtil.clear(this.context, true);
                if (this.data && this.data.occlusionRays)
                    plotter.drawOcclusionRays(this.context, {lineWidth:1/this.transform.getValue().getScale()}, this.data.occlusionRays)
            });
    }
}

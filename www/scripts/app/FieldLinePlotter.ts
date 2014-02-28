/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />

import ControlBuilder = require('ControlBuilder');
import constants = require('constants');
import geometry = require('util/geometry');

export function drawField(context: CanvasRenderingContext2D, options)
{
    context.save();
    context.setTransform(1, 0, 0, 1, 0, 0);
    context.fillStyle = options.groundFillStyle || '#008800';
    context.fillRect(0, 0, context.canvas.width, context.canvas.height);
    context.restore();
}

export function drawFieldLines(context: CanvasRenderingContext2D, options)
{
    // prepare to draw field lines
    context.lineWidth = constants.lineWidth;
    context.strokeStyle = options.lineStrokeStyle || '#ffffff';

    // center circle
    context.beginPath();
    context.arc(0, 0, constants.circleDiameter/2, 0, Math.PI*2, true);

    var halfCrossLengthScaled = constants.penaltyLineLength / 2,
        penaltyX = (constants.fieldX/2 - constants.penaltyMarkDistance),
        penaltyInnerX = penaltyX - halfCrossLengthScaled,
        penaltyOuterX = penaltyX + halfCrossLengthScaled;

    // center cross mark
    context.moveTo(-halfCrossLengthScaled, 0);
    context.lineTo(+halfCrossLengthScaled, 0);

    // left penalty mark
    context.moveTo(-penaltyInnerX, 0);
    context.lineTo(-penaltyOuterX, 0);
    context.moveTo(-penaltyX, halfCrossLengthScaled);
    context.lineTo(-penaltyX, -halfCrossLengthScaled);

    // right penalty mark
    context.moveTo(penaltyInnerX, 0);
    context.lineTo(penaltyOuterX, 0);
    context.moveTo(penaltyX, halfCrossLengthScaled);
    context.lineTo(penaltyX, -halfCrossLengthScaled);

    // outer square
    var x = constants.fieldX/2,
        y = constants.fieldY/2;
    context.strokeRect(-x, -y, constants.fieldX, constants.fieldY);

    context.moveTo(0, y);
    context.lineTo(0, -y);

    var goalAreaY = constants.goalAreaY / 2;

    // left goal area
    context.moveTo(-x, -goalAreaY);
    context.lineTo(-x + constants.goalAreaX, -goalAreaY);
    context.lineTo(-x + constants.goalAreaX, goalAreaY);
    context.lineTo(-x, goalAreaY);

    // right goal area
    context.moveTo(x, -goalAreaY);
    context.lineTo(x - constants.goalAreaX, -goalAreaY);
    context.lineTo(x - constants.goalAreaX, goalAreaY);
    context.lineTo(x, goalAreaY);

    context.stroke();
}

export function drawGoalPosts(context: CanvasRenderingContext2D, options, positions: geometry.IPoint2[])
{
    context.fillStyle = options.goalStrokeStyle || 'yellow';

    _.each(positions, pos =>
    {
        context.beginPath();
        context.arc(pos.x, pos.y, constants.goalPostDiameter/2, 0, Math.PI*2, true);
        context.fill();
    });
}

export function drawGoals(context: CanvasRenderingContext2D, options)
{
    var goalY = constants.goalY / 2,
        x = constants.fieldX/2;

    // TODO the position of these circles is slightly wrong, as the perimeter should line up with the edge of the line

    drawGoalPosts(context, options, [
        {x: x, y: goalY},
        {x: x, y:-goalY},
        {x:-x, y: goalY},
        {x:-x, y:-goalY}
    ]);

    context.strokeStyle = options.goalStrokeStyle || 'yellow';

    context.beginPath();

    // left goal
    context.moveTo(-x, -goalY);
    context.lineTo(-x - constants.goalX, -goalY);
    context.lineTo(-x - constants.goalX, goalY);
    context.lineTo(-x, goalY);

    // right goal
    context.moveTo(x, -goalY);
    context.lineTo(x + constants.goalX, -goalY);
    context.lineTo(x + constants.goalX, goalY);
    context.lineTo(x, goalY);

    context.stroke();
}

export function drawLineSegments(context: CanvasRenderingContext2D, options, lineSegments: geometry.ILineSegment2[], lineWidth, strokeStyle)
{
    context.lineWidth = lineWidth || 0.01;
    context.strokeStyle = strokeStyle || '#0000ff';

    context.beginPath();
    _.each(lineSegments, lineSegment =>
    {
        var p1 = lineSegment.p1,
            p2 = lineSegment.p2;
        context.moveTo(p1.x, p1.y);
        context.lineTo(p2.x, p2.y);
    });
    context.stroke();
}

export function drawVisibleFieldPoly(context: CanvasRenderingContext2D, options, visibleFieldPoly: number[][])
{
    if (visibleFieldPoly.length < 2)
        return;

    context.lineWidth = options.visibleFieldPolyLineWidth || 0.01;
    context.strokeStyle = options.visibleFieldPolyStrokeStyle || '#00ff00';

    context.beginPath();
    context.moveTo(visibleFieldPoly[0][0], visibleFieldPoly[0][1]);
    var len = visibleFieldPoly.length === 4 ? 4 : 2;
    for (var i = 1; i < len; i++)
    {
        context.lineTo(visibleFieldPoly[i][0], visibleFieldPoly[i][1]);
    }
    context.closePath();
    context.stroke();
}

export function drawBall(context: CanvasRenderingContext2D, options, position: number[])
{
    context.fillStyle = options.ballFillStyle || 'orange';

    context.beginPath();
    context.arc(position[0], position[1], constants.ballRadius, 0, Math.PI*2, true);
    context.fill();
}

export function drawAgentPosition(context: CanvasRenderingContext2D, options, agentPosition: number[])
{
    var agentDotRadius = options.agentDotRadius || 0.1,
        agentDirectionLength = options.agentDirectionLength || 0.2,
        heading = agentPosition[2] + Math.PI / 2;

    context.strokeStyle = options.agentPosStyle || 'red';
    context.fillStyle = options.agentPosStyle || 'red';

    context.beginPath();
    context.arc(agentPosition[0], agentPosition[1], agentDotRadius, 0, Math.PI*2, true);
    context.fill();
    context.beginPath();
    context.moveTo(agentPosition[0], agentPosition[1]);
    context.lineTo(agentPosition[0] + agentDirectionLength * Math.cos(heading),
                   agentPosition[1] + agentDirectionLength * Math.sin(heading));
    context.stroke();
}

export function drawParticles(context: CanvasRenderingContext2D, options, particles: number[][])
{
    var size = options.particleSize || 0.01;

    context.beginPath();
    _.each(particles, particle =>
    {
        var opacity =  Math.min(1, (particle[3] / 0.03) + 0.5);
        context.fillStyle = 'hsla(' + options.particleHue + ', 100%, 50%, ' + opacity + ')';
        var x = particle[0] - size/2,
            y = particle[1] - size/2;
        context.fillRect(x, y, size, size);
    });
}

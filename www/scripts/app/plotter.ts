/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />

import color = require('color');
import control = require('control');
import constants = require('constants');
import geometry = require('util/geometry');
import state = require('state');

export function drawField(context: CanvasRenderingContext2D, options: {groundFillStyle?: string})
{
    context.save();
    context.setTransform(1, 0, 0, 1, 0, 0);
    context.fillStyle = options.groundFillStyle || '#008800';
    context.fillRect(0, 0, context.canvas.width, context.canvas.height);
    context.restore();
}

export function drawFieldLines(context: CanvasRenderingContext2D, options: {lineStrokeStyle?: string})
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

export function drawGoalPosts(context: CanvasRenderingContext2D, options: {goalStrokeStyle?: string}, positions: number[][])
{
    context.fillStyle = options.goalStrokeStyle || 'yellow';

    _.each(positions, pos =>
    {
        context.beginPath();
        context.arc(pos[0], pos[1], constants.goalPostDiameter/2, 0, Math.PI*2, true);
        context.fill();
    });
}

export function drawOcclusionRays(context: CanvasRenderingContext2D,
                                  options: {
                                      lineWidth?: number;
                                      occlusionRayFillStyle?: string;
                                      occlusionEdgeStrokeStyle?: string;
                                      fieldEdgeStrokeStyle?: string
                                  },
                                  rays: number[][])
{
    context.lineWidth = options.lineWidth || 1.0;

    context.fillStyle = options.occlusionRayFillStyle || 'rgba(0,0,0,0.15)';
    context.beginPath();
    for (var i = 0; i < rays.length; i++)
        context.lineTo(rays[i][0], rays[i][1]);
    for (var i = rays.length - 1; i >= 0; i--)
        context.lineTo(rays[i][2], rays[i][3]);
    context.fill();

    context.strokeStyle = options.occlusionEdgeStrokeStyle || 'rgba(0,0,0,0.5)';
    context.beginPath();
    for (var i = 0; i < rays.length; i++)
        context.lineTo(rays[i][0], rays[i][1]);
    context.stroke();

    context.strokeStyle = options.fieldEdgeStrokeStyle || 'rgba(100,255,100,1)';
    context.beginPath();
    for (var i = rays.length - 1; i >= 0; i--)
        context.lineTo(rays[i][2], rays[i][3]);
    context.stroke();
}

export function drawGoals(context: CanvasRenderingContext2D, options: {goalStrokeStyle?: string})
{
    var goalY = (constants.goalY + constants.goalPostDiameter) / 2,
        x = constants.fieldX/2 + constants.goalPostDiameter/2 - constants.lineWidth/2;

    drawGoalPosts(context, options, [
        [ x,  goalY],
        [ x, -goalY],
        [-x,  goalY],
        [-x, -goalY]
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

export function drawLineSegments(context: CanvasRenderingContext2D, lineSegments: number[][], lineWidth, strokeStyle)
{
    context.lineWidth = lineWidth || 0.01;
    context.strokeStyle = strokeStyle || '#0000ff';

    context.beginPath();
    _.each(lineSegments, lineSegment =>
    {
        // TODO change line segment data to be in 2D as z is always zero
        context.moveTo(lineSegment[0], lineSegment[1]);
        context.lineTo(lineSegment[3], lineSegment[4]);
    });
    context.stroke();
}

export function drawJunctions(context: CanvasRenderingContext2D, junctions: {p: number[]; a: number; t: number}[])
{
    context.lineWidth = 0.03;
    context.strokeStyle = '#800080';
    var markerRadius = 0.1;
    _.each(junctions, junction =>
           {
               context.beginPath();
               context.arc(junction.p[0], junction.p[1], markerRadius, 0, 2 * Math.PI);
               if (junction.t == 0) { // X
                   context.moveTo(junction.p[0], junction.p[1] + markerRadius);
                   context.lineTo(junction.p[0], junction.p[1] - markerRadius);
                   context.moveTo(junction.p[0] - markerRadius, junction.p[1]);
                   context.lineTo(junction.p[0] + markerRadius, junction.p[1]);
               } else if (junction.t == 1) { // T
                   context.moveTo(junction.p[0] - markerRadius, junction.p[1]);
                   context.lineTo(junction.p[0] + markerRadius, junction.p[1]);
                   context.moveTo(junction.p[0], junction.p[1]);
                   context.lineTo(junction.p[0], junction.p[1] - markerRadius);
               } else if (junction.t == 2) { // L
                   context.moveTo(junction.p[0], junction.p[1]);
                   context.lineTo(junction.p[0] + markerRadius, junction.p[1]);
                   context.moveTo(junction.p[0], junction.p[1]);
                   context.lineTo(junction.p[0], junction.p[1] + markerRadius);
               }
               context.stroke();
           });
}

export function drawVisibleFieldPoly(context: CanvasRenderingContext2D, options: {visibleFieldPolyLineWidth?:number; visibleFieldPolyStrokeStyle?: string}, visibleFieldPoly: number[][])
{
    if (visibleFieldPoly.length < 2)
        return;

    context.lineWidth = options.visibleFieldPolyLineWidth || 0.01;
    context.strokeStyle = options.visibleFieldPolyStrokeStyle || '#0000ff';

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

export function drawBall(context: CanvasRenderingContext2D, options: {ballFillStyle?: string}, position: number[])
{
    context.fillStyle = options.ballFillStyle || 'red';

    context.beginPath();
    context.arc(position[0], position[1], constants.ballRadius, 0, Math.PI*2, true);
    context.fill();
}

export function drawAgentPosition(context: CanvasRenderingContext2D, options: {agentDotRadius?: number; agentDirectionLength?: number; agentPosStyle?: string}, agentPosition: number[])
{
    var agentDotRadius = options.agentDotRadius || 0.1,
        agentDirectionLength = options.agentDirectionLength || 0.2,
        heading = agentPosition[2] + Math.PI / 2;

    context.strokeStyle = options.agentPosStyle || 'red';
    context.fillStyle = options.agentPosStyle || 'red';
    context.lineWidth = agentDotRadius / 5;

    context.beginPath();
    context.arc(agentPosition[0], agentPosition[1], agentDotRadius, 0, Math.PI*2, true);
    context.fill();
    context.beginPath();
    context.moveTo(agentPosition[0], agentPosition[1]);
    context.lineTo(agentPosition[0] + agentDirectionLength * Math.cos(heading),
                   agentPosition[1] + agentDirectionLength * Math.sin(heading));
    context.stroke();
}

export function drawParticles(context: CanvasRenderingContext2D, options: {particleSize?:number; particleHue:number}, particles: number[][])
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

export function drawTeammates(context: CanvasRenderingContext2D, players: state.PlayerData[], scale: number)
{
    var agentPosOptions = {};

    // TODO draw other teammate data here, such as the ball position, role, action, robotId...

    _.each<state.PlayerData>(players, (player: state.PlayerData) =>
    {
        if (!player.isMe)
            drawAgentPosition(context, agentPosOptions, player.pos);
    });
}

export function drawStationaryMap(context: CanvasRenderingContext2D, data: state.StationaryMap)
{
    context.lineWidth = 0.01;

    var maxBallCount = 0,
        maxGoalPostCount = 0;

    _.each(data.balls, ball => maxBallCount = Math.max(maxBallCount, ball.count));
    _.each(data.goalPosts, goalPost => maxGoalPostCount = Math.max(maxGoalPostCount, goalPost.count));

    var maxScore = 0.0,
        maxBall: state.AveragePosition;
    _.each(data.balls, ball =>
    {
        if (ball.count > maxScore)
        {
            maxScore = ball.count;
            maxBall = ball;
        }
        context.strokeStyle = 'rgba(255,0,0,' + (ball.count / maxBallCount) + ')';
        context.beginPath();
        context.arc(ball.pos[0], ball.pos[1], constants.ballRadius, 0, Math.PI*2, true);
        context.stroke();
    });

    _.each(data.goalPosts, goalPost =>
    {
        var baseColor: color.Rgb;

        switch (goalPost.label)
        {
            case state.GoalLabel.Ours:    baseColor = new color.Rgb(1, 0.8, 0); break;
            case state.GoalLabel.Theirs:  baseColor = new color.Rgb(0.8, 1, 0); break;
            case state.GoalLabel.Unknown: baseColor = new color.Rgb(0.7, 0.7, 0); break;
        }

        var alpha = goalPost.count / maxGoalPostCount;
        context.strokeStyle = baseColor.toString(alpha);
        context.beginPath();
        context.arc(goalPost.pos[0], goalPost.pos[1], constants.goalPostDiameter/2, 0, Math.PI*2, true);
        context.stroke();
    });

    var startPos = maxBall
        ? maxBall.pos
        : [0, 0.12];

    _.each(data.kicks, kick =>
    {
        context.strokeStyle = 'purple';
        context.beginPath();
        context.moveTo(startPos[0], startPos[1]);
        context.lineTo(kick.endPos[0], kick.endPos[1]);
        context.stroke();
    });
}

/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/Constants'
    ],
    function (Constants)
    {
        //noinspection UnnecessaryLocalVariableJS
        var FieldLinePlotter = {
            start: function(context, options)
            {
                var canvas = context.canvas,
                    fieldCenter = options.fieldCenter || { x: canvas.width / 2, y: canvas.height / 2 };

                context.fillStyle = options.groundFillStyle || '#008800';
                context.fillRect(0, 0, canvas.width, canvas.height);

                context.save();
                context.translate(fieldCenter.x, fieldCenter.y);
            },
            end: function(context)
            {
                context.restore();
            },
            drawFieldLines: function (context, options)
            {
                var scale = options.scale || 1;

                // prepare to draw field lines
                context.lineWidth = Constants.lineWidth * scale;
                context.strokeStyle = options.lineStrokeStyle || '#ffffff';

                // center circle
                context.beginPath();
                context.arc(0, 0, scale * Constants.circleDiameter/2, 0, Math.PI*2, true);

                var halfCrossLengthScaled = scale * Constants.penaltyLineLength / 2;
                var penaltyX = scale * (Constants.fieldX/2 - Constants.penaltyMarkDistance);
                var penaltyInnerX = penaltyX - halfCrossLengthScaled;
                var penaltyOuterX = penaltyX + halfCrossLengthScaled;

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
                var x = scale * Constants.fieldX/2,
                    y = scale * Constants.fieldY/2;
                context.strokeRect(-x, -y, scale * Constants.fieldX, scale * Constants.fieldY);

                context.moveTo(0, y);
                context.lineTo(0, -y);

                var goalAreaY = scale * Constants.goalAreaY / 2;

                // left goal area
                context.moveTo(-x, -goalAreaY);
                context.lineTo(-x + scale*Constants.goalAreaX, -goalAreaY);
                context.lineTo(-x + scale*Constants.goalAreaX, goalAreaY);
                context.lineTo(-x, goalAreaY);

                // right goal area
                context.moveTo(x, -goalAreaY);
                context.lineTo(x - scale*Constants.goalAreaX, -goalAreaY);
                context.lineTo(x - scale*Constants.goalAreaX, goalAreaY);
                context.lineTo(x, goalAreaY);

                context.stroke();
            },
            drawGoals: function(context, options)
            {
                var scale = options.scale || 1,
                    goalY = scale * Constants.goalY / 2,
                    x = scale * Constants.fieldX/2;

                // TODO the position of these circles is slightly wrong, as the perimeter should line up with the edge of the line

                context.strokeStyle = options.goalStrokeStyle || 'yellow';

                context.beginPath();
                context.arc(+x, +goalY, scale * Constants.goalPostDiameter/2, 0, Math.PI*2, true);
                context.stroke();
                context.beginPath();
                context.arc(+x, -goalY, scale * Constants.goalPostDiameter/2, 0, Math.PI*2, true);
                context.stroke();
                context.beginPath();
                context.arc(-x, +goalY, scale * Constants.goalPostDiameter/2, 0, Math.PI*2, true);
                context.stroke();
                context.beginPath();
                context.arc(-x, -goalY, scale * Constants.goalPostDiameter/2, 0, Math.PI*2, true);
                context.stroke();

                context.beginPath();

                // left goal
                context.moveTo(-x, -goalY);
                context.lineTo(-x - scale*Constants.goalX, -goalY);
                context.lineTo(-x - scale*Constants.goalX, goalY);
                context.lineTo(-x, goalY);

                // right goal
                context.moveTo(x, -goalY);
                context.lineTo(x + scale*Constants.goalX, -goalY);
                context.lineTo(x + scale*Constants.goalX, goalY);
                context.lineTo(x, goalY);

                context.stroke();
            },
            drawLineSegments: function (context, options, lineSegments, lineWidth, strokeStyle)
            {
                var scale = options.scale || 1;

                context.lineWidth = lineWidth || 1;
                context.strokeStyle = strokeStyle || '#00ff00';

                context.beginPath();
                _.each(lineSegments, function (lineSegment)
                {
                    var p1 = lineSegment.p1,
                        p2 = lineSegment.p2;
                    context.moveTo(p1.x * scale, -p1.y * scale);
                    context.lineTo(p2.x * scale, -p2.y * scale);
                });
                context.stroke();
            },
            drawBall: function(context, options, position)
            {
                var scale = options.scale || 1;

                context.fillStyle = options.ballFillStyle || 'orange';

                context.beginPath();
                context.arc(position[0] * scale, -position[1] * scale, Constants.ballRadius * scale, 0, Math.PI*2, true);
                context.fill();
            },
            drawAgentPosition: function(context, options, agentPosition)
            {
                var scale = options.scale || 1,
                    agentDotRadius = options.agentDotRadius || 3,
                    agentDirectionLength = options.agentDirectionLength || 15,
                    heading = agentPosition[2] + Math.PI / 2;

                context.strokeStyle = options.agentPosStyle || 'red';
                context.fillStyle = options.agentPosStyle || 'red';

                context.beginPath();
                context.arc(agentPosition[0] * scale, -agentPosition[1] * scale, agentDotRadius, 0, Math.PI*2, true);
                context.fill();
                context.beginPath();
                context.moveTo(agentPosition[0] * scale, -agentPosition[1] * scale);
                context.lineTo(agentPosition[0] * scale + agentDirectionLength * Math.cos(heading),
                              -agentPosition[1] * scale - agentDirectionLength * Math.sin(heading));
                context.stroke();
            },
            drawParticles: function(context, options, particles)
            {
                var scale = options.scale || 1,
                    size = options.particleSize || 1;

                context.beginPath();
                _.each(particles, function (particle)
                {
                    context.fillStyle = particle[3] === 0 ? 'black' : options.particleStyle || 'cyan';
                    var x = Math.round( particle[0] * scale - size/2),
                        y = Math.round(-particle[1] * scale - size/2);
                    context.fillRect(x, y, size, size);
                });
            }
        };

        return FieldLinePlotter;
    }
);
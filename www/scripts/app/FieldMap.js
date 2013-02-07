define(
    [
        'scripts/app/Constants'
    ],
    function(Constants)
    {
        var FieldMap = function(canvas)
        {
            this.canvas = canvas;
            var $canvas = $(canvas);

            this.fieldCenterX = canvas.width/2;
            this.fieldCenterY = canvas.height/2;
            this.scale = Math.min(
                canvas.width / (Constants.fieldX + 2*Constants.outerMarginMinimum),
                canvas.height / (Constants.fieldY + 2*Constants.outerMarginMinimum));
            this.minScale = this.scale * 0.8; // can't zoom out too far

            var self = this;
            $canvas.on('mousewheel', function(event)
            {
                // TODO zoom around the mouse pointer, rather than (0,0)
                self.scale += event.originalEvent.wheelDelta / 20;
                self.scale = Math.max(self.minScale, self.scale);
                self.draw();
            });

            var isMouseDown = false, dragStartX, dragStartY;
            $canvas.on('mousedown', function(event)
            {
                isMouseDown = true;
                dragStartX = event.screenX;
                dragStartY = event.screenY;
            });

            $canvas.on('mouseup', function() {
                isMouseDown = false;
            });

            $canvas.on('mousemove', function(event) {
                if (isMouseDown) {
                    var dx = event.screenX - dragStartX,
                        dy = event.screenY - dragStartY;
                    dragStartX = event.screenX;
                    dragStartY = event.screenY;
                    self.fieldCenterX += dx;
                    self.fieldCenterY += dy;
                    self.draw();
                }
            });

            this.draw();
        };

        FieldMap.prototype.draw = function()
        {
            var context = this.canvas.getContext('2d');

            context.save();

            context.fillStyle = '#008800';
            context.fillRect(0, 0, this.canvas.width, this.canvas.height);

            context.translate(this.fieldCenterX, this.fieldCenterY);

            // prepare to draw field lines
            context.lineWidth = Constants.lineWidth * this.scale;
            context.strokeStyle = '#ffffff';

            // center circle
            context.beginPath();
            context.arc(0, 0, this.scale * Constants.circleDiameter/2, 0, Math.PI*2, true);

            var halfCrossLengthScaled = this.scale * Constants.penaltyLineLength / 2;
            var penaltyX = this.scale * (Constants.fieldX/2 - Constants.penaltyMarkDistance);
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
            var x = this.scale * Constants.fieldX/2,
                y = this.scale * Constants.fieldY/2;
            context.strokeRect(-x, -y, this.scale * Constants.fieldX, this.scale * Constants.fieldY);

            context.moveTo(0, y);
            context.lineTo(0, -y);

            var goalAreaY = this.scale * Constants.goalAreaY / 2;

            // left goal area
            context.moveTo(-x, -goalAreaY);
            context.lineTo(-x + this.scale*Constants.goalAreaX, -goalAreaY);
            context.lineTo(-x + this.scale*Constants.goalAreaX, goalAreaY);
            context.lineTo(-x, goalAreaY);

            // right goal area
            context.moveTo(x, -goalAreaY);
            context.lineTo(x - this.scale*Constants.goalAreaX, -goalAreaY);
            context.lineTo(x - this.scale*Constants.goalAreaX, goalAreaY);
            context.lineTo(x, goalAreaY);

            context.stroke();

            var goalY = this.scale * Constants.goalY / 2;

            // TODO actually the position of these circles is WRONG! as is many of the lines -- the insides should be used, considering line width

            context.strokeStyle = 'yellow';

            context.beginPath();
            context.arc(+x, +goalY, this.scale * Constants.goalPostDiameter/2, 0, Math.PI*2, true);
            context.stroke();
            context.beginPath();
            context.arc(+x, -goalY, this.scale * Constants.goalPostDiameter/2, 0, Math.PI*2, true);
            context.stroke();
            context.beginPath();
            context.arc(-x, +goalY, this.scale * Constants.goalPostDiameter/2, 0, Math.PI*2, true);
            context.stroke();
            context.beginPath();
            context.arc(-x, -goalY, this.scale * Constants.goalPostDiameter/2, 0, Math.PI*2, true);
            context.stroke();

            context.beginPath();

            // left goal
            context.moveTo(-x, -goalY);
            context.lineTo(-x - this.scale*Constants.goalX, -goalY);
            context.lineTo(-x - this.scale*Constants.goalX, goalY);
            context.lineTo(-x, goalY);

            // right goal
            context.moveTo(x, -goalY);
            context.lineTo(x + this.scale*Constants.goalX, -goalY);
            context.lineTo(x + this.scale*Constants.goalX, goalY);
            context.lineTo(x, goalY);

            context.stroke();

            context.restore();
        };

        return FieldMap;
    }
);
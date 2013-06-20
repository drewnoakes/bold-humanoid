/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlClient',
        'util/Colour',
        'util/CanvasExtensions'
    ],
    function(ControlClient, Colour)
    {
        var PixelLabelInspector = function(width, height)
        {
            this.canvas = document.createElement('canvas');
            this.canvas.className = 'pixel-label-inspector';
            this.canvas.width = width;
            this.canvas.height = height;
            this.context = this.canvas.getContext('2d');

            this.labels = {};
            this.ready = false;

            this.labels.ball  = {hue:{}, saturation:{}, value:{}, colour:'red'};
            this.labels.line  = {hue:{}, saturation:{}, value:{}, colour:'white'};
            this.labels.goal  = {hue:{}, saturation:{}, value:{}, colour:'yellow'};
            this.labels.field = {hue:{}, saturation:{}, value:{}, colour:'green'};

            ControlClient.withData('vision/lut', function(controls)
            {
                console.log('callback!');
                _.each(controls, function (control)
                {
                    switch (control.name)
                    {
                        case 'Ball Hue':              this.labels.ball.hue.mid = control.value; break;
                        case 'Ball Hue Range':        this.labels.ball.hue.range = control.value; break;
                        case 'Ball Saturation':       this.labels.ball.saturation.mid = control.value; break;
                        case 'Ball Saturation Range': this.labels.ball.saturation.range = control.value; break;
                        case 'Ball Value':            this.labels.ball.value.mid = control.value; break;
                        case 'Ball Value Range':      this.labels.ball.value.range = control.value; break;

                        case 'Line Hue':              this.labels.line.hue.mid = control.value; break;
                        case 'Line Hue Range':        this.labels.line.hue.range = control.value; break;
                        case 'Line Saturation':       this.labels.line.saturation.mid = control.value; break;
                        case 'Line Saturation Range': this.labels.line.saturation.range = control.value; break;
                        case 'Line Value':            this.labels.line.value.mid = control.value; break;
                        case 'Line Value Range':      this.labels.line.value.range = control.value; break;

                        case 'Goal Hue':              this.labels.goal.hue.mid = control.value; break;
                        case 'Goal Hue Range':        this.labels.goal.hue.range = control.value; break;
                        case 'Goal Saturation':       this.labels.goal.saturation.mid = control.value; break;
                        case 'Goal Saturation Range': this.labels.goal.saturation.range = control.value; break;
                        case 'Goal Value':            this.labels.goal.value.mid = control.value; break;
                        case 'Goal Value Range':      this.labels.goal.value.range = control.value; break;

                        case 'Field Hue':             this.labels.field.hue.mid = control.value; break;
                        case 'Field Hue Range':       this.labels.field.hue.range = control.value; break;
                        case 'Field Saturation':      this.labels.field.saturation.mid = control.value; break;
                        case 'Field Saturation Range':this.labels.field.saturation.range = control.value; break;
                        case 'Field Value':           this.labels.field.value.mid = control.value; break;
                        case 'Field Value Range':     this.labels.field.value.range = control.value; break;
                    }
                }.bind(this));

                this.ready = true;

            }.bind(this));
        };

        PixelLabelInspector.prototype.highlightHsv = function(hsv)
        {
            this.hsv = hsv;
            this.draw();
        };

        PixelLabelInspector.prototype.draw = function()
        {
            var context = this.context;

            context.fillStyle = '#000';
            context.clear();

            context.beginPath();

            var gutterWidth = 25,
                barHeight = 4,
                barSpacing = 2,
                componentSpacing = 2,
                components = ['hue', 'saturation', 'value'],
                space = this.canvas.width - gutterWidth;

            // TODO deal with the fact that hue wraps around

            var y = 0;
            for (var component = 0; component < 3; component++)
            {
                if (component != 0)
                {
                    // Draw horizontal separator line
                    context.lineWidth = 1;
                    context.strokeStyle = 'rgba(0,0,0,0.3)';
                    context.dashedLine(0, Math.floor(y) + 0.5, this.canvas.width, y, [1,1]);
                    context.stroke();
                    y++;
                }

                // Text label
                context.fillStyle = 'black';
                context.fillText('HSV'[component], 8, y + 16);

                var startY = y;

                y += componentSpacing;

                _.each(_.values(this.labels), function (label)
                {
                    var labelData = label[components[component]],
                        min = Math.max(0,   labelData.mid - (labelData.range/2)),
                        max = Math.min(255, labelData.mid + (labelData.range/2)),
                        minRatio = min/255,
                        maxRatio = max/255,
                        minX = minRatio*space,
                        maxX = maxRatio*space;

                    context.fillStyle = label.colour;
                    context.fillRect(gutterWidth + minX, y, maxX - minX, barHeight);

                    y += barHeight + barSpacing;

                }.bind(this));

                y += componentSpacing;

                if (this.hsv)
                {
                    var hoverRatio = this.hsv['hsv'[component]],
                        hoverX = Math.floor(gutterWidth + hoverRatio*space) + 0.5;

                    context.strokeStyle = 'rgba(0,0,0,0.7)';
                    context.moveTo(hoverX, startY);
                    context.lineTo(hoverX, y);
                    context.stroke();
                }
            }
        };

        PixelLabelInspector.prototype.setVisible = function(visible)
        {
            this.canvas.style.display = visible ? 'block' : 'none';
        };

        return PixelLabelInspector;
    }
);
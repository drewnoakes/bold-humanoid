/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'constants',
        'BodyFigure'
    ],
    function (DataProxy, constants, BodyFigure)
    {
        'use strict';

        var chartHeight = 300,
            chartWidth = 430,
            maxDataLength = chartWidth;

        var TrajectoryModule = function ()
        {
            this.container = $('<div></div>');
            this.title = 'trajectory';
            this.id = 'trajectory';
            this.element = this.container.get(0);
        };

        TrajectoryModule.prototype.load = function ()
        {
            this.mirrorValues = true;
            this.isRecording = false;
            this.skipFirstDatum = true;
            this.hoverJointId = -1;
            this.data = [];

            this.recordButton = document.createElement('button');
            this.recordButton.className = 'record';
            this.recordButton.textContent = 'record';
            this.recordButton.addEventListener('click', this.toggleIsRecording.bind(this));
            var mirrorCheckbox = document.createElement('input');
            mirrorCheckbox.id = 'mirror-checkbox';
            mirrorCheckbox.type = 'checkbox';
            mirrorCheckbox.checked = true;
            mirrorCheckbox.addEventListener('change', function ()
            {
                this.mirrorValues = mirrorCheckbox.checked;
                this.render();
            }.bind(this));
            var mirrorLabel = document.createElement('label');
            mirrorLabel.htmlFor = mirrorCheckbox.id;
            mirrorLabel.textContent = 'Mirror values';
            var controlContainer = document.createElement('div');
            controlContainer.className = 'controls';
            controlContainer.appendChild(this.recordButton);
            controlContainer.appendChild(mirrorCheckbox);
            controlContainer.appendChild(mirrorLabel);
            this.container.append(controlContainer);

            this.canvas = document.createElement('canvas');
            this.canvas.width = chartWidth;
            this.canvas.height = chartHeight;
            this.container.append(this.canvas);

            this.bodyFigure = new BodyFigure({hasHover: true, hasSelection: true});
            this.container.append(this.bodyFigure.element);

            this.bodyFigure.hoverJointId.track(this.render.bind(this));
            this.bodyFigure.selectedJointIds.track(this.render.bind(this));
            this.bodyFigure.visitJoints(function(jointId, jointDiv) { jointDiv.textContent = jointId; });
        };

        TrajectoryModule.prototype.unload = function ()
        {
            this.container.empty();

            if (this.subscription)
            {
                this.subscription.close();
                delete this.subscription;
            }
            
            delete this.canvas;
            delete this.bodyFigure;
            delete this.data;
            delete this.recordButton;
        };

        /*
           {
             cycle: 1234,
             joints: [
               { v: 4096, p: 32, i: 0, d: 0 }
             ]
           }
         */
        TrajectoryModule.prototype.onData = function (data)
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
        };

        TrajectoryModule.prototype.toggleIsRecording = function()
        {
            this.isRecording = !this.isRecording;

            if (this.isRecording) {
                this.canvas.getContext('2d').clear();
                this.recordButton.classList.add('recording');
                this.recordButton.textContent = 'recording...';
                this.subscription = DataProxy.subscribe(
                    constants.protocols.bodyControlState,
                    {
                        json: true,
                        onmessage: _.bind(this.onData, this)
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
        };

        TrajectoryModule.prototype.render = function ()
        {
            if (this.isRecording || this.data.length === 0)
                return;

            var ctx = this.canvas.getContext('2d');

            var x = d3.scale.linear()
                .range([0, chartWidth])
                .domain([this.data[0].cycle, this.data[this.data.length - 1].cycle]);

            var y = d3.scale.linear()
                .range([0, chartHeight])
                .domain([4096, 0]);

            var yMirror = d3.scale.linear()
                .range([0, chartHeight])
                .domain([0, 4096]);

            ctx.clear();

            var selectedJointIds = this.bodyFigure.selectedJointIds.getValue(),
                hoverJointId = this.bodyFigure.hoverJointId.getValue();

            var jointIds = selectedJointIds && selectedJointIds.length ? selectedJointIds : d3.range(1, 21);

            // Draw tick markers to show when samples were taken
            ctx.strokeStyle = '#888';
            _.each(this.data, function (d)
            {
                var px = Math.round(x(d.cycle) - 0.5) + 0.5;
                ctx.moveTo(px, chartHeight);
                ctx.lineTo(px, chartHeight - 5);
            });
            ctx.stroke();

            var drawLine = function (jointId)
            {
                jointId--;
                ctx.beginPath();
                _.each(this.data, function (d)
                {
                    var yScale = this.mirrorValues && jointId % 2 === 0 && jointId !== 20 ? yMirror : y;
                    var px = x(d.cycle),
                        py = yScale(d.joints[jointId].v);
                    ctx.lineTo(px, py);
                }.bind(this));
                ctx.stroke();
            }.bind(this);

            ctx.strokeStyle = '#792485';
            _.each(jointIds, function(jointId)
            {
                if (hoverJointId === jointId)
                    return;
                drawLine(jointId);
            }.bind(this));

            if (hoverJointId > 0) {
                ctx.strokeStyle = '#FFF';
                drawLine(hoverJointId);
            }
        };

        return TrajectoryModule;
    }
);

/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'Constants',
        'Protocols',
        'BodyFigure'
    ],
    function (DataProxy, Constants, Protocols, BodyFigure)
    {
        'use strict';

        var chartHeight = 300,
            chartWidth = 430,
            maxDataLength = chartWidth;

        var TrajectoryModule = function ()
        {
            this.isRecording = false;
            this.skipFirstDatum = true;
            this.hoverJointId = -1;
            this.data = [];
            this.container = $('<div></div>');
            this.title = 'trajectory';
            this.id = 'trajectory';
            this.panes = [
                {
                    title: 'main',
                    element: this.container,
                    supports: { fullScreen: false }
                }
            ];
        };

        TrajectoryModule.prototype.load = function ()
        {
            this.recordButton = document.createElement('button');
            this.recordButton.className = 'record';
            this.recordButton.textContent = 'record';
            this.recordButton.addEventListener('click', this.toggleIsRecording.bind(this));
            var controlContainer = document.createElement('div');
            controlContainer.className = 'controls';
            controlContainer.appendChild(this.recordButton);
            this.container.append(controlContainer);

            this.canvas = document.createElement('canvas');
            this.canvas.width = chartWidth;
            this.canvas.height = chartHeight;
            this.container.append(this.canvas);

            this.bodyFigure = new BodyFigure();
            this.container.append(this.bodyFigure.element);

            this.selectedJointIds = [];

            var jointDivs = this.bodyFigure.element.querySelectorAll('div.joint');
            _.each(jointDivs, function(jointDiv)
            {
                var jointId = parseInt(jointDiv.dataset.jointId);
                jointDiv.textContent = jointId;
                jointDiv.addEventListener('click', function()
                {
                    if (jointDiv.classList.contains('selected')) {
                        jointDiv.classList.remove('selected');
                        this.selectedJointIds = _.filter(this.selectedJointIds, function(d) { return d !== jointId; });
                    } else {
                        jointDiv.classList.add('selected');
                        this.selectedJointIds.push(jointId);
                    }
                    this.render();
                }.bind(this));
                jointDiv.addEventListener('mouseenter', function() { this.hoverJointId = jointId; this.render(); }.bind(this));
                jointDiv.addEventListener('mouseleave', function() { this.hoverJointId = -1; this.render(); }.bind(this));
            }.bind(this));
        };

        TrajectoryModule.prototype.unload = function ()
        {
            this.container.empty();

            if(this.subscription)
            {
                this.subscription.close();
                delete this.subscription;
            }
            
            delete this.canvas;
            delete this.bodyFigure;
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
                    Protocols.bodyControlState,
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

            ctx.clear();

            var jointIds = this.selectedJointIds.length ? this.selectedJointIds : d3.range(1, 21);

            var drawLine = function (jointId)
            {
                jointId--;
                ctx.beginPath();
                _.each(this.data, function (d)
                {
                    var px = x(d.cycle),
                        py = y(d.joints[jointId].v);
                    ctx.lineTo(px, py);
                });
                ctx.stroke();
            }.bind(this);

            ctx.strokeStyle = '#792485';
            _.each(jointIds, function(jointId)
            {
                if (this.hoverJointId === jointId)
                    return;
                drawLine(jointId);
            }.bind(this));

            if (this.hoverJointId > 0) {
                ctx.strokeStyle = '#FFF';
                drawLine(this.hoverJointId);
            }
        };

        return TrajectoryModule;
    }
);

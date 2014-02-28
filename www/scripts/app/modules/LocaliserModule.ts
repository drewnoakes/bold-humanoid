/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import constants = require('constants');
import data = require('data');
import control = require('control');
import state = require('state');
import Module = require('Module');

var chartOptions = {
    grid: {
        strokeStyle: 'rgb(40, 40, 40)',
        fillStyle: 'rgb(0, 0, 0)',
        lineWidth: 1,
        millisPerLine: 250,
        verticalSections: 6,
        sharpLines: true,
        borderVisible:  false
    },
    minValue: 0,
    labels: {
        fillStyle: '#ffffff'
    }
};

var chartHeight = 150;

class LocaliserModule extends Module
{
    private chartCanvas: HTMLCanvasElement;
    private chart: SmoothieChart;
    private averageSeries: TimeSeries;
    private maxSeries: TimeSeries;

    private ballVisibleMarker: HTMLDivElement;
    private goal1VisibleMarker: HTMLDivElement;
    private goal2VisibleMarker: HTMLDivElement;

    constructor()
    {
        super('localiser', 'localiser');
    }

    public load(element: HTMLDivElement)
    {
        this.chart = new SmoothieChart(chartOptions);

        this.chartCanvas = document.createElement('canvas');
        this.chartCanvas.width = 640;
        this.chartCanvas.height = chartHeight;
        element.appendChild(this.chartCanvas);

        this.chart.streamTo(this.chartCanvas, /*delayMs*/ 200);

        this.averageSeries = new TimeSeries();
        this.maxSeries = new TimeSeries();

        this.chart.addTimeSeries(this.averageSeries, {lineWidth:1, strokeStyle:'#0040ff', fillStyle:'rgba(0,64,255,0.26)'});
        this.chart.addTimeSeries(this.maxSeries, {lineWidth:1, strokeStyle:'#00ff00'});

        this.closeables.add(new data.Subscription<state.Particle>(
            constants.protocols.particleState,
            {
                onmessage: this.onParticleData.bind(this)
            }
        ));

        this.closeables.add(new data.Subscription<state.CameraFrame>(
            constants.protocols.cameraFrameState,
            {
                onmessage: this.onCameraData.bind(this)
            }
        ));

        var controls = document.createElement('div');
        controls.className = 'control-container localiser-controls flow';
        element.appendChild(controls);
        control.buildActions('localiser', controls);
        control.buildSettings('localiser', controls, this.closeables);

        // TODO show the number of lines visible
        // TODO show the % of the camera frame which is within the field

        var createMarker = type => {
            var marker = document.createElement('div');
            marker.className = 'marker ' + type;
            element.appendChild(marker);
            return marker;
        };

        this.ballVisibleMarker = createMarker('ball');
        this.goal1VisibleMarker = createMarker('goal');
        this.goal2VisibleMarker = createMarker('goal');
    }

    public unload()
    {
        this.chart.stop();
    }

    public onResized(width, height)
    {
        this.chartCanvas.width = width;
    }

    private onCameraData(data: state.CameraFrame)
    {
        if (data.ball) {
            this.ballVisibleMarker.classList.add('visible');
        } else {
            this.ballVisibleMarker.classList.remove('visible');
        }

        switch (data.goals.length) {
            case 0: {
                this.goal1VisibleMarker.classList.remove('visible');
                this.goal2VisibleMarker.classList.remove('visible');
                break;
            }
            case 1: {
                this.goal1VisibleMarker.classList.add('visible');
                this.goal2VisibleMarker.classList.remove('visible');
                break;
            }
            default: {
                this.goal1VisibleMarker.classList.add('visible');
                this.goal2VisibleMarker.classList.add('visible');
                break;
            }
        }
    }

    private onParticleData(data: state.Particle)
    {
        var time = new Date().getTime(),
            particles = data.particles;

        var count = 0, sum = 0, max = 0;
        for (var i = 0; i < particles.length; i++)
        {
            var w = particles[i][3];
            if (w !== 0)
            {
                count++;
                sum += w;
                if (max < w)
                    max = w;
            }
        }

        if (count !== 0)
        {
            this.averageSeries.append(time, sum/count);
            this.maxSeries.append(time, max);
        }
    }
}

export = LocaliserModule;

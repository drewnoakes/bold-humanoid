/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import constants = require('constants');
import control = require('control');
import data = require('data');
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
        borderVisible: false
    },
    labels: {
        fillStyle: '#ffffff'
    },
    minValue: 0
};

var chartHeight = 150;

class CommsModule extends Module
{
    private canvas: HTMLCanvasElement;
    private chart: SmoothieChart;
    private ignoreSeries: TimeSeries;
    private gameSeries: TimeSeries;

    constructor()
    {
        super('comms', 'communication', {fullScreen: true});
    }

    public load()
    {
        var controls = document.createElement('div');
        control.buildSettings("game-controller", controls, this.closeables);
        this.element.appendChild(controls);

        this.chart = new SmoothieChart(chartOptions);
        this.canvas = document.createElement('canvas');
        this.canvas.height = chartHeight;
        this.element.appendChild(this.canvas);

        this.ignoreSeries = new TimeSeries();
        this.gameSeries = new TimeSeries();
        this.chart.addTimeSeries(this.ignoreSeries, { strokeStyle: 'rgb(128, 0, 0)', fillStyle: 'rgba(0, 255, 0, 0.3)', lineWidth: 1 });
        this.chart.addTimeSeries(this.gameSeries,   { strokeStyle: 'rgb(0, 0, 255)', fillStyle: 'rgba(0, 255, 0, 0.3)', lineWidth: 1 });
        this.chart.streamTo(this.canvas, /*delayMs*/ 0);

        this.closeables.add(new data.Subscription<state.Debug>(
            constants.protocols.debug,
            {
                onmessage: this.onDebugState.bind(this)
            }
        ));

        this.closeables.add(() => this.chart.stop());
    }

    public unload()
    {
        super.unload();
    }

    private onDebugState(data: state.Debug)
    {
        var time = new Date().getTime();

        this.ignoreSeries.append(time, data.ignoredMessages);
        this.gameSeries.append(time, data.gameControllerMessages);
    }

    public onResized(width: number, height: number, isFullScreen: boolean)
    {
        this.canvas.width = width;
    }
}

export = CommsModule;

/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import constants = require('constants');
import control = require('control');
import data = require('data');
import DOMTemplate = require('DOMTemplate');
import Legend = require('controls/Legend');
import state = require('state');
import TabControl = require('controls/TabControl');
import util = require('util');
import Module = require('Module');

var radarSize = 200,
    moveScale = 3,
    moduleTemplate = DOMTemplate.forId("walk-module-template");

var chartOptions = {
    interpolation: 'step',
    grid: {
        strokeStyle: 'rgba(0,0,0,0.1)',
        fillStyle: 'transparent',
        lineWidth: 1,
        millisPerLine: 250,
        verticalSections: 5,
        sharpLines: true
    },
    labels: {
        fillStyle: 'rgba(0,0,0,0.5)',
        precision: 0
    }
};

class WalkModule extends Module
{
    private runningIndicator: HTMLDivElement;
    private radarCanvas: HTMLCanvasElement;

    private statusStoppedSeries: TimeSeries;
    private statusStartingSeries: TimeSeries;
    private statusWalkingSeries: TimeSeries;
    private statusStabilisingSeries: TimeSeries;

    private pitchCurrentSeries: TimeSeries;
    private pitchTargetSeries: TimeSeries;

    private xAmpCurrentSeries: TimeSeries;
    private xAmpTargetSeries: TimeSeries;

    private angleCurrentSeries: TimeSeries;
    private angleTargetSeries: TimeSeries;

    private hipRollSeries: TimeSeries;
    private kneeSeries: TimeSeries;
    private anklePitchSeries: TimeSeries;
    private ankleRollSeries: TimeSeries;

    private statusChart: SmoothieChart;
    private pitchChart: SmoothieChart;
    private xAmpChart: SmoothieChart;
    private turnChart: SmoothieChart;
    private balanceChart: SmoothieChart;

    constructor()
    {
        super('walk', 'walk');
    }

    public load(width: number)
    {
        var templateRoot = <HTMLElement>moduleTemplate.create({radarSize: radarSize, chartWidth: 440, chartHeight: 60});
        this.element.appendChild(templateRoot);

        this.runningIndicator = <HTMLDivElement>templateRoot.querySelector('.connection-indicator');
        this.radarCanvas = <HTMLCanvasElement>templateRoot.querySelector('canvas.radar');

        control.buildSettings('options.approach-ball', templateRoot.querySelector('.approach-ball-controls'), this.closeables);
        control.buildSettings('walk-engine.params', templateRoot.querySelector('.walk-engine-params-controls'), this.closeables);
        control.buildSettings('walk-engine.gains', templateRoot.querySelector('.walk-engine-gains-controls'), this.closeables);
        control.buildSettings('balance', templateRoot.querySelector('.balance-controls'), this.closeables);
        control.buildSettings('walk-module', templateRoot.querySelector('.walk-module-controls'), this.closeables);

        this.closeables.add(new data.Subscription<state.Walk>(
            constants.protocols.walkState,
            {
                onmessage: this.onWalkState.bind(this)
            }
        ));

        this.closeables.add(new data.Subscription<state.Balance>(
            constants.protocols.balanceState,
            {
                onmessage: this.onBalanceState.bind(this)
            }
        ));

        this.drawRadar();

        this.statusStoppedSeries = new TimeSeries();
        this.statusStartingSeries = new TimeSeries();
        this.statusWalkingSeries = new TimeSeries();
        this.statusStabilisingSeries = new TimeSeries();

        this.pitchCurrentSeries = new TimeSeries();
        this.pitchTargetSeries = new TimeSeries();

        this.xAmpCurrentSeries = new TimeSeries();
        this.xAmpTargetSeries = new TimeSeries();

        this.angleCurrentSeries = new TimeSeries();
        this.angleTargetSeries = new TimeSeries();

        this.hipRollSeries = new TimeSeries();
        this.kneeSeries = new TimeSeries();
        this.anklePitchSeries = new TimeSeries();
        this.ankleRollSeries = new TimeSeries();

        var stoppedColour = 'rgba(0,0,0,0.2)',
            startingColour = 'rgba(255,255,0,0.2)',
            walkingColour = 'rgba(0,255,0,0.2)',
            stabilisingColour = 'rgba(0,0,255,0.2)';

        var statusLegend = new Legend([
            { colour: stoppedColour, name: 'Stopped' },
            { colour: startingColour, name: 'Starting' },
            { colour: walkingColour, name: 'Walking' },
            { colour: stabilisingColour, name: 'Stabilising' }
        ]);

        templateRoot.querySelector('.status-legend').appendChild(statusLegend.element);

        this.statusChart = new SmoothieChart(_.extend<any,any,any,any,any,any>({}, chartOptions, {minValue: 0, maxValue: 1 }));
        this.statusChart.addTimeSeries(this.statusStoppedSeries,     { fillStyle: stoppedColour,     lineWidth: 1 });
        this.statusChart.addTimeSeries(this.statusStartingSeries,    { fillStyle: startingColour,    lineWidth: 1 });
        this.statusChart.addTimeSeries(this.statusWalkingSeries,     { fillStyle: walkingColour,     lineWidth: 1 });
        this.statusChart.addTimeSeries(this.statusStabilisingSeries, { fillStyle: stabilisingColour, lineWidth: 1 });
        this.statusChart.streamTo(<HTMLCanvasElement>templateRoot.querySelector('canvas.status-chart'), /*delayMs*/ 0);

        this.pitchChart = new SmoothieChart(_.extend<any,any,any,any,any,any>({}, chartOptions, {minValue: 10, maxValue: 15}));
        this.pitchChart.addTimeSeries(this.pitchCurrentSeries, { strokeStyle: 'rgb(0, 0, 255)', lineWidth: 1 });
        this.pitchChart.addTimeSeries(this.pitchTargetSeries,  { strokeStyle: 'rgba(0, 0, 255, 0.4)', lineWidth: 1 });
        this.pitchChart.streamTo(<HTMLCanvasElement>templateRoot.querySelector('canvas.pitch-chart'), /*delayMs*/ 0);

        this.xAmpChart = new SmoothieChart(_.extend<any,any,any,any,any,any>({}, chartOptions, {minValue: 0, maxValue: 40}));
        this.xAmpChart.addTimeSeries(this.xAmpCurrentSeries, { strokeStyle: 'rgb(121, 36, 133)', lineWidth: 1 });
        this.xAmpChart.addTimeSeries(this.xAmpTargetSeries,  { strokeStyle: 'rgba(121, 36, 133, 0.4)', lineWidth: 1 });
        this.xAmpChart.streamTo(<HTMLCanvasElement>templateRoot.querySelector('canvas.x-amp-chart'), /*delayMs*/ 0);

        this.turnChart = new SmoothieChart(_.extend<any,any,any,any,any,any>({}, chartOptions, {minValue: -25, maxValue: 25}));
        this.turnChart.addTimeSeries(this.angleCurrentSeries, { strokeStyle: 'rgb(121, 36, 133)', lineWidth: 1 });
        this.turnChart.addTimeSeries(this.angleTargetSeries,  { strokeStyle: 'rgba(121, 36, 133, 0.4)', lineWidth: 1 });
        this.turnChart.streamTo(<HTMLCanvasElement>templateRoot.querySelector('canvas.turn-chart'), /*delayMs*/ 0);

        var hipRollColour = 'red',
            kneeColour = 'yellow',
            anklePitchColour = 'green',
            ankleRollColour = 'orange';

        var balanceLegend = new Legend([
            { colour: hipRollColour, name: 'Hip Roll' },
            { colour: kneeColour, name: 'Knee' },
            { colour: anklePitchColour, name: 'Ankle Pitch' },
            { colour: ankleRollColour, name: 'Ankle Roll' }
        ]);

        templateRoot.querySelector('.balance-legend').appendChild(balanceLegend.element);

        this.balanceChart = new SmoothieChart(_.extend<any,any,any,any,any,any>({}, chartOptions, {}));
        this.balanceChart.addTimeSeries(this.hipRollSeries,    { lineWidth: 1, strokeStyle: hipRollColour });
        this.balanceChart.addTimeSeries(this.kneeSeries,       { lineWidth: 1, strokeStyle: kneeColour });
        this.balanceChart.addTimeSeries(this.anklePitchSeries, { lineWidth: 1, strokeStyle: anklePitchColour });
        this.balanceChart.addTimeSeries(this.ankleRollSeries,  { lineWidth: 1, strokeStyle: ankleRollColour });
        this.balanceChart.streamTo(<HTMLCanvasElement>templateRoot.querySelector('canvas.balance-chart'), /*delayMs*/ 0);

        new TabControl(<HTMLDListElement>templateRoot.querySelector('dl.tab-control'));
    }

    public unload()
    {
        this.statusChart.stop();
        this.pitchChart.stop();
        this.xAmpChart.stop();
        this.turnChart.stop();
        this.balanceChart.stop();

        delete this.statusChart;
        delete this.pitchChart;
        delete this.xAmpChart;
        delete this.turnChart;
        delete this.balanceChart;

        delete this.pitchCurrentSeries;
        delete this.pitchTargetSeries;
        delete this.xAmpCurrentSeries;
        delete this.xAmpTargetSeries;
        delete this.angleCurrentSeries;
        delete this.angleTargetSeries;
        delete this.hipRollSeries;
        delete this.kneeSeries;
        delete this.anklePitchSeries;
        delete this.ankleRollSeries;

        delete this.radarCanvas;
        delete this.runningIndicator;
    }

    private drawRadar(data?: state.Walk)
    {
        // TODO use a dirty flag and only draw on animation frames

        var context = this.radarCanvas.getContext('2d');

        context.clearRect(0, 0, radarSize, radarSize);

        // Draw crosshairs

        var mid = Math.round(radarSize / 2);

        context.strokeStyle = 'rgba(0, 0, 0, 0.3)';
        context.lineWidth = 2;
        context.beginPath();
        context.moveTo(mid, 0);
        context.lineTo(mid, radarSize);
        context.moveTo(0, mid);
        context.lineTo(radarSize, mid);
        context.stroke();

        if (!data)
            return;

        mid = (radarSize / 2) + 0.5;

        //
        // Angles
        //

        // Target
        context.strokeStyle = 'rgba(121, 36, 133, 0.3)';
        context.beginPath();
        context.lineCap = 'round';
        context.lineWidth = 20;
        context.arc(mid, mid, radarSize * 0.4, -Math.PI / 2, -Math.PI / 2 - (data.target[2] * Math.PI / 180), data.target[2] > 0);
        context.stroke();

        // Current
        context.strokeStyle = 'rgba(121, 36, 133, 1)';
        context.beginPath();
        context.lineCap = 'round';
        context.lineWidth = 9;
        context.arc(mid, mid, radarSize * 0.4, -Math.PI / 2, -Math.PI / 2 - (data.current[2] * Math.PI / 180), data.current[2] > 0);
        context.stroke();

        //
        // Movement Direction
        //

        // Target
        context.strokeStyle = 'rgba(121, 36, 133, 0.3)';
        context.beginPath();
        context.lineCap = 'round';
        context.lineWidth = 20;
        context.moveTo(mid, mid);
        context.lineTo(mid + (data.target[1] * moveScale), mid - (data.target[0] * moveScale));
        context.stroke();

        // Current
        context.strokeStyle = 'rgba(121, 36, 133, 1)';
        context.beginPath();
        context.lineCap = 'round';
        context.lineWidth = 9;
        context.moveTo(mid, mid);
        context.lineTo(mid + (data.current[1] * moveScale), mid - (data.current[0] * moveScale));
        context.stroke();
    }

    private onBalanceState(data: state.Balance)
    {
        var time = new Date().getTime();
        this.hipRollSeries.append(time, data.offsets.hipRoll);
        this.kneeSeries.append(time, data.offsets.knee);
        this.anklePitchSeries.append(time, data.offsets.anklePitch);
        this.ankleRollSeries.append(time, data.offsets.ankleRoll);
    }

    private onWalkState(data: state.Walk)
    {
        if (data.running)
        {
            this.runningIndicator.classList.add('connected');
            this.runningIndicator.classList.remove('disconnected');
            this.runningIndicator.textContent = data.phase.toString();
        }
        else
        {
            this.runningIndicator.classList.remove('connected');
            this.runningIndicator.classList.add('disconnected');
            this.runningIndicator.textContent = '';
        }

        var time = new Date().getTime();
        this.pitchCurrentSeries.append(time, data.hipPitch.current);
        this.pitchTargetSeries.append(time, data.hipPitch.target);
        this.xAmpCurrentSeries.append(time, data.current[0]);
        this.xAmpTargetSeries.append(time, data.target[0]);
        this.angleCurrentSeries.append(time, data.current[2]);
        this.angleTargetSeries.append(time, data.target[2]);

        this.statusStoppedSeries.append(time,     data.status === 0 ? 1 : 0);
        this.statusStartingSeries.append(time,    data.status === 1 ? 1 : 0);
        this.statusWalkingSeries.append(time,     data.status === 2 ? 1 : 0);
        this.statusStabilisingSeries.append(time, data.status === 3 ? 1 : 0);

        this.drawRadar(data);
    }
}

export = WalkModule;

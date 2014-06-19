/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import constants = require('constants');
import data = require('data');
import control = require('control');
import BodyFigure = require('controls/BodyFigure');
import state = require('state');
import Module = require('Module');

var chartOptions = {
    millisPerPixel: 1000,
    interpolation:'linear',
    grid: {
        strokeStyle: 'rgb(40, 40, 40)',
        fillStyle: 'rgb(0, 0, 0)',
        lineWidth: 0.5,
        millisPerLine: 60000,
        verticalSections: 6,
        sharpLines: true,
        borderVisible: false
    },
    labels: {
        fillStyle: '#ffffff',
        precision: 0
    }
};

var chartHeight = 150,
    chartWidth = 430,
    lowTemperature = 25,
    highTemperature = 65,
    lowVoltage = 11,
    highVoltage = 13;

class HardwareModule extends Module
{
    private voltageCanvas: HTMLCanvasElement;
    private voltageChart: SmoothieChart;
    private voltageSeries: TimeSeries;
    private temperatureCanvas: HTMLCanvasElement;
    private temperatureChart: SmoothieChart;
    private temperatureSeriesById: TimeSeries[];
    private bodyFigure: BodyFigure;
    private lastDataTime: number;

    constructor()
    {
        super('hardware', 'hardware');
    }

    public onResized(width: number, height: number, isFullScreen: boolean)
    {
        var rightMargin = 210;
        this.voltageCanvas.width = width - rightMargin;
        this.voltageCanvas.style.width = (width - rightMargin) + "px";
        this.temperatureCanvas.width = width - rightMargin;
        this.temperatureCanvas.style.width = (width - rightMargin) + "px";
    }

    public load()
    {
        this.temperatureSeriesById = [undefined];

        // VOLTAGE

        var voltageHeader = document.createElement('h2');
        voltageHeader.textContent = 'voltage';
        this.element.appendChild(voltageHeader);

        this.voltageCanvas = document.createElement('canvas');
        this.voltageCanvas.width = chartWidth;
        this.voltageCanvas.height = chartHeight;
        this.element.appendChild(this.voltageCanvas);

        this.voltageChart = new SmoothieChart(chartOptions);
        this.voltageChart.options.yRangeFunction = range =>
            ({
                min: Math.floor(Math.min(range.min, lowVoltage)),
                max: Math.ceil(Math.max(range.max, highVoltage))
            });
        this.voltageChart.streamTo(this.voltageCanvas, /*delayMs*/ 200);

        this.voltageSeries = new TimeSeries();
        this.voltageChart.addTimeSeries(this.voltageSeries, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });

        // TEMPERATURES

        var tempHeader = document.createElement('h2');
        tempHeader.textContent = 'temperature';
        this.element.appendChild(tempHeader);

        this.temperatureCanvas = document.createElement('canvas');
        this.temperatureCanvas.width = chartWidth;
        this.temperatureCanvas.height = chartHeight;
        this.element.appendChild(this.temperatureCanvas);

        this.temperatureChart = new SmoothieChart(chartOptions);
        this.temperatureChart.options.yRangeFunction = range =>
            ({
                min: Math.floor(Math.min(range.min, lowTemperature)),
                max: Math.ceil(Math.max(range.max, highTemperature))
            });
        this.temperatureChart.streamTo(this.temperatureCanvas, /*delayMs*/ 200);

        for (var i = 1; i <= 20; i++) {
            var series = new TimeSeries();
            this.temperatureChart.addTimeSeries(series, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });
            this.temperatureSeriesById.push(series);
        }

        this.bodyFigure = new BodyFigure();
        this.element.appendChild(this.bodyFigure.element);

        var controlContainer = document.createElement('div');
        controlContainer.className = 'control-container';
        control.buildActions("hardware", controlContainer);
        this.element.appendChild(controlContainer);

        this.closeables.add(new data.Subscription<state.Hardware>(
            constants.protocols.hardwareState,
            {
                onmessage: this.onHardwareState.bind(this)
            }
        ));
    }

    public unload()
    {
        this.voltageChart.stop();
        this.temperatureChart.stop();

        delete this.voltageChart;
        delete this.voltageSeries;
        delete this.temperatureChart;
        delete this.temperatureSeriesById;
        delete this.bodyFigure;
    }

    private onHardwareState(data: state.Hardware)
    {
        var time = new Date().getTime();

        // Only allow one point per second, to avoid using too much memory
        if (this.lastDataTime && (time - this.lastDataTime) < 1000)
            return;

        this.lastDataTime = time;

        this.voltageSeries.append(time, data.volts);

        _.each(data.joints, joint =>
        {
            this.temperatureSeriesById[joint.id].append(time, joint.temp);

            var jointElement = this.bodyFigure.getJointElement(joint.id),
                t = Math.max(Math.min(highTemperature, joint.temp), lowTemperature),
                ratio = (t - lowTemperature) / (highTemperature - lowTemperature),
                hue = 140 * (1 - ratio),
                hsl = 'hsl('  + hue + ', 100%, 50%)';

            jointElement.textContent = joint.temp.toString();
            jointElement.style.background = hsl;
        });
    }
}

export = HardwareModule;

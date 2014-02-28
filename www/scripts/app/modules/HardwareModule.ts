/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import constants = require('constants');
import data = require('data');
import control = require('control');
import BodyFigure = require('BodyFigure');
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
    private voltageChart: SmoothieChart;
    private voltageSeries: TimeSeries;
    private temperatureChart: SmoothieChart;
    private temperatureSeriesById: TimeSeries[] = [undefined];
    private bodyFigure: BodyFigure;
    private lastDataTime: number;

    constructor()
    {
        super('hardware', 'hardware');
    }

    public load(element: HTMLDivElement)
    {
        // VOLTAGE

        var voltageHeader = document.createElement('h2');
        voltageHeader.textContent = 'voltage';
        element.appendChild(voltageHeader);

        var voltageCanvas = document.createElement('canvas');
        voltageCanvas.width = chartWidth;
        voltageCanvas.height = chartHeight;
        element.appendChild(voltageCanvas);

        this.voltageChart = new SmoothieChart(chartOptions);
        this.voltageChart.options.yRangeFunction = range =>
            ({
                min: Math.floor(Math.min(range.min, lowVoltage)),
                max: Math.ceil(Math.max(range.max, highVoltage))
            });
        this.voltageChart.streamTo(voltageCanvas, /*delayMs*/ 200);

        this.voltageSeries = new TimeSeries();
        this.voltageChart.addTimeSeries(this.voltageSeries, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });

        // TEMPERATURES

        var tempHeader = document.createElement('h2');
        tempHeader.textContent = 'temperature';
        element.appendChild(tempHeader);

        var temperatureCanvas = document.createElement('canvas');
        temperatureCanvas.width = chartWidth;
        temperatureCanvas.height = chartHeight;
        element.appendChild(temperatureCanvas);

        this.temperatureChart = new SmoothieChart(chartOptions);
        this.temperatureChart.options.yRangeFunction = range =>
            ({
                min: Math.floor(Math.min(range.min, lowTemperature)),
                max: Math.ceil(Math.max(range.max, highTemperature))
            });
        this.temperatureChart.streamTo(temperatureCanvas, /*delayMs*/ 200);

        for (var i = 1; i <= 20; i++) {
            var series = new TimeSeries();
            this.temperatureChart.addTimeSeries(series, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });
            this.temperatureSeriesById.push(series);
        }

        this.bodyFigure = new BodyFigure();
        element.appendChild(this.bodyFigure.element);

        var controlContainer = document.createElement('div');
        controlContainer.className = 'control-container';
        control.buildActions("hardware", controlContainer);
        element.appendChild(controlContainer);

        this.closeables.add(new data.Subscription<state.Hardware>(
            constants.protocols.hardwareState,
            {
                onmessage: this.onData.bind(this)
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

    private onData(data: state.Hardware)
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

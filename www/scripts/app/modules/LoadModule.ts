/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import constants = require('constants');
import data = require('data');
import BodyFigure = require('controls/BodyFigure');
import state = require('state');
import Module = require('Module');

var chartOptions = {
    millisPerPixel: 20,
    interpolation:'linear',
    grid: {
        strokeStyle: 'rgb(40, 40, 40)',
        fillStyle: 'rgb(0, 0, 0)',
        lineWidth: 0.5,
        millisPerLine: 500,
        verticalSections: 7,
        sharpLines: true,
        borderVisible: false
    },
    labels: {
        fillStyle: '#ffffff',
        precision: 2
    },
    horizontalLines: [
        { color: '#ffffff', lineWidth: 1, value: 0 }
    ]
};

var chartHeight = 300,
    chartWidth = 430,
    lowDiff = 5,
    highDiff = 50;

class LoadModule extends Module
{
    private diffCanvas: HTMLCanvasElement;
    private diffChart: SmoothieChart;
    private bodyFigure: BodyFigure;
    private diffSeriesById: TimeSeries[];

    constructor()
    {
        super('load', 'load');
    }

    public load(element: HTMLDivElement)
    {
        var posErrHeader = document.createElement('h2');
        posErrHeader.textContent = 'position error';
        element.appendChild(posErrHeader);

        this.diffCanvas = document.createElement('canvas');
        this.diffCanvas.width = chartWidth;
        this.diffCanvas.height = chartHeight;
        element.appendChild(this.diffCanvas);

        this.diffChart = new SmoothieChart(chartOptions);
        this.diffChart.options.yRangeFunction = range =>
        {
            var lim = Math.max(Math.abs(range.min), Math.abs(range.max));
            if (lim < 50) {
                lim = 50;
            }
            return {
                min: -lim,
                max:  lim
            };
        };
        this.diffChart.streamTo(this.diffCanvas, /*delayMs*/ 0);

        this.diffSeriesById = [undefined];
        for (var i = 1; i <= 20; i++) {
            var series = new TimeSeries();
            this.diffChart.addTimeSeries(series, { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 });
            this.diffSeriesById.push(series);
        }

        this.bodyFigure = new BodyFigure({ hasHover: true, hasSelection: true });
        this.bodyFigure.hoverJointId.track(this.updateSeriesColours.bind(this));
        this.bodyFigure.selectedJointIds.track(this.updateSeriesColours.bind(this));
        element.appendChild(this.bodyFigure.element);

        this.closeables.add(new data.Subscription<state.Body>(
            constants.protocols.bodyState,
            {
                onmessage: this.onBodyState.bind(this)
            }
        ));
    }

    public unload()
    {
        this.diffChart.stop();

        delete this.diffCanvas;
        delete this.diffChart;
        delete this.diffSeriesById;
        delete this.bodyFigure;
    }

    private updateSeriesColours()
    {
        _.each(_.range(1,21), jointId =>
        {
            var series = this.diffSeriesById[jointId],
                isHovered = jointId === this.bodyFigure.hoverJointId.getValue(),
                isSelected = this.bodyFigure.selectedJointIds.getValue().indexOf(jointId) !== -1;

            this.diffChart.getTimeSeriesOptions(series).strokeStyle
                = isHovered || isSelected
                  ? 'rgba(121, 36, 133, 0.7)'
                  : '#ffffff';

            if (isHovered || isSelected)
                this.diffChart.bringToFront(series);
        });
    }

    private onBodyState(data: state.Body)
    {
        var time = new Date().getTime();

        _.each(_.range(1,21), jointId =>
        {
            var error = data.errors[jointId];

            this.diffSeriesById[jointId].append(time, error);

            var jointElement = this.bodyFigure.getJointElement(jointId),
                t = Math.max(Math.min(highDiff, Math.abs(error)), lowDiff),
                ratio = (t - lowDiff) / (highDiff - lowDiff),
                hue = 140 * (1 - ratio),
                hsl = 'hsl('  + hue + ', 100%, 50%)';

            jointElement.textContent = error.toString();
            jointElement.style.background = hsl;
        });
    }
}

export = LoadModule;

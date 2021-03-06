/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/d3.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import data = require('data');
import state = require('state');
import constants = require('constants');
import PolarTrace = require('controls/PolarTrace');
import Legend = require('controls/Legend');
import Module = require('Module');

// TODO reuse RGB <==> XYZ colour coding on polar trace axes

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
    }
};

var red = '#ED303C',
    grn = '#44C425',
    blu = '#00A8C6';

var charts = [
    {
        title: 'gyro',
        options: chartOptions,
        series: [
            { strokeStyle: red, lineWidth: 1 },
            { strokeStyle: grn, lineWidth: 1 },
            { strokeStyle: blu, lineWidth: 1 }
        ]
    },
    {
        title: 'acc',
        options: chartOptions,
        series: [
            { strokeStyle: red, lineWidth: 1 },
            { strokeStyle: grn, lineWidth: 1 },
            { strokeStyle: blu, lineWidth: 1 }
        ]
    }
];

var chartHeight = 150;

class IMUModule extends Module
{
    private charts: SmoothieChart[];
    private seriesArray: TimeSeries[];
    private chartCanvases: HTMLCanvasElement[];
    private polarTraceXY: PolarTrace;
    private polarTraceYZ: PolarTrace;
    private polarTraceXZ: PolarTrace;

    constructor()
    {
        super('imu', 'IMU', {fullScreen: true});
    }

    public load(width: number)
    {
        this.seriesArray = [];
        this.chartCanvases = [];

        this.buildLegend();

        this.buildCharts(width);

        var addPolarTrace = (name: string, xColour: string, yColour: string) =>
        {
            var trace = new PolarTrace(),
                div = document.createElement('div'),
                h2 = document.createElement('h2');
            div.className = 'polar-trace-container';
            h2.textContent = name;
            div.appendChild(h2);
            div.appendChild(trace.element);
            trace.setAxesColours(xColour, yColour)

            this.element.appendChild(div);
            return trace;
        };

        this.polarTraceXY = addPolarTrace('X|Y (above)', red, grn);
        this.polarTraceYZ = addPolarTrace('Y|Z (side)', grn, blu);
        this.polarTraceXZ = addPolarTrace('X|Z (behind)', red, blu);

        this.closeables.add(new data.Subscription<state.Hardware>(
            constants.protocols.hardwareState,
            {
                onmessage: this.onHardwareState.bind(this)
            }
        ));

        _.each(this.charts, chart => this.closeables.add({ close: () => chart.stop() }));
    }

    private buildLegend()
    {
        var legend = new Legend([
            {name:"X", colour: red},
            {name:"Y", colour: grn},
            {name:"Z", colour: blu}
        ]);

        this.element.appendChild(legend.element);
    }

    private buildCharts(width: number)
    {
        this.charts = [];
        _.each(charts, chartDefinition =>
        {
            var chart = new SmoothieChart(chartDefinition.options);
            this.charts.push(chart);
            chart.options.yRangeFunction = range =>
            {
                // Find the greatest absolute value
                var max = Math.max(Math.abs(range.min), Math.abs(range.max));
                // Ensure we're viewing at least a quarter of the range, so that
                // very small values don't appear exaggeratedly large
                max = Math.max(max, 1);
                return {min:-max, max:max};
            };
            chart.options.horizontalLines.push({color:'#ffffff', lineWidth: 1, value: 0});

            var h2 = document.createElement('h2');
            h2.textContent = chartDefinition.title;
            this.element.appendChild(h2);

            var canvas = document.createElement('canvas');
            canvas.width = width;
            canvas.height = chartHeight;
            this.element.appendChild(canvas);

            this.chartCanvases.push(canvas);

            chart.streamTo(canvas, /*delayMs*/ 0);

            _.each(chartDefinition.series, seriesDefinition =>
            {
                var series = new TimeSeries();
                chart.addTimeSeries(series, seriesDefinition);
                this.seriesArray.push(series);
            });
        });
    }

    public onResized(width: number, height: number, isFullScreen: boolean)
    {
        _.each(this.chartCanvases, canvas => canvas.width = width);
    }

    private onHardwareState(data: state.Hardware)
    {
        var time = new Date().getTime();

        // copy values to charts
        this.seriesArray[0].append(time, data.gyro[0]);
        this.seriesArray[1].append(time, data.gyro[1]);
        this.seriesArray[2].append(time, data.gyro[2]);
        this.seriesArray[3].append(time, data.acc[0]);
        this.seriesArray[4].append(time, data.acc[1]);
        this.seriesArray[5].append(time, data.acc[2]);

        this.polarTraceXY.addValue(data.acc[0], data.acc[1]);
        this.polarTraceYZ.addValue(data.acc[1], data.acc[2]);
        this.polarTraceXZ.addValue(data.acc[0], data.acc[2]);
    }
}

export = IMUModule;

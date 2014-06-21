/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import constants = require('constants');
import data = require('data');
import Closeable = require('util/Closeable');
import math = require('util/math');
import state = require('state');

var seriesOptions = {
    strokeStyle: 'rgb(0, 255, 0)',
    fillStyle: 'rgba(0, 255, 0, 0.3)',
    lineWidth: 1
};

var chartHeight = 150;

interface ITimingEntry
{
    label: string;
    avg: math.MovingAverage;
    update: (timestamp: number, millis: number)=>void;
    row: HTMLTableRowElement;
    children: ITimingEntry[];
    isExpanded: boolean;
    time?: number;
    millis?: number;
    maxMillis?: number;
}

class TimingPane
{
    private container: HTMLDivElement;
    private thresholdMillis: number;
    private chart: SmoothieChart;
    private canvas: HTMLCanvasElement;
    private series: TimeSeries;
    private table: HTMLTableElement;
    private fps: HTMLDivElement;
    private closeables: Closeable = new Closeable();
    private entryByLabel: {[label:string]:ITimingEntry} = {};

    constructor(private protocol: string, private targetFps: number)
    {
        this.container = document.createElement('div');
        this.thresholdMillis = 1000 / targetFps;
    }

    public load(element: HTMLDivElement)
    {
        var chartOptions = {
            interpolation: 'step',
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
            yRangeFunction: range => ({min: 0, max: Math.max(this.thresholdMillis, range.max)}),
            horizontalLines: [
                {color: '#FF0000', lineWidth: 1, value: this.thresholdMillis}
            ]
        };

        this.chart = new SmoothieChart(chartOptions);
        this.canvas = document.createElement('canvas');
        this.canvas.height = chartHeight;
        element.appendChild(this.canvas);

        this.series = new TimeSeries();
        this.chart.addTimeSeries(this.series, seriesOptions);
        this.chart.streamTo(this.canvas, /*delayMs*/ 0);

        this.fps = document.createElement('div');
        this.fps.className = 'fps';
        element.appendChild(this.fps);

        var reset = document.createElement('a');
        reset.href = '#';
        reset.className = 'reset';
        reset.textContent = 'reset';
        reset.addEventListener('click', () =>
        {
            this.resetMaximums();
            return false;
        });
        element.appendChild(reset);

        this.table = document.createElement('table');
        this.table.className = 'timing-details';
        element.appendChild(this.table);

        this.entryByLabel = {};

        this.closeables.add(new data.Subscription<state.Timing>(
            this.protocol,
            {
                onmessage: this.onTimingState.bind(this)
            }
        ));
    }

    public unload()
    {
        this.chart.stop();

        this.closeables.closeAll();

        delete this.series;
        delete this.chart;
        delete this.table;
        delete this.fps;
        delete this.canvas;
        delete this.entryByLabel;
    }

    public onResized(width:number)
    {
        this.canvas.width = width;
    }

    private onTimingState(data: state.Timing)
    {
        var time = new Date().getTime();
        var timings = data.timings;

        _.each(_.keys(timings), key =>
        {
            var millis = <number>timings[key];
            this.getOrCreateEntry(key).update(time, millis);
        });

        this.fps.textContent = data.fps.toFixed(1);
        this.updateChart(time);
    }

    private updateChart(time)
    {
        var totalTime = 0;

        _.each(_.values(this.entryByLabel), entry =>
        {
            if (entry.time === time && entry.label.indexOf('/') === -1)
                totalTime += entry.millis;
        });

        this.series.append(time, totalTime);
    }

    private resetMaximums()
    {
        _.each(_.values(this.entryByLabel), entry => entry.maxMillis = 0);
    }

    private getOrCreateEntry(label: string): ITimingEntry
    {
        var entry = this.entryByLabel[label];

        if (!entry) {
            // If this entry is a child of another entry, try to find it's parent
            var parts = label.split('/'),
                hasParent = parts.length !== 1,
                parent;

            // Some messing around to get paths in a nice order, where parents appear above children
            // Image Processing
            // Image Processing/Pixel Label
            // Image Processing/Pixel Label/Find Horizon
            // Image Processing/Pixel Label/Pixels Above
            for (var i = 1; i < parts.length; i++) {
                var parentPath = parts.slice(0, i).join('/');
                parent = this.getOrCreateEntry(parentPath);
                parent.row.classList.add('parent');
                if (i === 1)
                    parent.row.classList.add('root');
            }

            var row = document.createElement('tr');

            var cellExpand = document.createElement('td'),     // {'class': 'expander'}).appendTo(row).get(0),
                cellLabel = document.createElement('td'),      //.text(label).appendTo(row).get(0),
                cellMillis = document.createElement('td'),     // {'class': 'duration'}).appendTo(row).get(0),
                cellMAvgMillis = document.createElement('td'), // {'class': 'avg-duration'}).appendTo(row).get(0),
                cellMaxMillis = document.createElement('td');  // {'class': 'max-duration'}).appendTo(row).get(0);

            cellExpand.className = 'expander';
            cellLabel.textContent = label;
            cellMillis.className = 'duration';
            cellMAvgMillis.className = 'avg-duration';
            cellMaxMillis.className = 'max-duration';

            row.appendChild(cellExpand);
            row.appendChild(cellLabel);
            row.appendChild(cellMillis);
            row.appendChild(cellMAvgMillis);
            row.appendChild(cellMaxMillis);

            entry = {
                label: label,
                avg: new math.MovingAverage(this.targetFps * 4), // four second moving average
                update: (timestamp: number, millis: number) =>
                {
                    entry.time = timestamp;
                    entry.millis = millis;
                    if (!entry.maxMillis || entry.maxMillis < millis) {
                        entry.maxMillis = millis;
                        cellMaxMillis.textContent = millis.toFixed(3);
                    }
                    cellMillis.textContent = millis.toFixed(3);
                    cellMAvgMillis.textContent = entry.avg.next(millis).toFixed(3);
                },
                row: row,
                children: [],
                isExpanded: false
            };

            var setChildRowDisplay = (e: ITimingEntry) =>
            {
                var display = e.isExpanded ? 'table-row' : 'none';
                _.each(e.children, child =>
                {
                    child.row.style.display = display;
                    setChildRowDisplay(child);
                });
            };

            var setRowExpansion = isExpanded =>
            {
                entry.isExpanded = isExpanded;

                if (isExpanded)
                    entry.row.classList.add("expanded");
                else
                    entry.row.classList.remove("expanded");
            };

            entry.row.addEventListener('click', () =>
            {
                setRowExpansion(!entry.isExpanded);
                setChildRowDisplay(entry);
            });

            this.entryByLabel[label] = entry;

            if (hasParent) {
                parent.children.push(entry);
                entry.row.style.display = parent.isExpanded ? 'table-row' : 'none';
            }

            setRowExpansion(entry.isExpanded);

            this.table.appendChild(row);
        }

        return entry;
    }
}

export = TimingPane;

/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/d3.d.ts" />

import DataProxy = require('DataProxy');
import constants = require('constants');
import Module = require('Module');

var colourById = [];
colourById[1] = 'yellow';
colourById[2] = 'orange';
colourById[3] = 'green';
colourById[4] = 'white';

var countFormatter: (value: number) => string = d3.format(",d"),
    chartHeight = 140,
    chartWidth = 640,
    spacing = 4,
    totalArea = 320*240;

class HistogramModule extends Module
{
    private svg: SVGElement;
    private chart: D3.Selection;
    private xScale: D3.Scale.LinearScale;
    private yScale: D3.Scale.OrdinalScale;

    constructor()
    {
        super('histogram', 'histogram');
    }

    public load(element: HTMLDivElement)
    {
        this.closeables.add(DataProxy.subscribe(
            constants.protocols.labelCount,
            {
                json: true,
                onmessage: _.bind(this.onmessage, this)
            }
        ));

        this.svg = <SVGElement>document.createElementNS(d3.ns.prefix.svg, 'svg');

        element.appendChild(this.svg);

        this.chart = d3.select(this.svg)
                .attr('class', 'chart')
                .attr('width', chartWidth)
                .attr('height', chartHeight);

        this.xScale = d3.scale.linear()
           .domain([0, totalArea])
           .range([0, chartWidth]);

        this.yScale = d3.scale.ordinal()
           .domain([1,2,3,4])
           .rangeBands([0, chartHeight]);

        this.chart.append('line')
             .attr('y1', 0)
             .attr('y2', chartHeight)
             .style('stroke', '#000');
    }

    public unload()
    {
        delete this.svg;
        delete this.chart;
        delete this.xScale;
        delete this.yScale;
    }

    private onmessage(data)
    {
        var labels = data.labels;

        var bars = this.chart.selectAll('rect')
            .data(labels);

        bars.enter()
            .append('rect')
            .attr('y', d => this.yScale(d.id) + spacing/2)
            .attr('width', d => this.xScale(<number>d.count))
            .attr('height', this.yScale.rangeBand() - spacing)
            .attr('fill', d => colourById[d.id]);

        bars.transition()
            .duration(30)
            .attr('width', d => this.xScale(<number>d.count));

        var texts = this.chart.selectAll('text.label')
            .data(labels);

        texts.enter()
             .append('text')
             .attr('class', 'label')
             .attr('x', d => this.xScale(<number>d.count))
             .attr('y', d => this.yScale(d.id) + this.yScale.rangeBand() / 2)
             .attr('dx', 3)
             .attr('dy', '.35em')
             .attr('text-anchor', 'start')
             .text(d => d.name + ' (' + countFormatter(<number>d.count) + ')');

        texts.text(d => d.name + ' (' + countFormatter(<number>d.count) + ')')
            .transition()
            .duration(30)
            .attr('x', d => this.xScale(<number>d.count));
    }
}

export = HistogramModule;

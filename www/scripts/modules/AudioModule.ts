/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../../libs/d3.d.ts" />

import data = require('data');
import state = require('state');
import constants = require('constants');
import Module = require('Module');

var countFormatter: (value: number) => string = d3.format(",d"),
    chartHeight = 240,
    chartWidth = 640,
    axisSpace = 25;

class AudioModule extends Module
{
    private svg: SVGElement;
    private chart: D3.Selection;
    private xScale: D3.Scale.LinearScale;
    private yScale: D3.Scale.LinearScale;
    private xAxis: D3.Svg.Axis;
    private yAxis: D3.Svg.Axis;

    constructor()
    {
        super('audio', 'audio');
    }

    public load(width: number)
    {
        this.closeables.add(new data.Subscription<state.AudioPowerSpectrum>(
            constants.protocols.audioPowerSpectrumState,
            {
                onmessage: this.onAudioPowerSpectrumState.bind(this)
            }
        ));

        this.svg = <SVGElement>document.createElementNS(d3.ns.prefix.svg, 'svg');

        this.element.appendChild(this.svg);

        this.chart = d3.select(this.svg)
                .attr('class', 'chart')
                .attr('width', chartWidth)
                .attr('height', chartHeight);

        this.xScale = d3.scale.linear()
           .range([axisSpace, chartWidth - 10]);

        this.yScale = d3.scale.linear()
           .domain([50, 0])
           .range([15, chartHeight - axisSpace]);

        this.yAxis = d3.svg.axis().orient('left').scale(this.yScale).ticks(10);
        this.xAxis = d3.svg.axis().orient('bottom');

        d3.select(this.svg)
            .append('g')
            .attr('class', 'y-axis axis')
            .attr('transform', 'translate(' + axisSpace + ',0)')
            .call(this.yAxis);

        d3.select(this.svg)
            .append('g')
            .attr('class', 'x-axis axis')
            .attr('transform', 'translate(0,' + (chartHeight - axisSpace) + ')');

        d3.select(this.svg)
            .append('g')
            .attr('class', 'series');
    }

    public unload()
    {
        delete this.svg;
        delete this.chart;
        delete this.xScale;
        delete this.yScale;
    }

    private onAudioPowerSpectrumState(data: state.AudioPowerSpectrum)
    {
        var width = Math.max(2, chartWidth / data.dbLevels.length);

        var markerHeight = 5;

        this.xScale.domain([0, data.dbLevels.length]);

        // TODO try using a log scale for frequency
        // TODO only rebuild this scale when needed
        this.xAxis.scale(d3.scale.linear().range([axisSpace, chartWidth - 10]).domain([0, data.maxHertz]));
        d3.select(this.svg).select('g.x-axis').call(this.xAxis);

        var bars = this.chart.select('g.series')
            .selectAll('rect')
            .data(data.dbLevels);

        bars.enter()
            .append('rect')
            .attr('width', d => width)
            .attr('height', markerHeight)
            .attr('fill', d => 'blue');

        bars.exit().remove();

        bars.attr('y', d => this.yScale(Math.max(0,d)) - markerHeight)
            .attr('x', (d,i) => this.xScale(i));
    }
}

export = AudioModule;

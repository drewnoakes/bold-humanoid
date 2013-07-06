/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'Protocols',
        'DataProxy'
    ],
    function(Protocols, DataProxy)
    {
        'use strict';

        var colourById = [];
        colourById[1] = 'yellow';
        colourById[2] = 'orange';
        colourById[3] = 'green';
        colourById[4] = 'white';

        var countFormatter = d3.format(",d"),
            chartHeight = 140,
            chartWidth = 640,
            spacing = 4,
            totalArea = 320*240;

        var HistogramModule = function()
        {
            this.$container = $('<div></div>');

            this.title = 'histogram';
            this.id = 'histogram';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    supports: { fullScreen: true, advanced: false }
                }
            ];
        };

        HistogramModule.prototype.load = function()
        {
            this.subscription = DataProxy.subscribe(
                Protocols.labelCount,
                {
                    json: true,
                    onmessage: _.bind(this.onmessage, this)
                }
            );

            this.svg = document.createElementNS(d3.ns.prefix.svg, 'svg');

            this.$container.append(this.svg);

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
        };

        HistogramModule.prototype.unload = function()
        {
            this.$container.empty();
            this.subscription.close();
            delete this.svg;
            delete this.chart;
            delete this.xScale;
            delete this.yScale;
        };

        HistogramModule.prototype.onmessage = function(data)
        {
            var labels = data.labels;

            var bars = this.chart.selectAll('rect')
                .data(labels);

            bars.enter()
                .append('rect')
                .attr('y', function(d) { return this.yScale(d.id) + spacing/2;}.bind(this))
                .attr('width', function(d) { return this.xScale(d.count); }.bind(this))
                .attr('height', this.yScale.rangeBand() - spacing)
                .attr('fill', function(d) { return colourById[d.id]; });

            bars.transition()
                .duration(30)
                .attr('width', function(d) { return this.xScale(d.count); }.bind(this));

            var texts = this.chart.selectAll('text.label')
                .data(labels);

            texts.enter()
                 .append('text')
                 .attr('class', 'label')
                 .attr('x', function(d) { return this.xScale(d.count); }.bind(this))
                 .attr('y', function(d) { return this.yScale(d.id) + this.yScale.rangeBand() / 2; }.bind(this))
                 .attr('dx', 3)
                 .attr('dy', '.35em')
                 .attr('text-anchor', 'start')
                 .text(function(d) { return d.name + ' (' + countFormatter(d.count) + ')'; });

            texts.text(function(d) { return d.name + ' (' + countFormatter(d.count) + ')'; })
                .transition()
                .duration(30)
                .attr('x', function(d) { return this.xScale(d.count);}.bind(this));
        };

        return HistogramModule;
    }
);

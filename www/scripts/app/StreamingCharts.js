/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/DataProxy',
        'scripts/app/Protocols'
    ],
    function(DataProxy, Protocols)
    {
        'use strict';

        var chartOptions = {
            grid: {
                strokeStyle: 'rgb(40, 40, 40)',
                fillStyle: 'rgb(0, 0, 0)',
                lineWidth: 1,
                millisPerLine: 250,
                verticalSections: 6,
                sharpLines: true
            },
            labels: {
                fillStyle: '#ffffff'
            }
        };

        var charts = [
            {
                title: 'gyro',
                options: chartOptions,
                series: [
                    { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 },
                    { strokeStyle: 'rgb(0, 255, 0)', lineWidth: 1 },
                    { strokeStyle: 'rgb(0, 0, 255)', lineWidth: 1 }
                ]
            },
            {
                title: 'acc',
                options: chartOptions,
                series: [
                    { strokeStyle: 'rgb(255, 0, 0)', lineWidth: 1 },
                    { strokeStyle: 'rgb(0, 255, 0)', lineWidth: 1 },
                    { strokeStyle: 'rgb(0, 0, 255)', lineWidth: 1 }
                ]
            }
        ];

        var chartWidth = 640,
            chartHeight = 120,
            container = $('#streaming-charts'),
            seriesArray = [];

        //
        // build the chart objects
        //
        _.each(charts, function(chartDefinition)
        {
            var chart = new SmoothieChart(chartDefinition.options);
            chart.yRangeFunction = function(range)
            {
                // Find the greatest absolute value
                var max = Math.max(Math.abs(range.min), Math.abs(range.max));
                // Ensure we're viewing at least a quarter of the range, so that
                // very small values don't appear exaggeratedly large
                max = Math.max(max, 1);
                return {min:-max, max:max};
            };
            chart.options.horizontalLines.push({color:'#ffffff', lineWidth: 1, value: 0});

            container.append($('<h2></h2>', {text:chartDefinition.title}));
            var canvas = document.createElement('canvas');
            canvas.width = chartWidth;
            canvas.height = chartHeight;
            container.append(canvas);

            chart.streamTo(canvas, /*delayMs*/ 200);

            _.each(chartDefinition.series, function(seriesDefinition)
            {
                var series = new TimeSeries();
                chart.addTimeSeries(series, seriesDefinition);
                seriesArray.push(series);
            });
        });

        //
        // connect the chart with data
        //
        // TODO when the concept of Modules are introduced, track this subscription for module unload
        var subscription = DataProxy.subscribe(
            Protocols.agentModel,
            {
                onmessage: function (msg)
                {
                    var time = new Date().getTime(),
                        floats = DataProxy.parseFloats(msg.data);

                    // copy values to charts
                    for (var f = 0; f < floats.length && f < seriesArray.length; f++) {
                        seriesArray[f].append(time, floats[f]);
                    }
                }
            }
        );
    }
);

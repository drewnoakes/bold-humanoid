/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
  [
    'DataProxy'
  ],
  function(DataProxy)
  {
    // TODO allow resetting maximums
    // TODO reorder table rows lexicographically

    var seriesOptions = {
      strokeStyle: 'rgb(0, 255, 0)',
      fillStyle: 'rgba(0, 255, 0, 0.3)',
      lineWidth: 1
    };

    var chartHeight = 150;

    var TimingPane = function(protocol, thresholdMillis)
    {
      this.$container = $('<div></div>');
      this.container = this.$container.get(0);
      this.protocol = protocol;
      this.thresholdMillis = thresholdMillis;
    };

    TimingPane.prototype.load = function()
    {
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
        },
        yRangeFunction: function (range)
        {
          return {min: 0, max: Math.max(this.thresholdMillis, range.max)};
        }.bind(this)
      };

      this.chart = new SmoothieChart(chartOptions);
      this.canvas = document.createElement('canvas');
      this.canvas.height = chartHeight;
      this.$container.append(this.canvas);

      this.series = new TimeSeries();
      this.chart.addTimeSeries(this.series, seriesOptions);
      this.chart.streamTo(this.canvas, /*delayMs*/ 100);
      this.chart.options.horizontalLines.push({color:'#FF0000', lineWidth: 1, value: this.thresholdMillis});

      this.$fps = $('<div></div>', {'class':'fps'}).appendTo(this.$container);

      this.table = $('<table></table>', {'class':'timing-details'}).appendTo(this.$container);

      this.entryByLabel = {};

      this.subscription = DataProxy.subscribe(
        this.protocol,
        {
          json: true,
          onmessage: _.bind(this.onData, this)
        }
      );

      this.fpsInterval = setInterval(function ()
      {
        this.$fps.text(this.fpsCount ? this.fpsCount + ' FPS' : '');
        this.fpsCount = 0;
      }.bind(this), 1000);
    };

    TimingPane.prototype.unload = function()
    {
      this.chart.stop();
      this.$container.empty();
      this.subscription.close();

      clearInterval(this.fpsInterval);
      delete this.fpsInterval;
    };

    TimingPane.prototype.onData = function(data)
    {
      this.fpsCount++;
      var time = new Date().getTime();

      _.each(_.keys(data), function (key)
      {
        var millis = data[key];
        this.getOrCreateEntry(key).update(time, millis);
      }.bind(this));

      this.updateChart(time);
    };

    TimingPane.prototype.onResized = function(width, height)
    {
      this.canvas.width = width;
    };

    TimingPane.prototype.updateChart = function(time)
    {
      var totalTime = 0;

      _.each(_.values(this.entryByLabel), function(entry)
      {
        if (entry.time === time && entry.label.indexOf('/') === -1)
          totalTime += entry.millis;
      });

      this.series.append(time, totalTime);
    };

    TimingPane.prototype.getOrCreateEntry = function(label)
    {
      var entry = this.entryByLabel[label];
      if (!entry) {
        var row = $('<tr></tr>').appendTo(this.table);

        $('<td></td>').text(label).appendTo(row);
        var cellMillis = $('<td></td>', {'class': 'duration'}).appendTo(row),
            cellMaxMillis = $('<td></td>', {'class': 'max-duration'}).appendTo(row);

        entry = {
          label: label,
          update: function(timestamp, millis) {
            entry.time = timestamp;
            entry.millis = millis;
            if (!entry.maxMillis || entry.maxMillis < millis)
            {
              entry.maxMillis = millis;
              cellMaxMillis.text(millis.toFixed(3));
            }
            cellMillis.text(millis.toFixed(3));
          }
        };

        this.entryByLabel[label] = entry;
      }
      return entry;
    };

    return TimingPane;
  }
);
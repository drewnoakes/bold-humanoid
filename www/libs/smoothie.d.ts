interface ITimeSeriesOptions
{
    resetBounds?: boolean;
    resetBoundsInterval?: number;
}

interface ITimeSeriesPresentationOptions
{
    strokeStyle?: string;
    fillStyle?: string;
    lineWidth?: number;
}

declare class TimeSeries
{
    /**
     * Initialises a new <code>TimeSeries</code> with optional data options.
     *
     * Options are of the form (defaults shown):
     *
     * <pre>
     * {
     *   resetBounds: true,        // enables/disables automatic scaling of the y-axis
     *   resetBoundsInterval: 3000 // the period between scaling calculations, in millis
     * }
     * </pre>
     *
     * Presentation options for TimeSeries are specified as an argument to <code>SmoothieChart.addTimeSeries</code>.
     */
    constructor(options?: ITimeSeriesOptions);

    /**
     * Recalculate the min/max values for this <code>TimeSeries</code> object.
     *
     * This causes the graph to scale itself in the y-axis.
     */
    resetBounds();

    /**
     * Adds a new data point to the <code>TimeSeries</code>, preserving chronological order.
     *
     * @param timestamp the position, in time, of this data point
     * @param value the value of this data point
     * @param sumRepeatedTimeStampValues if <code>timestamp</code> has an exact match in the series, this flag controls
     * whether it is replaced, or the values summed (defaults to false.)
     */
    append(timestamp: number, value: number, sumRepeatedTimeStampValues?: boolean);

    dropOldData(oldestValidTime: number, maxDataSetLength: number);
}

interface IGridOptions
{
    /** The background colour of the chart. */
    fillStyle?: string;
    /** The pixel width of grid lines. */
    lineWidth?: number;
    /** Colour of grid lines. */
    stokeStyle?: string;
    /** Distance between vertical grid lines. */
    millisPerLine?: number;
    /** Controls whether grid lines are 1px sharp, or softened. */
    sharpLines?: boolean;
    /** Number of vertical sections marked out by horizontal grid lines. */
    verticalSections?: number;
    /** Whether the grid lines trace the border of the chart or not. */
    borderVisible?: boolean;
}

interface ILabelOptions
{
    /** Enables/disables labels showing the min/max values. */
    disabled?: boolean;
    /** Colour for text of labels. */
    fillStyle?: string;
    fontSize?: number;
    fontFamily?: string;
    precision?: number;
}

interface IRange { min: number; max: number }

interface IHorizontalLine
{
    value?: number;
    color?: string;
    lineWidth?: number;
}

interface IChartOptions
{
    /** Specify to clamp the lower y-axis to a given value. */
    minValue?: number;
    /** Specify to clamp the upper y-axis to a given value. */
    maxValue?: number;
    /** Allows proportional padding to be added above the chart. for 10% padding, specify 1.1. */
    maxValueScale?: number;
    yRangeFunction?: (range:IRange)=>IRange;
    /** Controls the rate at which y-value zoom animation occurs. */
    scaleSmoothing?: number;
    /** Sets the speed at which the chart pans by. */
    millisPerPixel?: number;
    maxDataSetLength?: number;
    /** One of: 'bezier', 'linear', 'step' */
    interpolation?: string;
    /** Optional function to format time stamps for bottom of chart. You may use <code>SmoothieChart.timeFormatter</code>, or your own/ */
    timestampFormatter?: (date:Date)=>string;
    horizontalLines?: IHorizontalLine[];

    grid?: IGridOptions;

    labels?: ILabelOptions;
}

/**
 * Initialises a new <code>SmoothieChart</code>.
 *
 * Options are optional and may be sparsely populated. Just specify the values you
 * need and the rest will be given sensible defaults.
 */
declare class SmoothieChart
{
    options: IChartOptions;

    constructor(chartOptions?: IChartOptions);

    /**
     * Adds a <code>TimeSeries</code> to this chart, with optional presentation options.
     */
    addTimeSeries(series: TimeSeries, seriesOptions?: ITimeSeriesPresentationOptions);

    /**
     * Removes the specified <code>TimeSeries</code> from the chart.
     */
    removeTimeSeries(series: TimeSeries);

    /**
     * Gets render options for the specified <code>TimeSeries</code>.
     *
     * As you may use a single <code>TimeSeries</code> in multiple charts with different formatting in each usage,
     * these settings are stored in the chart.
     */
    getTimeSeriesOptions(timeSeries: TimeSeries): ITimeSeriesPresentationOptions;

    /**
     * Brings the specified <code>TimeSeries</code> to the top of the chart. It will be rendered last.
     */
    bringToFront(timeSeries: TimeSeries);

    /**
     * Instructs the <code>SmoothieChart</code> to start rendering to the provided canvas, with specified delay.
     *
     * @param canvas the target canvas element
     * @param delayMillis an amount of time to wait before a data point is shown. This can prevent the end of the series
     * from appearing on screen, with new values flashing into view, at the expense of some latency.
     */
    streamTo(canvas: HTMLCanvasElement, delayMillis: number);

    /**
     * Starts the animation of this chart. Called by <code>streamTo</code>.
     */
    start();

    /**
     * Stops the animation of this chart.
     */
    stop();

    updateValueRange();

    render(canvas?: HTMLCanvasElement, time?: number);
}

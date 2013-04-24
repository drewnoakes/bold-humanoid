/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'Protocols'
    ],
    function (DataProxy, Protocols)
    {
        var ControlClient = {};

        var callbacksByFamily = {};

        var dataByFamily,
            subscription;

        var onControlData = function(data)
        {
            // Store the control data
            dataByFamily = data;

            // Raise any queued callbacks
            _.each(_.keys(data), function(family)
            {
                if (callbacksByFamily[family])
                {
                    // Invoke all pending callbacks
                    _.each(callbacksByFamily[family], function (callback)
                    {
                        callback(data[family]);
                    });
                    // Remove the callbacks now they've been satisfied
                    callbacksByFamily[family].length = 0;
                }
            });
        };

        ControlClient.connect = function()
        {
            subscription = DataProxy.subscribe(
                Protocols.control,
                {
                    json: true,
                    onmessage: _.bind(onControlData, this)
                }
            );
        };

        ControlClient.withData = function(family, callback)
        {
            if (dataByFamily)
            {
                // We have data, so provide it immediately
                callback(dataByFamily[family]);
            }
            else
            {
                // No data yet, so store the callback
                if (typeof(callbacksByFamily[family]) === 'undefined')
                {
                    callbacksByFamily[family] = [];
                }

                callbacksByFamily[family].push(callback);
            }
        };

        ControlClient.sendCommand = function(family, id, value)
        {
            var command = {
                family: family,
                id: id
            };

            if (typeof(value) !== 'undefined')
            {
                command.value = value;
            }

            console.log('Sending command', command);

            subscription.send(JSON.stringify(command));
        };

        return ControlClient;
    }
);
/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/WebSocketFactory'
    ],
    function (WebSocketFactory)
    {
        'use strict';

        var dataByProtocol = {};

        var DataProxy = {};

        var proxyEvent = function (protocolData, eventName)
        {
            return function (msg)
            {
                for (var i = 0; i < protocolData.callbacks.length; i++) {
                    var c = protocolData.callbacks[i][eventName];
                    if (typeof(c) === 'function') {
                        // TODO use arguments instead of 'msg'?
                        c(msg);
                    }
                }
            }
        };

        DataProxy.subscribe = function (protocol, callbacks)
        {
            var protocolData = dataByProtocol[protocol];

            if (!protocolData) {
                protocolData = { callbacks: [callbacks] };
                dataByProtocol[protocol] = protocolData;
                protocolData.socket = WebSocketFactory.open(protocol);
                protocolData.socket.onmessage = proxyEvent(protocolData, 'onmessage');
                protocolData.socket.onerror = proxyEvent(protocolData, 'onerror');
            }
            else {
                protocolData.callbacks.push(callbacks);
            }

            return {
                close: function ()
                {
                    for (var i = 0; i < protocolData.callbacks.length; i++) {
                        var callback = protocolData.callbacks[i];
                        if (callback === callbacks) {
                            // remove this item from the queue
                            protocolData.callbacks.splice(i, 1);

                            if (protocolData.callbacks.length === 0) {
                                // no one is using this socket any more, so close it down
                                protocolData.socket.close();

                                delete dataByProtocol[protocol];
                            }
                        }
                    }
                }
            }
        };

        DataProxy.parseFloats = function (s)
        {
            var bits = s.split('|');
            var floats = [];
            for (var i = 0; i < bits.length; i++) {
                floats.push(parseFloat(bits[i]));
            }
            return floats;
        };

        return DataProxy;
    }
);
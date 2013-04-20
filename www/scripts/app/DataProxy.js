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
                var parsed;
                for (var i = 0; i < protocolData.clients.length; i++) {
                    var client = protocolData.clients[i];
                    var c = client[eventName];
                    if (typeof(c) === 'function') {
                        if (client.json) {
                            if (!parsed && msg.data) {
                                parsed = JSON.parse(msg.data);
                            }
                            c(parsed);
                        } else {
                            // TODO use arguments instead of 'msg'?
                            c(msg);
                        }
                    }
                }
            }
        };

        DataProxy.subscribe = function (protocol, options)
        {
            var protocolData = dataByProtocol[protocol];

            if (!protocolData) {
                protocolData = { clients: [options] };
                dataByProtocol[protocol] = protocolData;
                protocolData.socket = WebSocketFactory.open(protocol);
                protocolData.socket.onmessage = proxyEvent(protocolData, 'onmessage');
                protocolData.socket.onerror = proxyEvent(protocolData, 'onerror');
            }
            else {
                protocolData.clients.push(options);
            }

            return {
                send: function (message)
                {
                    protocolData.socket.send(message);
                },
                close: function ()
                {
                    for (var i = 0; i < protocolData.clients.length; i++) {
                        var callback = protocolData.clients[i];
                        if (callback === options) {
                            // remove this client
                            protocolData.clients.splice(i, 1);

                            if (protocolData.clients.length === 0) {
                                // no one is using this socket any more, so close it down
                                WebSocketFactory.close(protocol);

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
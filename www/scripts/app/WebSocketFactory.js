/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
      'scripts/app/Constants'
    ],
    function(Constants)
    {
        'use strict';

        var getWebSocketUrl = function ()
        {
            var u = document.URL;
            if (u.substring(0, 4) === "http")
                u = u.substr(7);
            if (u.indexOf(":") != -1)
              u = u.substring(0, u.indexOf(":"));
            return "ws://" + u.split('/')[0] + ":" + Constants.webSocketPort;
        };

        var indicatorByProtocol = {};

        //noinspection UnnecessaryLocalVariableJS

        var WebSocketFactory = {
            open: function(protocol)
            {
                var webSocketUrl = getWebSocketUrl();

                var socket = typeof MozWebSocket !== 'undefined'
                    ? new MozWebSocket(webSocketUrl, protocol)
                    : new WebSocket(webSocketUrl, protocol);

                // Reuse the indicator, in case we are re-connecting
                var connectionIndicator = indicatorByProtocol[protocol];
                if (!connectionIndicator) {
                    connectionIndicator = $('<div></div>', {title: protocol});
                    connectionIndicator.appendTo($('#socket-connections'));
                    indicatorByProtocol[protocol] = connectionIndicator;
                }

                connectionIndicator.attr({'class': 'connection-indicator connecting'});

                socket.onopen = function ()
                {
                    connectionIndicator.attr({'class': 'connection-indicator connected'});
                };

                socket.onclose = function ()
                {
                    connectionIndicator.attr({'class': 'connection-indicator disconnected'});
                };

                socket.onerror = function (e)
                {
                    connectionIndicator.attr({'class': 'connection-indicator error'});
                };

                return socket;
            }
        };

        return WebSocketFactory;
    }
);
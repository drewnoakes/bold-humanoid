/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
      'scripts/app/Settings'
    ],
    function(Settings)
    {
        'use strict';

        var indicatorByProtocol = {};
        var socketByProtocol = {};

        //noinspection UnnecessaryLocalVariableJS

        var WebSocketFactory = {
            open: function(protocol)
            {
                var socket = typeof MozWebSocket !== 'undefined'
                    ? new MozWebSocket(Settings.webSocketUrl, protocol)
                    : new WebSocket(Settings.webSocketUrl, protocol);

                socketByProtocol[protocol] = socket;

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
            },
            close: function (protocol)
            {
                socketByProtocol[protocol].close();
                delete socketByProtocol[protocol];
                indicatorByProtocol[protocol].remove();
                delete indicatorByProtocol[protocol];
            }
        };

        return WebSocketFactory;
    }
);
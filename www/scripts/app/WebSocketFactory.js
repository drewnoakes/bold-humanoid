/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function()
    {
        'use strict';

        var getWebSocketUrl = function ()
        {
            var u = document.URL;
            if (u.substring(0, 4) === "http")
                u = u.substr(7);
            return "ws://" + u.split('/')[0];
        };

        //noinspection UnnecessaryLocalVariableJS

        var WebSocketFactory = {
            open: function(protocol)
            {
                var webSocketUrl = getWebSocketUrl();

                var socket = typeof MozWebSocket !== "undefined"
                    ? new MozWebSocket(webSocketUrl, protocol)
                    : new WebSocket(webSocketUrl, protocol);

                var connectionIndicator = $('<div></div>')
                    .addClass('connection-indicator connecting')
                    .attr({title: protocol});

                connectionIndicator.appendTo($('#socket-connections'));

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
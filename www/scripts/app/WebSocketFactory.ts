/**
 * @author Drew Noakes http://drewnoakes.com
 */

import Settings = require('Settings');

var indicatorByProtocol = {};
var socketByProtocol = {};

declare class MozWebSocket
{
    constructor(url:string, protocol:string);
}

var elementContainer = document.querySelector('#socket-connections');

var WebSocketFactory = {
    open: function(protocol)
    {
        var socket: WebSocket = typeof MozWebSocket !== 'undefined'
            ? <WebSocket>new MozWebSocket(Settings.webSocketUrl, protocol)
            : new WebSocket(Settings.webSocketUrl, protocol);

        socketByProtocol[protocol] = socket;

        // Reuse the indicator, in case we are re-connecting
        var connectionIndicator = indicatorByProtocol[protocol];
        if (!connectionIndicator) {
            connectionIndicator = document.createElement('div');
            connectionIndicator.title = protocol;
            elementContainer.appendChild(connectionIndicator);
            indicatorByProtocol[protocol] = connectionIndicator;
        }

        connectionIndicator.className = 'connection-indicator connecting';

        socket.onopen = () => connectionIndicator.className = 'connection-indicator connected';
        socket.onclose = () => connectionIndicator.className = 'connection-indicator disconnected';
        socket.onerror = (e) => connectionIndicator.className = 'connection-indicator error';

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

export = WebSocketFactory;

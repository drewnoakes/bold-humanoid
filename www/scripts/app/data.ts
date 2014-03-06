/**
 * @author Drew Noakes http://drewnoakes.com
 */

import Settings = require('Settings');

var indicatorByProtocol = {};
var socketByProtocol: {[protocol:string]:WebSocket} = {};

declare class MozWebSocket
{
    constructor(url: string, protocol: string);
}

var elementContainer = document.querySelector('#socket-connections');

function openWebSocket(protocol: string) : WebSocket
{
    if (!protocol || protocol === '')
        throw new Error("Invalid protocol: " + protocol);
    if (socketByProtocol[protocol] !== undefined)
        throw new Error("WebSocket already open for protocol: " + protocol);

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
    socket.onerror = e => connectionIndicator.className = 'connection-indicator error';

    return socket;
}

function closeWebSocket(protocol: string): void
{
    socketByProtocol[protocol].close();
    delete socketByProtocol[protocol];
    indicatorByProtocol[protocol].remove();
    delete indicatorByProtocol[protocol];
}

interface IClient
{
    parseJson: boolean;
    onmessage: (data: any)=>void;
    onerror?: (err: ErrorEvent)=>void;
}

interface IProtocol
{
    name: string;
    clients: IClient[];
    socket: WebSocket;
}

var proxyEvent = (protocol: IProtocol, eventName: string, unsubscribe: ()=>void) =>
{
    return msg =>
    {
        var parsed;
        for (var i = 0; i < protocol.clients.length; i++) {
            var client = protocol.clients[i];
            var callback = client[eventName];
            console.assert(typeof(callback) === 'function');
            try {
                if (client.parseJson) {
                    if (!parsed && msg.data) {
                        parsed = JSON.parse(msg.data);
                    }
                    callback(parsed);
                } else {
                    // TODO use arguments instead of 'msg'?
                    callback(msg);
                }
            } catch(ex) {
                console.error(protocol.name, 'subscription handler raised an error. Forcing unsubscribe.', ex);
                unsubscribe();
                throw ex;
            }
        }
    }
};

var dataByProtocol: {[name: string]: IProtocol} = {};

export interface ISubscriptionOptions<TData>
{
    onmessage: (data: TData)=>void;
    /** Called with WebSocket errors. */
    onerror?: (data: ErrorEvent)=>void;
    parseJson?: boolean;
}

export class Subscription<TData>
{
    private protocol: IProtocol;
    private client: IClient;

    constructor(private protocolName: string, private options: ISubscriptionOptions<TData>)
    {
        this.protocol = dataByProtocol[protocolName];

        if (!this.protocol) {
            // First client of this protocol
            var socket = openWebSocket(protocolName);
            this.protocol = { name: protocolName, clients: [], socket: socket };
            dataByProtocol[protocolName] = this.protocol;
            this.protocol.socket.onmessage = proxyEvent(this.protocol, 'onmessage', () => this.close());
            this.protocol.socket.onerror = proxyEvent(this.protocol, 'onerror', () => this.close());
        }

        this.client = {
            // default to 'true'
            parseJson: typeof(options.parseJson) === 'boolean' ? options.parseJson : true,
            onmessage: options.onmessage,
            onerror: options.onerror
        };

        this.protocol.clients.push(this.client);

        // Ensure our functions are always called with this instance.
        this.send = this.send.bind(this);
        this.close = this.close.bind(this);
    }

    public send(message)
    {
        this.protocol.socket.send(message);
    }

    public close()
    {
        // Search for our client
        for (var i = 0; i < this.protocol.clients.length; i++)
        {
            if (this.protocol.clients[i] === this.client)
            {
                // Remove this client
                this.protocol.clients.splice(i, 1);

                if (this.protocol.clients.length === 0)
                {
                    // No one is using this socket any more, so close it down
                    closeWebSocket(this.protocol.name);

                    delete dataByProtocol[this.protocol.name];
                }

                break;
            }
        }
    }
}

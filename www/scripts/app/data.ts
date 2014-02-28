/**
 * @author Drew Noakes http://drewnoakes.com
 */

import WebSocketFactory = require('WebSocketFactory');

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

var proxyEvent = (protocol: IProtocol, eventName: string) =>
{
    return msg =>
    {
        var parsed;
        for (var i = 0; i < protocol.clients.length; i++) {
            var client = protocol.clients[i];
            var c = client[eventName];
            if (typeof(c) === 'function') {
                if (client.parseJson) {
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

var dataByProtocol: {[name: string]: IProtocol} = {};

export interface ISubscriptionOptions<TData>
{
    onmessage: (data: TData)=>void;
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
            var socket = WebSocketFactory.open(protocolName);
            this.protocol = { name: protocolName, clients: [], socket: socket };
            dataByProtocol[protocolName] = this.protocol;
            this.protocol.socket.onmessage = proxyEvent(this.protocol, 'onmessage');
            this.protocol.socket.onerror = proxyEvent(this.protocol, 'onerror');
        }

        this.client = {
            // default to 'true'
            parseJson: typeof(options.parseJson) === 'boolean' ? options.parseJson : true,
            onmessage: options.onmessage,
            onerror: options.onerror
        };

        this.protocol.clients.push(this.client);
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
                    // No one is using this socket anymore, so close it down
                    WebSocketFactory.close(this.protocol.name);

                    delete dataByProtocol[this.protocol.name];
                }

                break;
            }
        }
    }
}

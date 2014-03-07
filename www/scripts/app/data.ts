/**
 * @author Drew Noakes http://drewnoakes.com
 */

import Settings = require('Settings');

declare class MozWebSocket
{
    constructor(url: string, protocol: string);
}

// GLOBAL STATE

var elementContainer = document.querySelector('#connections .indicators');
var protocolByName: {[name: string]: Protocol} = {};

// CLIENT

interface IClient
{
    parseJson: boolean;
    onmessage: (data: any)=>void;
    onerror?: (err: ErrorEvent)=>void;
}

// PROTOCOL

class Protocol
{
    public clients: IClient[] = [];
    public socket: WebSocket;
    private indicator: HTMLDivElement;

    constructor(public protocolName: string)
    {
        console.assert(!protocolName[protocolName]);

        this.indicator = document.createElement('div');
        this.indicator.title = protocolName;
        this.indicator.className = 'connection-indicator connecting';

        elementContainer.appendChild(this.indicator);
    }

    public addClient(client: IClient)
    {
        if (this.clients.length === 0)
        {
            // This is the first client, so open the socket
            this.createSocket();
        }

        this.clients.push(client);
        this.indicator.title = this.protocolName + ' (' + this.clients.length + ' client' + (this.clients.length==1?')':'s)');
    }

    private createSocket()
    {
        if (this.socket)
        {
            console.assert(this.socket.readyState === WebSocket.CLOSED);
            this.socket.close();
        }

        this.socket = typeof MozWebSocket !== 'undefined'
            ? <WebSocket>new MozWebSocket(Settings.webSocketUrl, this.protocolName)
            : new WebSocket(Settings.webSocketUrl, this.protocolName);

        // Wire up the indicator
        this.indicator.className = 'connection-indicator connecting';
        this.socket.onopen = () => this.indicator.className = 'connection-indicator connected';
        this.socket.onclose = () => this.indicator.className = 'connection-indicator disconnected';
        this.socket.onerror = e => this.indicator.className = 'connection-indicator error';

        // The callback logic for errors and messages is very similar.
        // In both cases, just proxy the argument onwards.
        this.socket.onerror = this.proxyEventToClients('onerror');
        this.socket.onmessage = this.proxyEventToClients('onmessage');
    }

    public removeClient(client: IClient)
    {
        // Find the client
        for (var i = 0; i < this.clients.length; i++)
        {
            if (this.clients[i] === client)
            {
                // Remove the client
                this.clients.splice(i, 1);

                if (this.clients.length === 0)
                {
                    // No one is using this socket any more, so close it down
                    this.socket.close();
                    delete this.socket;
                }

                break;
            }
        }
    }
    public disconnect()
    {
        this.socket.close();
    }

    public reconnect()
    {
        if (this.socket.readyState === WebSocket.CLOSED)
        {
           this.createSocket();
        }
    }

    private proxyEventToClients(eventName: string): (arg:any)=>void
    {
        return msg =>
        {
            var parsed;
            for (var i = 0; i < this.clients.length; i++) {
                var client = this.clients[i];
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
                    console.error(this.protocolName, 'subscription handler raised an error. Closing WebSocket.', ex);
                    this.socket.close();
                    throw ex;
                }
            }
        }
    }
}

// EXPORTS

export function disconnectAll()
{
    _.each<Protocol>(_.values(protocolByName), protocol => protocol.disconnect());
}

export function reconnectAll()
{
    _.each<Protocol>(_.values(protocolByName), protocol => protocol.reconnect());
}

export interface ISubscriptionOptions<TData>
{
    onmessage: (data: TData)=>void;
    /** Called with WebSocket errors. */
    onerror?: (data: ErrorEvent)=>void;
    parseJson?: boolean;
}
export class Subscription<TData>
{
    private protocol: Protocol;
    private client: IClient;

    constructor(protocolName: string, options: ISubscriptionOptions<TData>)
    {
        this.client = {
            // default to 'true'
            parseJson: typeof(options.parseJson) === 'boolean' ? options.parseJson : true,
            onmessage: options.onmessage,
            onerror: options.onerror
        };

        // Look up or create Protocol
        this.protocol = protocolByName[protocolName];
        if (!this.protocol)
            this.protocol = protocolByName[protocolName] = new Protocol(protocolName);

        // Add another client
        this.protocol.addClient(this.client);

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
        this.protocol.removeClient(this.client);
    }
}

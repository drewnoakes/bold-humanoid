/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/jquery.d.ts" />

import constants = require('constants');

declare class MozWebSocket
{
    constructor(url: string, protocol: string);
}

// GLOBAL STATE

var elementContainer = document.querySelector('#connections .indicators');
var protocols: Protocol[] = [];
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
        this.indicator.style.display = 'none';

        elementContainer.appendChild(this.indicator);

        protocols.push(this);
    }

    public addClient(client: IClient)
    {
        if (this.clients.length === 0)
        {
            // This is the first client, so open the socket
            this.createSocket();
            $(this.indicator).fadeIn();
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
            ? <WebSocket>new MozWebSocket(constants.webSocketUrl, this.protocolName)
            : new WebSocket(constants.webSocketUrl, this.protocolName);

        // Wire up the indicator
        this.indicator.className = 'connection-indicator connecting';
        this.socket.onopen = () => {
            this.indicator.className = 'connection-indicator connected';
            raiseConnectionChanged();
        };
        this.socket.onclose = () => {
            this.indicator.className = 'connection-indicator disconnected';
            raiseConnectionChanged();
        };
        this.socket.onerror = e => {
            this.indicator.className = 'connection-indicator error';
            raiseConnectionChanged();
        };

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
                    $(this.indicator).fadeOut();
                }

                break;
            }
        }
    }

    public disconnect()
    {
        if (this.socket.readyState !== WebSocket.CLOSED)
            this.socket.close();
    }

    public reconnect()
    {
        if (this.clients.length === 0)
            return;

        if (this.socket.readyState === WebSocket.CLOSED)
           this.createSocket();
    }

    public isConnected() : boolean
    {
        return this.socket && this.socket.readyState == WebSocket.OPEN;
    }

    private proxyEventToClients(eventName: string): (arg:any)=>void
    {
        return msg =>
        {
            var parsed: any;
            for (var i = 0; i < this.clients.length; i++) {
                var client = this.clients[i];
                var callback = client[eventName];
                if (!callback)
                    continue;
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
                    if (ex instanceof SyntaxError)
                        console.error(this.protocolName, 'Error parsing message. Closing WebSocket.', ex, msg);
                    else
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
    console.log('Disconnecting all protocols');
    _.each<Protocol>(protocols, protocol => protocol.disconnect());
}

export function reconnectAll()
{
    console.log('Reconnecting all protocols');
    _.each<Protocol>(protocols, protocol => protocol.reconnect());
}

export function isAllDisconnected()
{
    return _.all<Protocol>(protocols, protocol => !protocol.isConnected());
}

var callbacks: {():void}[] = [];

export function onConnectionChanged(callback: ()=>void)
{
    callbacks.push(callback);
}

var raiseConnectionChanged = function ()
{
    _.each<()=>void>(callbacks, callback => callback());
};

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

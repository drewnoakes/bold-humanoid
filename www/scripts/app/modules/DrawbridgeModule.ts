/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import constants = require('constants');
import control = require('control');
import data = require('data');
import DOMTemplate = require('DOMTemplate');
import Module = require('Module');
import state = require('state');
import util = require('util');

var template = DOMTemplate.forId('drawbridge-module-template');

enum TeamColour
{
    Cyan = 1,
    Magenta = 2
}

interface IDrawbridgeData
{
    unum: number;
    team: number;
    col: TeamColour;

    uptime: number;

    activity: string;
    role: string;
    status: string;
    ver: string;

    agent: {
        ball: number[];
        goals: number[][];
    };

    game: {
        mode: string;
        age: number;
    };

    hw: {
        power: boolean;
        volt: number;
        temps: number[];
    };

    teammates: {
        unum: number;
        ms: number;
    }[];

    options: string[];
}

class DrawbridgeModule extends Module
{
    private socket: WebSocket;
    private containerByUnum: {[unum:string]:HTMLDivElement} = {};

    constructor()
    {
        super('drawbridge', 'drawbridge', {fullScreen: true});
    }

    // TODO catch and display errors

    public load(width: number)
    {
        this.socket = new WebSocket("ws://localhost:8888", "drawbridge");
        this.socket.onmessage = msg => this.onMessage(JSON.parse(msg.data));
        this.closeables.add(() => this.socket.close());
    }

    private onMessage(data: IDrawbridgeData)
    {
        var container = this.containerByUnum[data.unum];

        if (container != null)
        {
            util.clearChildren(container);
        }
        else
        {
            container = document.createElement('div');
            this.containerByUnum[data.unum.toString()] = container;
            this.element.appendChild(container);
        }

        container.appendChild(template.create(data));
    }
}

export = DrawbridgeModule;

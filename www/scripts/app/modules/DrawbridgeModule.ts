/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import color = require('color');
import constants = require('constants');
import control = require('control');
import data = require('data');
import DOMTemplate = require('DOMTemplate');
import math = require('util/math');
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
    host: string;
    name: string;
    ver: string;
    uptime: number;

    activity: string;
    role: string;
    status: string;

    fpsThink: number;
    fpsMotion: number;

    agent?: {
        ball: number[];
        goals: number[][];
    };

    game?: {
        mode: string;
        age: number;
    };

    hw?: {
        power: boolean;
        volt: number;
        temps: number[];
    };

    teammates?: {
        unum: number;
        ms: number;
    }[];

    options: string[];

    fsms: {
        fsm: string;
        state: string
    }[]
}

interface IPlayerViewModel extends IDrawbridgeData
{
    uptimeColour: string;
    voltageColour: string;
    fpsThinkColour: string;
    fpsMotionColour: string;
    temperatures: {
        degrees: number;
        colour: string;
        title: string;
    }[];
    maxTemperature: number;
    maxTemperatureColour: string;
    maxTemperatureJoint: string;
    gameControllerAgeColour: string;
}

var green = new color.Rgb(0, 0.8, 0),
    red = new color.Rgb(0.8, 0, 0);

class DrawbridgeModule extends Module
{
    private socket: WebSocket;
    private containerById: {[id:string]:HTMLDivElement} = {};

    constructor()
    {
        super('drawbridge', 'drawbridge', {fullScreen: true});
    }

    // TODO catch and display problems such as stopped messages, temperatures, different SHA1
    // TODO sort UI elements
    // TODO good fit for D3?

    public load(width: number)
    {
        this.socket = new WebSocket("ws://localhost:8888", "drawbridge");
        this.socket.onmessage = msg => this.onMessage(JSON.parse(msg.data));
        this.closeables.add(() => this.socket.close());
    }

    private onMessage(data: IDrawbridgeData)
    {
        var container = this.containerById[DrawbridgeModule.getPlayerId(data)];

        if (container != null)
        {
            util.clearChildren(container);
        }
        else
        {
            container = document.createElement('div');
            container.className = 'player-tile';
            container.dataset['sorthint'] = (data.team * 100 + data.unum).toString();
            this.containerById[DrawbridgeModule.getPlayerId(data)] = container;
            this.element.appendChild(container);
        }

        var textColour = new color.Rgb(constants.isNightModeActive.getValue() ? "#777777": "#000000"),
            noteColour = new color.Rgb(constants.isNightModeActive.getValue() ? "#444444": "#555555");

        var viewModel: IPlayerViewModel = <any>data;

        viewModel.uptimeColour = color.blend(green, textColour, math.clamp(data.uptime / 30, 0, 1));
        viewModel.voltageColour = color.blend(red, textColour, math.clamp((data.hw.volt - 11) / 1.5, 0, 1));

        viewModel.fpsThinkColour = color.blend(textColour, red, math.clamp(Math.abs(data.fpsThink - 30) / 5, 0, 1));
        viewModel.fpsMotionColour = color.blend(textColour, red, math.clamp(Math.abs(data.fpsMotion - 125) / 10, 0, 1));
        viewModel.fpsThink = <any>viewModel.fpsThink.toFixed(1);
        viewModel.fpsMotion = <any>viewModel.fpsMotion.toFixed(1);

        if (data.game)
            viewModel.gameControllerAgeColour = color.blend(noteColour, red, math.clamp((data.game.age - 500) / 5000, 0, 1));

        // Process temperature data for display
        viewModel.temperatures = [];
        viewModel.maxTemperature = 0;
        for (var i = 0; i < data.hw.temps.length; i++)
        {
            var temp = data.hw.temps[i],
                col = color.blend(textColour, red, math.clamp((temp - 45) / 20, 0, 1)),
                title = constants.jointNiceNames[i + 1];

            viewModel.temperatures.push({
                degrees: temp,
                colour: col,
                title: title
            });

            if (temp > viewModel.maxTemperature)
            {
                viewModel.maxTemperature = temp;
                viewModel.maxTemperatureColour = col;
                viewModel.maxTemperatureJoint = title;
            }
        }

        // remove some obvious/distracting option names
        for (var i = 0; i < viewModel.options.length; i++)
        {
            switch (viewModel.options[i])
            {
                case "win":
                case "dispatch":
                case "striker":
                case "keeper":
                    viewModel.options.splice(i, 1);
                    i--;
                    break;
            }
        }

        container.appendChild(template.create(viewModel));

        // Ensure tiles are sorted on screen
        util.sortChildren(this.element, tile => parseInt((<HTMLElement>tile).dataset['sorthint']));
    }

    private static getPlayerId(data: IDrawbridgeData): string
    {
        return data.team.toString() + "-" + data.unum.toString();
    }
}

export = DrawbridgeModule;

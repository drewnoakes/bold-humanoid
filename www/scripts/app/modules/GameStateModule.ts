/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import data = require('data');
import constants = require('constants');
import DOMTemplate = require('DOMTemplate');
import Module = require('Module');
import state = require('state');
import util = require('util');

var moduleTemplate = new DOMTemplate('game-module-template');

var padLeft = (nr, n, str) =>
{
    return new Array(n - String(nr).length + 1).join(str || '0') + nr;
};

class GameStateModule extends Module
{
    private element: HTMLDivElement;

    constructor()
    {
        super('game', 'game');
    }

    public load(element: HTMLDivElement)
    {
        this.element = element;
        this.closeables.add(new data.Subscription<state.Game>(
            constants.protocols.gameState,
            {
                onmessage: this.onGameState.bind(this)
            }
        ));
    }

    private onGameState(data: state.Game)
    {
        var templateData: any = data;

        templateData.timeString = Math.floor(data.secondsRemaining / 60) + ':' + padLeft(data.secondsRemaining % 60, 2, '0');

        var amendTeam = (team: any) =>
        {
            for (var i = 0; i < team.players.length; i++)
            {
                var p = team.players[i];
                p.num = i;
                if (typeof(p.penaltySecondsRemaining) !== 'undefined')
                {
                    p.isDone = p.penaltySecondsRemaining === 0;
                }
            }
        };

        amendTeam(templateData.team1);
        amendTeam(templateData.team2);

        util.clearChildren(this.element);
        this.element.appendChild(moduleTemplate.create(templateData));
    }
}

export = GameStateModule;

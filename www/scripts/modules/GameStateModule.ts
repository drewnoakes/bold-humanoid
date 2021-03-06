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

var moduleTemplate = DOMTemplate.forId('game-module-template');

var padLeft = (nr, n, str) =>
{
    return new Array(n - String(nr).length + 1).join(str || '0') + nr;
};

interface ExtendedTeamData extends state.TeamData
{
    players: ExtendedPlayerData[];
}

interface ExtendedPlayerData extends state.PlayerData
{
    num?: number;
    showPenalty?: boolean;
    isDone?: boolean;
}

interface ITemplateData extends state.Game
{
    timeString?: string;
    secondaryTimeString?: string;
    myTeam: ExtendedTeamData;
    opponentTeam: ExtendedTeamData;
}

class GameStateModule extends Module
{
    constructor()
    {
        super('game', 'game');
    }

    public load(width: number)
    {
        this.closeables.add(new data.Subscription<state.Game>(
            constants.protocols.gameState,
            {
                onmessage: this.onGameState.bind(this)
            }
        ));
    }

    private static formatTimeString(seconds: number): string
    {
        return Math.floor(seconds / 60) + ':' + padLeft(Math.abs(seconds % 60), 2, '0')
    }

    private onGameState(data: state.Game)
    {
        var templateData: ITemplateData = util.clone(data);

        templateData.timeString = GameStateModule.formatTimeString(data.secondsRemaining);
        templateData.secondaryTimeString = GameStateModule.formatTimeString(data.secondsSecondaryTime);

        var amendTeam = (team: ExtendedTeamData) =>
        {
            for (var i = 0; i < team.players.length; i++)
            {
                var p: ExtendedPlayerData = team.players[i];
                p.num = i;
                if (typeof(p.penaltySecondsRemaining) !== 'undefined')
                {
                    p.showPenalty = p.penalty !== "Substitute";
                    p.isDone = p.penaltySecondsRemaining === 0;
                }
                else
                {
                    p.showPenalty = false;
                }
            }
        };

        amendTeam(templateData.myTeam);
        amendTeam(templateData.opponentTeam);

        util.clearChildren(this.element);
        this.element.appendChild(moduleTemplate.create(templateData));
    }
}

export = GameStateModule;

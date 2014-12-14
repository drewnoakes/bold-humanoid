/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import constants = require('constants');
import control = require('control');
import data = require('data');
import DOMTemplate = require('DOMTemplate');
import math = require('util/math');
import Module = require('Module');
import state = require('state');
import util = require('util');

var template = DOMTemplate.forId('team-module-template');

class TeamModule extends Module
{
    constructor()
    {
        super('team', 'team', {fullScreen: true});
    }

    public load(width: number)
    {
        this.closeables.add(new data.Subscription<state.Team>(
            constants.protocols.teamState,
            {
                onmessage: this.onTeamState.bind(this),
                onerror: e => this.element.style.opacity = '0.4'
            }
        ));
    }

    private onTeamState(data: state.Team)
    {
        util.clearChildren(this.element);

        if (data.players.length === 0)
            return;

        var newestTime = _.max<number>(_.map(data.players, player => player.updateTime));
        var myTeamNumber = _.findWhere<state.PlayerData>(data.players, player => player.isMe).team;

        data.players.sort((a,b) => a.unum < b.unum ? -1 : a.unum === b.unum ? 0 : 1);

        var playerViewModels = _.map<state.PlayerData,any>(
            data.players,
            player => ({
                unum: player.unum,
                team: player.team,
                activity: state.getPlayerActivityName(player.activity),
                state: state.getPlayerStatusName(player.status),
                role: state.getPlayerRoleName(player.role),
                pos: '[' + (player.pos[0] != null ? player.pos[0].toFixed(2) : 'null') + ', '
                         + (player.pos[1] != null ? player.pos[1].toFixed(2) : 'null') + '] '
                         + (player.pos[2] != null ? Math.round(math.radToDeg(player.pos[2])).toString() : 'null') + '°',
                posConfidence: player.posConfidence.toFixed(2),
                ballRelative: !player.ballRelative.length ? '-' : '[' + player.ballRelative[0].toFixed(2) + ', ' + player.ballRelative[1].toFixed(2) + ']',
                ballDistance: !player.ballRelative.length ? '-' : Math.sqrt(Math.pow(player.ballRelative[0], 2) + Math.pow(player.ballRelative[1], 2)).toFixed(2),
                age: (newestTime - player.updateTime).toString(),

                clazz: (player.isMe ? 'me ' : '')
                    + (newestTime - player.updateTime > 2000 ? 'old ' : '')
                    + (player.team === myTeamNumber ? 'teammate' : 'opponent')
            }));

        this.element.appendChild(template.create(playerViewModels));
    }
}

export = TeamModule;

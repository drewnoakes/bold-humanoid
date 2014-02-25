/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'constants',
        'DOMTemplate'
    ],
    function(DataProxy, constants, DOMTemplate)
    {
        'use strict';

        var moduleTemplate = new DOMTemplate('game-module-template');

        var padLeft = function (nr, n, str)
        {
            return new Array(n - String(nr).length + 1).join(str || '0') + nr;
        };

        var GameStateModule = function()
        {
            this.$container = $('<div></div>');
            this.options = {};

            this.title = 'game';
            this.id = 'game';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container
                }
            ];
        };
        
        GameStateModule.prototype.load = function()
        {
            this.subscription = DataProxy.subscribe(
                constants.protocols.gameState,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );
        };

        GameStateModule.prototype.unload = function()
        {
            this.subscription.close();
        };

        GameStateModule.prototype.onData = function(data)
        {
            /*
            {
              "playMode": "Ready",
              "playerPerTeam": 5,
              "isFirstHalf": true,
              "nextKickOffTeamNum": 2,
              "isPenaltyShootOut": false,
              "isOvertime": false,
              "lastDropInTeamNum": 0,
              "secSinceDropIn": 255,
              "secondsRemaining": 0,
              "team1": {
                "num": 1,
                "score": 0,
                "players": [
                  {
                    "penalty": "Ball Manipulation",
                    "penaltySecondsRemaining": 0
                  },
                  {
                    "penalty": "Illegal Attack",
                    "penaltySecondsRemaining": 19
                  },
                  {
                    "penalty": null
                  },
                  {
                    "penalty": null
                  },
                  {
                    "penalty": null
                  }
                ]
              },
              "team2": {
                "num": 24,
                "score": 0,
                "players": [
                  {
                    "penalty": null
                  },
                  {
                    "penalty": null
                  },
                  {
                    "penalty": null
                  },
                  {
                    "penalty": null
                  },
                  {
                    "penalty": "Request For Pickup To Service",
                    "penaltySecondsRemaining": 76
                  }
                ]
              }
            }
            */

            data.timeString = Math.floor(data.secondsRemaining / 60) + ':' + padLeft(data.secondsRemaining % 60, 2, '0');

            var amendTeam = function(team)
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

            amendTeam(data.team1);
            amendTeam(data.team2);

            this.$container.empty();
            this.$container.append(moduleTemplate.create(data));
        };

        return GameStateModule;
    }
);
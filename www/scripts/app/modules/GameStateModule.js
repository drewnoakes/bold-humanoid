define(
    [
        'DataProxy',
        'Protocols'
    ],
    function(DataProxy, Protocols)
    {
        'use strict';

        var moduleTemplate = Handlebars.compile($('#game-module-template').html());

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
                Protocols.gameState,
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

            this.$container.html(moduleTemplate(data));
        };

        return GameStateModule;
    }
);
/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlBuilder'
    ],
    function(ControlBuilder)
    {
        'use strict';

        var VoiceModule = function()
        {
            this.$container = $('<div></div>');

            this.title = 'voice';
            this.id = 'voice';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    supports: { fullScreen: true, advanced: false }
                }
            ];
        };

        VoiceModule.prototype.load = function()
        {
            ControlBuilder.actions('voice.speak', $('<div></div>', {'class': 'control-container ambulator-controls'}).appendTo(this.$container).get(0));
        };

        VoiceModule.prototype.unload = function()
        {
            this.$container.empty();
        };

        return VoiceModule;
    }
);

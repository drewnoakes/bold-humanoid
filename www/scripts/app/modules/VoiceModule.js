/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlBuilder',
        'util/Closeable'
    ],
    function(ControlBuilder, Closeable)
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

            this.closables = new Closeable();
        };

        VoiceModule.prototype.load = function()
        {
            this.closables.add(ControlBuilder.build('options.announce-fsm-states', this.$container.get(0)));

            var controls = $('<div></div>', {'class': 'control-container ambulator-controls'}).appendTo(this.$container).get(0);
            ControlBuilder.actions('voice.speak', controls);
        };

        VoiceModule.prototype.unload = function()
        {
            this.$container.empty();
            this.closables.closeAll();
        };

        return VoiceModule;
    }
);

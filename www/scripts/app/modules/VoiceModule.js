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
            var usage = document.createElement('div');
            usage.className = 'control-container';
            this.closables.add(ControlBuilder.build('options.announce-fsm-states', usage));
            this.$container.append(usage);

            var controls = document.createElement('div');
            controls.className = 'control-container flow';
            ControlBuilder.buildAll('voice', controls);
            this.$container.append(controls);

            var sayings = $('<div></div>', {'class': 'control-container'}).appendTo(this.$container).get(0);
            ControlBuilder.actions('voice.speak', sayings);
        };

        VoiceModule.prototype.unload = function()
        {
            this.$container.empty();
            this.closables.closeAll();
        };

        return VoiceModule;
    }
);

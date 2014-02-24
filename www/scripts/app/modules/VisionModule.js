/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'WebSocketFactory',
        'Protocols',
        'DataProxy',
        'ControlClient',
        'ControlBuilder',
        'DOMTemplate',
        'PixelLabelInspector',
        'Color',
        'util/Closeable',
        'util/Controls'
    ],
    function(WebSocketFactory, Protocols, DataProxy, ControlClient, ControlBuilder, DOMTemplate, PixelLabelInspector, color, Closeable, controls)
    {
        'use strict';

        var moduleTemplate = new DOMTemplate("vision-module-template");

        var VisionModule = function()
        {
            this.$container = $('<div></div>');

            /////

            this.title = 'vision';
            this.id = 'vision';
            this.supports = { advanced: true };
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    supports: { fullScreen: false, advanced: true }
                }
            ];

            this.closables = new Closeable();
        };

        VisionModule.prototype.load = function()
        {
            var element = moduleTemplate.create();
            this.$container.append(element);

            var pixelLabelContainer = element.querySelector('div.pixel-labels');
            this.closables.add(ControlBuilder.buildAll('vision.pixel-labels', pixelLabelContainer));

            var dataStreamerContainer = element.querySelector('div.data-streamer');
            this.closables.add(ControlBuilder.buildAll('data-streamer', dataStreamerContainer));

            var captureContainer = element.querySelector('div.capture');
            ControlBuilder.action('camera.save-yuv-frame', captureContainer);
            ControlBuilder.action('camera.save-debug-frame', captureContainer);
            this.closables.add(ControlBuilder.build('camera.recording-frames', captureContainer));

            var visionOptionsContainer = element.querySelector('div.vision-options');
            this.closables.add(ControlBuilder.build('vision.ignore-above-horizon', visionOptionsContainer));
            this.closables.add(ControlBuilder.build('vision.label-counter.enable', visionOptionsContainer));

            var blobSettingsContainer = element.querySelector('div.blob-detection');
            this.closables.add(ControlBuilder.build('vision.blob-detection.enable', blobSettingsContainer));

            var ballSettingsContainer = element.querySelector('div.ball-detection');
            this.closables.add(ControlBuilder.buildAll('vision.ball-detection', ballSettingsContainer));

            var goalSettingsContainer = element.querySelector('div.goal-detection');
            this.closables.add(ControlBuilder.buildAll('vision.goal-detection', goalSettingsContainer));

            var granularitySettingsContainer = element.querySelector('div.granularity');
            this.closables.add(ControlBuilder.build('vision.image-granularity', granularitySettingsContainer));
            this.closables.add(ControlBuilder.build('vision.max-granularity', granularitySettingsContainer));

            var fieldEdgeContainer = element.querySelector('div.field-edge');
            this.closables.add(ControlBuilder.build('vision.field-edge-pass.field-edge-type', fieldEdgeContainer));
            this.closables.add(ControlBuilder.build('vision.field-edge-pass.min-vertical-run-length', fieldEdgeContainer));
            this.closables.add(ControlBuilder.build('vision.field-edge-pass.complete.smoothing-window-length', fieldEdgeContainer));
            this.closables.add(ControlBuilder.build('vision.field-edge-pass.use-convex-hull', fieldEdgeContainer));

            var lineContainer = element.querySelector('div.line-detection');
            this.closables.add(ControlBuilder.build('vision.line-detection.enable', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.line-dots.hysteresis', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.delta-r', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.delta-theta-degs', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.max-line-gap', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.max-lines-returned', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.min-line-length', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.min-votes', lineContainer));

            this.closables.add(ControlBuilder.buildAll('round-table.image-features', element.querySelector('div.image-features')));

            this.closables.add(ControlBuilder.buildAll('camera.settings', element.querySelector('div.camera-settings')));
            this.closables.add(ControlBuilder.buildAll('camera.calibration', element.querySelector('div.camera-calibration')));

            var imageColoursContainer = element.querySelector('div.image-colours');
            this.closables.add(ControlBuilder.buildAll('round-table.image-colours', imageColoursContainer));
            this.closables.add(ControlBuilder.build('round-table.cartoon.background-colour', imageColoursContainer));

            this.closables.add(ControlBuilder.buildAll('head-module', element.querySelector('div.head-settings')));

            new controls.TabControl(element);
        };

        VisionModule.prototype.unload = function()
        {
            this.$container.empty();

            this.closables.closeAll();
        };

        return VisionModule;
    }
);

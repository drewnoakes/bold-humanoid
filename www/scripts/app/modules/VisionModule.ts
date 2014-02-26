/**
 * @author Drew Noakes http://drewnoakes.com
 */

import ControlBuilder = require('ControlBuilder');
import DOMTemplate = require('DOMTemplate');
import TabControl = require('util/TabControl');
import Module = require('Module');

var moduleTemplate = new DOMTemplate("vision-module-template");

class VisionModule extends Module
{
    constructor()
    {
        super('vision', 'vision');
    }

    public load(element: HTMLDivElement)
    {
        var content = <HTMLDListElement>moduleTemplate.create();
        element.appendChild(content);

        var pixelLabelContainer = content.querySelector('div.pixel-labels');
        ControlBuilder.buildAll('vision.pixel-labels', pixelLabelContainer, this.closeables);

        var dataStreamerContainer = content.querySelector('div.data-streamer');
        ControlBuilder.buildAll('data-streamer', dataStreamerContainer, this.closeables);

        var captureContainer = content.querySelector('div.capture');
        ControlBuilder.action('camera.save-yuv-frame', captureContainer);
        ControlBuilder.action('camera.save-debug-frame', captureContainer);
        ControlBuilder.build('camera.recording-frames', captureContainer, this.closeables);

        var visionOptionsContainer = content.querySelector('div.vision-options');
        ControlBuilder.build('vision.ignore-above-horizon', visionOptionsContainer, this.closeables);
        ControlBuilder.build('vision.label-counter.enable', visionOptionsContainer, this.closeables);

        var blobSettingsContainer = content.querySelector('div.blob-detection');
        ControlBuilder.build('vision.blob-detection.enable', blobSettingsContainer, this.closeables);

        var ballSettingsContainer = content.querySelector('div.ball-detection');
        ControlBuilder.buildAll('vision.ball-detection', ballSettingsContainer, this.closeables);

        var goalSettingsContainer = content.querySelector('div.goal-detection');
        ControlBuilder.buildAll('vision.goal-detection', goalSettingsContainer, this.closeables);

        var granularitySettingsContainer = content.querySelector('div.granularity');
        ControlBuilder.build('vision.image-granularity', granularitySettingsContainer, this.closeables);
        ControlBuilder.build('vision.max-granularity', granularitySettingsContainer, this.closeables);

        var fieldEdgeContainer = content.querySelector('div.field-edge');
        ControlBuilder.build('vision.field-edge-pass.field-edge-type', fieldEdgeContainer, this.closeables);
        ControlBuilder.build('vision.field-edge-pass.min-vertical-run-length', fieldEdgeContainer, this.closeables);
        ControlBuilder.build('vision.field-edge-pass.complete.smoothing-window-length', fieldEdgeContainer, this.closeables);
        ControlBuilder.build('vision.field-edge-pass.use-convex-hull', fieldEdgeContainer, this.closeables);

        var lineContainer = content.querySelector('div.line-detection');
        ControlBuilder.build('vision.line-detection.enable', lineContainer, this.closeables);
        ControlBuilder.build('vision.line-detection.line-dots.hysteresis', lineContainer, this.closeables);
        ControlBuilder.build('vision.line-detection.mask-walk.delta-r', lineContainer, this.closeables);
        ControlBuilder.build('vision.line-detection.mask-walk.delta-theta-degs', lineContainer, this.closeables);
        ControlBuilder.build('vision.line-detection.mask-walk.max-line-gap', lineContainer, this.closeables);
        ControlBuilder.build('vision.line-detection.mask-walk.max-lines-returned', lineContainer, this.closeables);
        ControlBuilder.build('vision.line-detection.mask-walk.min-line-length', lineContainer, this.closeables);
        ControlBuilder.build('vision.line-detection.mask-walk.min-votes', lineContainer, this.closeables);

        ControlBuilder.buildAll('round-table.image-features', content.querySelector('div.image-features'), this.closeables);

        ControlBuilder.buildAll('camera.settings', content.querySelector('div.camera-settings'), this.closeables);
        ControlBuilder.buildAll('camera.calibration', content.querySelector('div.camera-calibration'), this.closeables);

        var imageColoursContainer = content.querySelector('div.image-colours');
        ControlBuilder.buildAll('round-table.image-colours', imageColoursContainer, this.closeables);
        ControlBuilder.build('round-table.cartoon.background-colour', imageColoursContainer, this.closeables);

        ControlBuilder.buildAll('head-module', content.querySelector('div.head-settings'), this.closeables);

        new TabControl(content);
    }
}

export = VisionModule;

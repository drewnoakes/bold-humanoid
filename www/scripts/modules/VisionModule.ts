/**
 * @author Drew Noakes http://drewnoakes.com
 */

import control = require('control');
import DOMTemplate = require('DOMTemplate');
import TabControl = require('controls/TabControl');
import Module = require('Module');

var moduleTemplate = DOMTemplate.forId("vision-module-template");

class VisionModule extends Module
{
    constructor()
    {
        super('vision', 'vision');
    }

    public load(width: number)
    {
        var content = <HTMLDListElement>moduleTemplate.create();
        this.element.appendChild(content);

        var pixelLabelContainer = content.querySelector('div.pixel-labels');
        control.buildSettings('vision.pixel-labels', pixelLabelContainer, this.closeables);

        var dataStreamerContainer = content.querySelector('div.data-streamer');
        control.buildSettings('data-streamer', dataStreamerContainer, this.closeables);

        var captureContainer = content.querySelector('div.capture');
        control.buildAction('camera.save-yuv-frame', captureContainer);
        control.buildAction('camera.save-debug-frame', captureContainer);
        control.buildSetting('camera.recording-frames', captureContainer, this.closeables);

        var visionOptionsContainer = content.querySelector('div.vision-options');
        control.buildSetting('vision.ignore-above-horizon', visionOptionsContainer, this.closeables);
        control.buildSetting('vision.label-counter.enable', visionOptionsContainer, this.closeables);

        var blobSettingsContainer = content.querySelector('div.blob-detection');
        control.buildSetting('vision.blob-detection.enable', blobSettingsContainer, this.closeables);

        var ballSettingsContainer = content.querySelector('div.ball-detection');
        control.buildSettings('vision.ball-detection', ballSettingsContainer, this.closeables);

        var goalSettingsContainer = content.querySelector('div.goal-detection');
        control.buildSettings('vision.goal-detection', goalSettingsContainer, this.closeables);

        var playerSettingsContainer = content.querySelector('div.player-detection');
        control.buildSettings('vision.player-detection', playerSettingsContainer, this.closeables);

        var occlusionSettingsContainer = content.querySelector('div.occlusion');
        control.buildSettings('vision.occlusion', occlusionSettingsContainer, this.closeables);

        var granularitySettingsContainer = content.querySelector('div.granularity');
        control.buildSetting('vision.image-granularity', granularitySettingsContainer, this.closeables);
        control.buildSetting('vision.max-granularity', granularitySettingsContainer, this.closeables);

        var fieldEdgeContainer = content.querySelector('div.field-edge');
        control.buildSetting('vision.field-edge-pass.field-edge-type', fieldEdgeContainer, this.closeables);
        control.buildSetting('vision.field-edge-pass.min-vertical-run-length', fieldEdgeContainer, this.closeables);
        control.buildSetting('vision.field-edge-pass.complete.smoothing-window-length', fieldEdgeContainer, this.closeables);
        control.buildSetting('vision.field-edge-pass.use-convex-hull', fieldEdgeContainer, this.closeables);

        var lineContainer = content.querySelector('div.line-detection');
        control.buildSetting('vision.line-detection.enable', lineContainer, this.closeables);
        control.buildSettings('vision.line-detection.scanning', lineContainer, this.closeables);

        /*
        control.buildSetting('vision.line-detection.line-dots.hysteresis', lineContainer, this.closeables);
        */

        control.buildSettings('round-table.image-features', content.querySelector('div.image-features'), this.closeables);

        var cameraSettings = content.querySelector('div.camera-settings');
        control.buildAction('camera.refresh-all-control-values', cameraSettings);
        control.buildSettings('camera.settings', cameraSettings, this.closeables);

        control.buildSettings('camera.calibration', content.querySelector('div.camera-calibration'), this.closeables);

        var imageColoursContainer = content.querySelector('div.image-colours');
        control.buildSettings('round-table.image-colours', imageColoursContainer, this.closeables);
        control.buildSetting('round-table.cartoon.background-colour', imageColoursContainer, this.closeables);

        control.buildSettings('head-module', content.querySelector('div.head-settings'), this.closeables);

        new TabControl(content);
    }
}

export = VisionModule;

/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'WebSocketFactory',
        'Protocols',
        'DataProxy',
        'ControlBuilder',
        'DOMTemplate',
        'PixelLabelInspector',
        'util/Colour',
        'util/Closeable',
        'util/MouseEventUtil'
    ],
    function(WebSocketFactory, Protocols, DataProxy, ControlBuilder, DOMTemplate, PixelLabelInspector, Colour, Closeable, MouseEventUtil)
    {
        'use strict';

        var moduleTemplate = new DOMTemplate("camera-module-template");

        var CameraModule = function()
        {
            this.$container = $('<div></div>');

            /////

            this.title = 'camera';
            this.id = 'camera';
            this.supports = { advanced: true };
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
//                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true, advanced: true }
                }
            ];

            this.closables = new Closeable();
        };

        CameraModule.prototype.load = function()
        {
            var element = moduleTemplate.create();
            this.$container.append(element);

            this.cameraCanvas = element.querySelector('.camera-canvas');
            this.$cameraCanvas = $(this.cameraCanvas);

            this.hoverPixelInfo = element.querySelector('.hover-pixel-info');

            var pixelLabelInspectorCanvas = element.querySelector('.pixel-label-inspector');
            this.pixelLabelInspector = new PixelLabelInspector(pixelLabelInspectorCanvas, 320, 85);
            this.pixelLabelInspector.setVisible(false);

            this.bindInteraction();

            this.createContext();

            this.closables.add(DataProxy.subscribe(
                Protocols.camera,
                {
                    json: false,
                    onmessage: _.bind(this.onmessage, this)
                }
            ));

            ControlBuilder.action('head-module.move-down', element.querySelector('button[name="down"]'));
            ControlBuilder.action('head-module.move-up', element.querySelector('button[name="up"]'));
            ControlBuilder.action('head-module.move-left', element.querySelector('button[name="left"]'));
            ControlBuilder.action('head-module.move-right', element.querySelector('button[name="right"]'));
            ControlBuilder.action('head-module.move-home', element.querySelector('button[name="home"]'));
            ControlBuilder.action('head-module.move-zero', element.querySelector('button[name="zero"]'));
            this.closables.add(ControlBuilder.build('head-module.move-fine', element.querySelector('div.movement')));

            this.closables.add(ControlBuilder.buildAll('vision.pixel-labels', element.querySelector('div.pixel-labels')));

            var captureContainer = element.querySelector('.capture');
            ControlBuilder.action('camera.save-frame', captureContainer);
            this.closables.add(ControlBuilder.build('camera.recording-frames', captureContainer));

            var visionOptionsContainer = element.querySelector('div.vision-options');
            this.closables.add(ControlBuilder.build('vision.ignore-above-horizon', visionOptionsContainer));
            this.closables.add(ControlBuilder.build('vision.label-counter.enable', visionOptionsContainer));

            var visionSettingsContainer = element.querySelector('div.object-detection');
            this.closables.add(ControlBuilder.build('vision.min-ball-area', visionSettingsContainer));
            this.closables.add(ControlBuilder.build('vision.min-goal-dimension-pixels', visionSettingsContainer));

            var granularitySettingsContainer = element.querySelector('div.granularity');
            this.closables.add(ControlBuilder.build('vision.image-granularity', granularitySettingsContainer));
            this.closables.add(ControlBuilder.build('vision.max-granularity', granularitySettingsContainer));

            var fieldEdgeContainer = element.querySelector('div.field-edge');
            this.closables.add(ControlBuilder.build('vision.field-edge-pass.field-edge-type', fieldEdgeContainer));
            this.closables.add(ControlBuilder.build('vision.field-edge-pass.min-vertical-run-length', fieldEdgeContainer));
            this.closables.add(ControlBuilder.build('vision.field-edge-pass.complete.smoothing-window-length', fieldEdgeContainer));

            var lineContainer = element.querySelector('div.line-detection');
            this.closables.add(ControlBuilder.build('vision.line-detection.enable', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.line-dots.hysteresis', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.delta-r', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.delta-theta-degs', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.max-line-gap', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.max-lines-returned', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.min-line-length', lineContainer));
            this.closables.add(ControlBuilder.build('vision.line-detection.mask-walk.min-votes', lineContainer));

            var imageSettingsContainer = element.querySelector('div.image-settings');
            this.closables.add(ControlBuilder.build('round-table.image-type', imageSettingsContainer));
            this.closables.add(ControlBuilder.build('round-table.camera-frame-frequency', imageSettingsContainer));

            this.closables.add(ControlBuilder.buildAll('round-table.image-features', element.querySelector('div.image-features')));

            this.closables.add(ControlBuilder.buildAll('camera.settings', element.querySelector('div.camera-settings')));

            this.closables.add(ControlBuilder.buildAll('round-table.image-colours', element.querySelector('div.image-colours')));

//            ControlBuilder.build('camera',                $('<div></div>', {'class': 'control-container camera-controls'}).appendTo(this.$container));
//            ControlBuilder.build('debug-image',           $('<div></div>', {'class': 'control-container image-controls'}).appendTo(this.$container));
//            ControlBuilder.build('image-capture',         $('<div></div>', {'class': 'control-container image-capture-controls'}).appendTo(this.$container));
//            ControlBuilder.build('debug-image-features',  $('<div></div>', {'class': 'control-container image-controls-features'}).appendTo(this.$container));
//            ControlBuilder.build('vision/line-detection', $('<div></div>', {'class': 'control-container lines-controls'}).appendTo(this.$container));
//            ControlBuilder.build('vision/label-count',    $('<div></div>', {'class': 'control-container label-count-controls'}).appendTo(this.$container));
//            ControlBuilder.build('vision/objects',        $('<div></div>', {'class': 'control-container object-controls'}).appendTo(this.$container));
//            ControlBuilder.build('vision/field-edge',     $('<div></div>', {'class': 'control-container field-edge-controls'}).appendTo(this.$container));
//            ControlBuilder.build('vision/horizon',        $('<div></div>', {'class': 'control-container horizon-controls'}).appendTo(this.$container));
//            ControlBuilder.build('vision/lut',            $('<div></div>', {'class': 'control-container lut-controls'}).appendTo(this.$container));
        };

        CameraModule.prototype.unload = function()
        {
            this.$container.empty();

            this.closables.closeAll();
        };

        CameraModule.prototype.bindInteraction = function()
        {
            var isImageLarge = false;
            this.$cameraCanvas.click(function (event)
            {
                var $controlDivs = $('.control-container', this.$container);
                if (isImageLarge) {
                    isImageLarge = false;
                    this.$cameraCanvas.css({width: this.cameraCanvas.width});
                    // TODO BUG this is showing advanced control divs when in basic mode
                    $controlDivs.delay(400).fadeIn();
                    this.pixelLabelInspector.setVisible(true);
                } else {
                    if (event.shiftKey) {
                        var rgb = this.context.getImageData(event.offsetX, event.offsetY, 1, 1).data,
                            hsv = Colour.rgbToHsv({r:rgb[0]/255, g:rgb[1]/255, b:rgb[2]/255});
                        console.log(Math.round(hsv.h * 255) + ',' + Math.round(hsv.s * 255) + ',' + Math.round(hsv.v * 255));
                    } else {
                        isImageLarge = true;
                        this.pixelLabelInspector.setVisible(false);
                        this.hoverPixelInfo.textContent = '';
                        this.$cameraCanvas.css({width: '100%'});
                        $controlDivs.hide();
                    }
                }
            }.bind(this));
            this.$cameraCanvas.mouseleave(function ()
            {
                this.hoverPixelInfo.textContent = '';
                this.pixelLabelInspector.setVisible(false);
            }.bind(this));
            this.$cameraCanvas.mousemove(function (e)
            {
                if (!this.context || isImageLarge)
                    return;
                MouseEventUtil.polyfill(e);
                var x = e.offsetX,
                    y = e.offsetY,
                    rgb = this.context.getImageData(x, y, 1, 1).data,
                    hsv = Colour.rgbToHsv({r:rgb[0]/255, g:rgb[1]/255, b:rgb[2]/255});
                this.hoverPixelInfo.textContent =
                    'Pos: ' + (this.cameraCanvas.width - x) + ',' + (this.cameraCanvas.height - y) +
                    ' RGB: ' + rgb[0] + ',' + rgb[1] + ',' + rgb[2] +
                    ' HSV: ' + Math.round(hsv.h * 255) + ',' + Math.round(hsv.s * 255) + ',' + Math.round(hsv.v * 255);
                this.pixelLabelInspector.setVisible(true);
                this.pixelLabelInspector.highlightHsv(hsv);
            }.bind(this));
        };

        CameraModule.prototype.createContext = function ()
        {
            this.context = this.cameraCanvas.getContext('2d');

            // The image arrives flipped in both x and y axes.
            // Rotate image to correct orientation via scaling by -1 in x and y.
            this.context.translate(this.cameraCanvas.width, this.cameraCanvas.height);
            this.context.scale(-1, -1);
        };

        CameraModule.prototype.onmessage = function(message)
        {
            if (!(message.data instanceof Blob))
            {
                console.warn('[CameraModule.js] Expected blob, got: ', message.data);
                return;
            }

            // append to the blob (ignore weird syntax)
            this.imgBlob = new Blob([message.data], {type: 'image/jpeg'});

            var objectURL = (window.webkitURL || window.URL).createObjectURL(this.imgBlob);
            var img = new Image();
            img.onload = function()
            {
                var changedSize = false;
                if (img.width !== this.cameraCanvas.width) {
                    this.cameraCanvas.width = img.width;
                    changedSize = true;
                }
                if (img.height !== this.cameraCanvas.height) {
                    this.cameraCanvas.height = img.height;
                    changedSize = true;
                }
                if (changedSize) {
                    // need to re-establish the context after changing the canvas size
                    this.createContext();
                }
                this.context.drawImage(img, 0, 0);
            }.bind(this);
            img.src = objectURL;
        };

        return CameraModule;
    }
);

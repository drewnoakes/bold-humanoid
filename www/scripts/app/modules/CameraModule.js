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
        'util/MouseEventUtil'
    ],
    function(WebSocketFactory, Protocols, DataProxy, ControlClient, ControlBuilder, DOMTemplate, PixelLabelInspector, color, Closeable, MouseEventUtil)
    {
        'use strict';

        var moduleTemplate = new DOMTemplate("camera-module-template");

        var CameraModule = function()
        {
            this.$container = $('<div></div>');

            /////

            this.title = 'camera';
            this.id = 'camera';
            this.supports = { advanced: false };
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    supports: { fullScreen: true, advanced: false }
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

            var imageSettingsContainer = element.querySelector('div.camera-module-controls');
            this.closables.add(ControlBuilder.build('round-table.image-type', imageSettingsContainer));
            this.closables.add(ControlBuilder.build('round-table.camera-frame-frequency', imageSettingsContainer));

            ControlBuilder.action('head-module.move-down', imageSettingsContainer);
            ControlBuilder.action('head-module.move-up', imageSettingsContainer);
            ControlBuilder.action('head-module.move-left', imageSettingsContainer);
            ControlBuilder.action('head-module.move-right', imageSettingsContainer);
            ControlBuilder.action('head-module.move-home', imageSettingsContainer);
            ControlBuilder.action('head-module.move-zero', imageSettingsContainer);
            this.closables.add(ControlBuilder.build('head-module.move-fine', imageSettingsContainer));

//            ControlBuilder.action('head-module.move-down', element.querySelector('button[name="down"]'));
//            ControlBuilder.action('head-module.move-up', element.querySelector('button[name="up"]'));
//            ControlBuilder.action('head-module.move-left', element.querySelector('button[name="left"]'));
//            ControlBuilder.action('head-module.move-right', element.querySelector('button[name="right"]'));
//            ControlBuilder.action('head-module.move-home', element.querySelector('button[name="home"]'));
//            ControlBuilder.action('head-module.move-zero', element.querySelector('button[name="zero"]'));
//            this.closables.add(ControlBuilder.build('head-module.move-fine', imageSettingsContainer));
        };

        CameraModule.prototype.unload = function()
        {
            this.$container.empty();

            this.closables.closeAll();
        };

        CameraModule.prototype.bindInteraction = function()
        {
            var imageTypeSetting = ControlClient.getSetting('round-table.image-type');

            // If the image is clicked when shift is held, log the HSV value in the image to the console
            this.$cameraCanvas.click(function (event)
            {
                if (event.shiftKey) {
                    var rgb = this.context.getImageData(event.offsetX, event.offsetY, 1, 1).data,
                        hsv = new color.Rgb(rgb[0]/255, rgb[1]/255, rgb[2]/255).toHsv();
                    console.log(Math.round(hsv.h * 255) + ',' + Math.round(hsv.s * 255) + ',' + Math.round(hsv.v * 255));
                }
            }.bind(this));

            this.$cameraCanvas.mouseleave(function ()
            {
                this.hoverPixelInfo.textContent = '';
                this.pixelLabelInspector.setVisible(false);
            }.bind(this));

            this.$cameraCanvas.mousemove(function (e)
            {
                if (!this.context)
                    return;
                MouseEventUtil.polyfill(e);
                var x = e.offsetX,
                    y = e.offsetY,
                    hoverText = 'Pos: ' + (this.cameraCanvas.width - x) + ',' + (this.cameraCanvas.height - y);
                if (imageTypeSetting.value === 2) {
                    var rgb = this.context.getImageData(x, y, 1, 1).data,
                        hsv = new color.Rgb(rgb[0]/255, rgb[1]/255, rgb[2]/255).toHsv();
                    this.pixelLabelInspector.setVisible(true);
                    this.pixelLabelInspector.highlightHsv(hsv);
                    hoverText += ' RGB: ' + rgb[0] + ',' + rgb[1] + ',' + rgb[2] +
                                 ' HSV: ' + Math.round(hsv.H * 255) + ',' + Math.round(hsv.S * 255) + ',' + Math.round(hsv.V * 255);
                }
                this.hoverPixelInfo.textContent = hoverText;
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
            console.assert(message.data instanceof Blob);

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

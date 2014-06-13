/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import Closeable = require('util/Closeable');
import color = require('color');
import constants = require('constants');
import control = require('control');
import data = require('data');
import DOMTemplate = require('DOMTemplate');
import HeadControls = require('controls/HeadControls');
import mouse = require('util/mouse');
import Module = require('Module');
import PixelLabelInspector = require('controls/PixelLabelInspector');

var moduleTemplate = DOMTemplate.forId("camera-module-template");

class CameraModule extends Module
{
    private cameraCanvas: HTMLCanvasElement;
    private context: CanvasRenderingContext2D;
    private hoverPixelInfo: HTMLDivElement;
    private pixelLabelInspector: PixelLabelInspector;

    constructor()
    {
        super('camera', 'camera', {fullScreen: true});
    }

    public load()
    {
        var content = <HTMLElement>moduleTemplate.create();
        this.element.appendChild(content);

        this.cameraCanvas = <HTMLCanvasElement>content.querySelector('.camera-canvas');
        this.hoverPixelInfo = <HTMLDivElement>content.querySelector('.hover-pixel-info');

        var pixelLabelInspectorCanvas = <HTMLCanvasElement>content.querySelector('.pixel-label-inspector');
        this.pixelLabelInspector = new PixelLabelInspector(pixelLabelInspectorCanvas, 320, 120);
        this.pixelLabelInspector.setVisible(false);

        this.bindInteraction();

        this.createContext();

        this.closeables.add(new data.Subscription<any>(
            constants.protocols.camera,
            {
                parseJson: false,
                onmessage: this.onCameraData.bind(this)
            }
        ));

        var imageSettingsContainer = <HTMLElement>content.querySelector('div.camera-module-controls');
        control.buildSetting('round-table.image-type', imageSettingsContainer, this.closeables);
        control.buildSetting('round-table.camera-frame-frequency', imageSettingsContainer, this.closeables);

        imageSettingsContainer.appendChild(new HeadControls().element);

        control.buildSetting('head-module.move-fine', imageSettingsContainer, this.closeables);
        control.buildActions('head-module.look-at', imageSettingsContainer);
    }

    public unload()
    {
        delete this.cameraCanvas;
        delete this.hoverPixelInfo;
        delete this.pixelLabelInspector;
        delete this.context;
    }

    public onResized(width: number, height: number, isFullScreen: boolean)
    {
        if (isFullScreen)
        {
            this.cameraCanvas.style.width = "100%";
        }
        else
        {
            this.cameraCanvas.style.width = constants.cameraImageWidth + 'px';
        }
    }

    private scaleImagePoint(offsetX: number, offsetY: number)
    {
        var xScale = constants.cameraImageWidth / this.cameraCanvas.clientWidth,
            yScale = constants.cameraImageHeight / this.cameraCanvas.clientHeight;

        return {
            x: Math.round(offsetX * xScale),
            y: Math.round(offsetY * yScale)
        };
    }

    private bindInteraction()
    {
        var imageTypeSetting = control.getSetting('round-table.image-type');

        // If the image is clicked when shift is held, log the HSV value in the image to the console
        this.cameraCanvas.addEventListener('click', event =>
        {
            if (event.shiftKey) {
                var point = this.scaleImagePoint(event.offsetX, event.offsetY),
                    rgb = this.context.getImageData(point.x, point.y, 1, 1).data,
                    hsv = new color.Rgb(rgb[0]/255, rgb[1]/255, rgb[2]/255).toHsv();
                console.log(Math.round(hsv.H * 255) + ',' + Math.round(hsv.S * 255) + ',' + Math.round(hsv.V * 255));
            }
        });

        this.cameraCanvas.addEventListener('mouseleave', () =>
        {
            this.hoverPixelInfo.textContent = '';
            this.pixelLabelInspector.setVisible(false);
        });

        this.cameraCanvas.addEventListener('mousemove', e =>
        {
            if (!this.context)
                return;
            mouse.polyfill(e);
            var point = this.scaleImagePoint(event.offsetX, event.offsetY),
                hoverX = this.cameraCanvas.clientWidth - 1 - point.x,
                hoverY = this.cameraCanvas.clientHeight - 1 - point.y,
                hoverText = 'Pos: ' + hoverX + ',' + hoverY;
            if (imageTypeSetting.value === 2) {
                var rgb = this.context.getImageData(point.x, point.y, 1, 1).data,
                    hsv = new color.Rgb(rgb[0]/255, rgb[1]/255, rgb[2]/255).toHsv();
                this.pixelLabelInspector.setVisible(true);
                this.pixelLabelInspector.highlightHsv(hsv);
                hoverText += ' RGB: ' + rgb[0] + ',' + rgb[1] + ',' + rgb[2] +
                             ' HSV: ' + Math.round(hsv.H * 255) + ',' + Math.round(hsv.S * 255) + ',' + Math.round(hsv.V * 255);
            }
            this.hoverPixelInfo.textContent = hoverText;
        });
    }

    private createContext()
    {
        this.context = this.cameraCanvas.getContext('2d');

        // The image arrives flipped in both x and y axes.
        // Rotate image to correct orientation via scaling by -1 in x and y.
        this.context.translate(this.cameraCanvas.width, this.cameraCanvas.height);
        this.context.scale(-1, -1);
    }

    private onCameraData(message: any)
    {
        console.assert(message.data instanceof Blob);

        // For some good information on Blob:
        // https://www.inkling.com/read/javascript-definitive-guide-david-flanagan-6th/chapter-22/blobs

        // Wrap the untyped Blob data in order to specify the content type
        var imgBlob = new Blob([message.data], {type: 'image/jpeg'}),
            url = (<any>window).webkitURL || (<any>window).URL,
            objectURL = url.createObjectURL(imgBlob),
            img = new Image();

        img.onload = () =>
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

            // Release the Blob's object URL
            url.revokeObjectURL(objectURL);
        };

        // Trigger the image to load from the object URL
        img.src = objectURL;
    }
}

export = CameraModule;

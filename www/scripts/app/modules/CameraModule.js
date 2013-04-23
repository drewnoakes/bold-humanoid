define(
    [
        'scripts/app/WebSocketFactory',
        'scripts/app/Protocols',
        'scripts/app/DataProxy',
        'scripts/app/ControlBuilder',
        'scripts/app/util/Colour'
    ],
    function(WebSocketFactory, Protocols, DataProxy, ControlBuilder, Colour)
    {
        'use strict';

        var StateEnum = {
            GET_PREFIX : 0,
            GET_IMAGE : 1
        };

        var CameraModule = function()
        {
            this.$canvas = $('<canvas></canvas>', {'class':'camera-canvas'});
            this.canvas = this.$canvas.get(0);

            this.$container = $('<div></div>');
            this.$hoverPixelInfo = $('<div></div>', {'class': 'hover-pixel-info'});

            this.$container.append(this.$canvas)
                           .append(this.$hoverPixelInfo);

            this.bindInteraction();

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
        };

        CameraModule.prototype.bindInteraction = function ()
        {
            var isImageLarge = false;
            this.$canvas.click(function (event)
            {

                var $controlDivs = $('.module.camera .control-container');
                if (isImageLarge) {
                    isImageLarge = false;
                    this.$canvas.css({width: this.canvas.width});
                    $controlDivs.delay(400).fadeIn();
                } else {
                    if (event.shiftKey) {
                        var rgb = this.context.getImageData(event.offsetX, event.offsetY, 1, 1).data,
                            hsv = Colour.rgbToHsv({r:rgb[0]/255, g:rgb[1]/255, b:rgb[2]/255});
                        console.log(Math.round(hsv.h * 255) + ',' + Math.round(hsv.s * 255) + ',' + Math.round(hsv.v * 255));
                    } else {
                        isImageLarge = true;
                        this.$hoverPixelInfo.text('');
                        this.$canvas.css({width: '100%'});
                        $controlDivs.hide();
                    }
                }
            }.bind(this));
            this.$canvas.mouseleave(function ()
            {
                this.$hoverPixelInfo.text('')
            }.bind(this));
            this.$canvas.mousemove(function (e)
            {
                if (!this.context || isImageLarge)
                    return;
                var x = e.offsetX,
                    y = e.offsetY,
                    rgb = this.context.getImageData(x, y, 1, 1).data,
                    hsv = Colour.rgbToHsv({r:rgb[0]/255, g:rgb[1]/255, b:rgb[2]/255});
                this.$hoverPixelInfo.text(
                    'Pos: ' + (this.canvas.width - x) + ',' + (this.canvas.height - y) +
                    ' RGB: ' + rgb[0] + ',' + rgb[1] + ',' + rgb[2] +
                    ' HSV: ' + Math.round(hsv.h * 255) + ',' + Math.round(hsv.s * 255) + ',' + Math.round(hsv.v * 255)
                );
            }.bind(this));
        };

        CameraModule.prototype.load = function()
        {
            this.imgState = StateEnum.GET_PREFIX;
            this.createContext();

            this.subscription = DataProxy.subscribe(
                Protocols.camera,
                {
                    json: false,
                    onmessage: _.bind(this.onmessage, this)
                }
            );

            ControlBuilder.build('lut',    $('<div></div>', {'class': 'control-container lut-controls'}).appendTo(this.$container));
            ControlBuilder.build('lines',  $('<div></div>', {'class': 'control-container lines-controls'}).appendTo(this.$container));
            ControlBuilder.build('camera', $('<div></div>', {'class': 'control-container camera-controls'}).appendTo(this.$container));
            ControlBuilder.build('debug',  $('<div></div>', {'class': 'control-container debug-controls'}).appendTo(this.$container));
        };

        CameraModule.prototype.unload = function()
        {
            this.subscription.close();
        };

        CameraModule.prototype.createContext = function ()
        {
            this.context = this.canvas.getContext('2d');

            // rotate image to correct orientation
            this.context.translate(this.canvas.width, this.canvas.height);
            this.context.scale(-1, -1);
        };

        CameraModule.prototype.onmessage = function(message)
        {
            var self = this;
            switch (this.imgState)
            {
                case StateEnum.GET_PREFIX:
                {
                    if (typeof(message.data) === 'string')
                    {
                        this.imgToRead = parseInt(message.data);
                        // clean out the blob
                        this.imgBlob = new Blob([], {type: 'image/jpeg'});
                        this.imgState = StateEnum.GET_IMAGE;
                    }
                    break;
                }
                case StateEnum.GET_IMAGE:
                {
                    if (!(message.data instanceof Blob))
                    {
                        console.warn('[CameraModule.js] Expected blob, got: ', message.data);
                        this.imgState = StateEnum.GET_PREFIX;
                        break;
                    }

                    // append to the blob (ignore weird syntax)
                    this.imgBlob = new Blob([this.imgBlob, message.data], {type: 'image/jpeg'});

                    this.imgToRead -= message.data.size;

                    if (this.imgToRead <= 0)
                    {
                        var objectURL = (window.webkitURL || window.URL).createObjectURL(this.imgBlob);
                        var img = new Image();
                        img.onload = function()
                        {
                            var changedSize = false;
                            if (img.width !== self.canvas.width) {
                                self.canvas.width = img.width;
                                changedSize = true;
                            }
                            if (img.height !== self.canvas.height) {
                                self.canvas.height = img.height;
                                changedSize = true;
                            }
                            if (changedSize) {
                                // need to re-establish the context after changing the canvas size
                                self.createContext();
                            }
                            self.context.drawImage(img, 0, 0);
                        };
                        img.src = objectURL;
                        this.imgState = StateEnum.GET_PREFIX;
                    }
                    break;
                }
            }
        };

        return CameraModule;
    }
);

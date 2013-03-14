define(
    [
        'scripts/app/WebSocketFactory',
        'scripts/app/Protocols',
        'scripts/app/DataProxy',
        'scripts/app/ControlBuilder'
    ],
    function(WebSocketFactory, Protocols, DataProxy, ControlBuilder)
    {
        'use strict';

        var StateEnum = {
            GET_CONTROLS : 0,
            GET_PREFIX : 1,
            GET_IMAGE : 2
        };

        var CameraModule = function()
        {
            this.$canvas = $('<canvas></canvas>', {'class':'camera-canvas'});
            this.canvas = this.$canvas.get(0);

            var container = $('<div></div>');
            this.$cameraControlContainer = $('<div></div>', {'class': 'control-container camera-controls'});
            this.$debugControlContainer  = $('<div></div>', {'class': 'control-container debug-controls'});

            container.append(this.$canvas)
                     .append(this.$cameraControlContainer)
                     .append(this.$debugControlContainer);

            this.bindInteraction();

            /////

            this.title = 'camera';
            this.moduleClass = 'camera';
            this.panes = [
                {
                    title: 'main',
                    element: container.get(0),
//                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        CameraModule.prototype.bindInteraction = function ()
        {
            var isImageLarge = false,
                self = this;
            this.$canvas.click(function ()
            {
                if (isImageLarge) {
                    isImageLarge = false;
                    self.$canvas.css({width: ''});
                    self.$cameraControlContainer.fadeIn();
                    self.$debugControlContainer.fadeIn();
                }
                else {
                    isImageLarge = true;
                    self.$canvas.css({width: '100%'});
                    self.$cameraControlContainer.hide();
                    self.$debugControlContainer.hide();
                }
            });
        };

        CameraModule.prototype.load = function()
        {
            this.imgState = StateEnum.GET_CONTROLS;
            this.createContext();

            this.subscription = DataProxy.subscribe(Protocols.camera, { onmessage: _.bind(this.onmessage, this) });
        };

        CameraModule.prototype.unload = function()
        {
            this.subscription.cancel();
        };

        CameraModule.prototype.createContext = function ()
        {
            this.context = this.canvas.getContext('2d');

            // rotate image to correct orientation
            this.context.translate(this.canvas.width, this.canvas.height);
            this.context.scale(-1, -1);
        };

        CameraModule.prototype.sendCommand = function(family, id, value)
        {
            var command = {
                family: family,
                id: id
            };

            if (typeof(value) !== 'undefined')
            {
                command.value = value;
            }

            console.log('Sending command', command);

            this.socket.send(JSON.stringify(command));
        };

        CameraModule.prototype.onmessage = function(message)
        {
            var self = this;
            switch (this.imgState)
            {
                case StateEnum.GET_CONTROLS:
                {
                    var controls = JSON.parse(message.data);

                    console.info('Received control data', controls);

                    ControlBuilder.build('camera', controls.camera, this.$cameraControlContainer, _.bind(this.sendCommand, this));
                    ControlBuilder.build('debug',  controls.debug,  this.$debugControlContainer,  _.bind(this.sendCommand, this));

                    this.imgState = StateEnum.GET_PREFIX;
                    break;
                }
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

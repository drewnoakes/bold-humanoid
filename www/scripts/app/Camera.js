define(
    [
        'scripts/app/WebSocketFactory',
        'scripts/app/Protocols',
        'scripts/app/ControlBuilder'
    ],
    function(WebSocketFactory, Protocols, ControlBuilder)
    {
        'use strict';

        var protocol = Protocols.camera,
            socket,
            imgState = 0,
            imgSize = 0,
            imgToRead = 0,
            imgBlob;

        var StateEnum = {
            GET_CONTROLS : 0,
            GET_PREFIX : 1,
            GET_IMAGE : 2
        };

        var canvas = document.getElementById('camera-canvas');

        var createContext = function ()
        {
            var context = canvas.getContext('2d');

            // rotate image to correct orientation[
            context.translate(canvas.width, canvas.height);
            context.scale(-1, -1);

            return context;
        };

        var context = createContext();

        var sendCommand = function(family, id, value)
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

            socket.send(JSON.stringify(command));
        };

        var onmessage = function(message)
        {
            switch (imgState)
            {
                case StateEnum.GET_CONTROLS:
                {
                    var controls = JSON.parse(message.data);

                    console.info('Received control data', controls);

                    ControlBuilder.build('camera', controls.camera, $('#camera-controls'), sendCommand);
                    ControlBuilder.build('debug',  controls.debug,  $('#head-controls'), sendCommand);
//                    ControlBuilder.build('vision', controls.vision, $('#vision-controls'), sendCommand);

                    imgState = StateEnum.GET_PREFIX;
                    break;
                }
                case StateEnum.GET_PREFIX:
                {
                    if (typeof(message.data) === 'string')
                    {
                        imgSize = imgToRead = parseInt(message.data);
                        imgBlob = new Blob([], {type: 'image/jpeg'});
                        imgState = StateEnum.GET_IMAGE;
                    }
                    break;
                }
                case StateEnum.GET_IMAGE:
                {
                    if (!(message.data instanceof Blob))
                    {
                        console.warn('[Camera.js] Expected blob, got: ', message.data);
                        imgState = StateEnum.GET_PREFIX;
                        break;
                    }

                    imgBlob = new Blob([imgBlob, message.data], {type: 'image/jpeg'});
                    imgToRead -= message.data.size;

                    if (imgToRead <= 0)
                    {
                        var objectURL = (window.webkitURL || window.URL).createObjectURL(imgBlob);
                        var img = new Image();
                        img.onload = function()
                        {
                            // TODO ensure this is still the latest image (might be out of order)
                            var changedSize = false;
                            if (img.width !== canvas.width) {
                                canvas.width = img.width;
                                changedSize = true;
                            }
                            if (img.height !== canvas.height) {
                                canvas.height = img.height;
                                changedSize = true;
                            }
                            if (changedSize) {
                                // need to re-establish the context after changing the canvas size
                                context = createContext();
                            }
                            context.drawImage(img, 0, 0);
                        };
                        img.src = objectURL;
                        imgState = StateEnum.GET_PREFIX;
                    }
                    break;
                }
            }
        };

        var errorHandle = function(e)
        {
            console.error('Camera socket error; closing and reconnecting', e);

            imgState = StateEnum.GET_CONTROLS;

            createSocket();
        };

        var createSocket = function()
        {
            if (socket && socket.readyState == WebSocket.OPEN)
            {
                socket.close();
            }

            socket = WebSocketFactory.open(protocol);
            socket.onerror = errorHandle;
            socket.onmessage = onmessage;
        };

        createSocket();
    }
);

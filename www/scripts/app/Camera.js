define(
    [
        'scripts/app/WebSocketFactory',
        'scripts/app/Protocols'
    ],
    function(WebSocketFactory, Protocols)
    {
        var protocol = Protocols.camera,
            socket,
            imgState = 0,
            imgSize = 0,
            imgToRead = 0,
            imgBlob,
            controls;

        var ControlTypeEnum = {
            INT: 1,
            BOOL: 2,
            MENU: 3,
            BUTTON: 4,
            INT64: 5,
            CTRL_CLASS: 6,
            STRING: 7,
            BITMASK: 8
        };

        var StateEnum = {
            GET_CONTROLS : 0,
            GET_IMG_TAGS : 1,
            GET_PREFIX : 2,
            GET_IMAGE : 3
        };

        var controlHandle = function(e)
        {
            var c = e.data,
                cmd = {
                    command: "setControl",
                    id: c.id,
                    val: parseInt(
                        c.type == ControlTypeEnum.INT ? e.target.value :
                        c.type == ControlTypeEnum.BOOL ? (e.target.checked ? 1 : 0) :
                        c.type == ControlTypeEnum.MENU ? e.target.value :
                            0)
                };
            socket.send(JSON.stringify(cmd));
        };

        $('#camera-frame-period').change(function()
        {
            var cmd = {
                command: "framePeriod",
                period: parseInt(this.selectedOptions[0].value)
            };
            socket.send(JSON.stringify(cmd));
        });

        var labelHandle = function(e)
        {
            var cmd = {
                command: "selectStream",
                id: parseInt(e.target.value)
            };
            socket.send(JSON.stringify(cmd));
        };

        var genLabels = function(labels)
        {
            var menu = $('<select>').change(labelHandle);
            for (var i = 0; i < labels.length; ++i)
            {
                var item = labels[i];
                var option = $('<option>').val(item.id).text(item.label);
                if (i == 0)
                    option.attr('selected', 'selected');
                menu.append(option);
            }
            $('#camera-labels-container').append(menu);
        };

        var genControls = function(controls)
        {
            for (var i = 0; i < controls.length; ++i)
            {
                var c = controls[i],
                    element = $('<div></div>').addClass('camera-control');

                switch (c.type)
                {
                case ControlTypeEnum.INT:
                    element.append($('<h3></h3>').html(c.name + ' <span class="values">(' + c.minimum + ' - ' + c.maximum + ' [' + c.defaultValue + '])</span>'));
                    element.append($('<input>', {type: 'text'}).val(c.value).change(c,controlHandle));
                    break;

                case ControlTypeEnum.BOOL:
                    var id = 'cameraControl' + i;
                    element.append($('<input>', {id: id, type: 'checkbox', checked: c.value}).change(c,controlHandle));
                    element.append($('<label>', {for: id}).html(c.name + ' <span class="values">[' + (c.defaultValue?'on':'off') + ']</span>'));
                    break;

                case ControlTypeEnum.MENU:
                    var defaultItem = c.menuItems[parseInt(c.defaultValue)];
                    element.append($('<h3></h3>').html(c.name + (defaultItem ? ' <span class="values">[' + defaultItem.name + ']</span>':'')));
                    var menu = $('<select></select>').change(c,controlHandle);
                    for (var j = 0; j < c.menuItems.length; ++j)
                    {
                        var item = c.menuItems[j];
                        menu.append($('<option></option>', {selected: c.value == item.index}).val(item.index).text(item.name));
                    }
                    element.append(menu);
                    break;
                }

                $('#camera-controls-container').append(element);
            }
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

        var msgHandle = function(msg)
        {
            switch (imgState)
            {
            // get control data
            case StateEnum.GET_CONTROLS:
                controls = JSON.parse(msg.data);
                console.info(controls);
                genControls(controls);
                imgState = StateEnum.GET_IMG_TAGS;
                break;

            // get image tags
            case StateEnum.GET_IMG_TAGS:
                var labels = JSON.parse(msg.data);
                genLabels(labels);
                imgState = StateEnum.GET_PREFIX;
                break;

            // get image prefix
            case StateEnum.GET_PREFIX:
                if (typeof(msg.data) === 'string')
                {
                    imgSize = imgToRead = parseInt(msg.data);
                    imgBlob = new Blob([], {type: "image/jpeg"});
                    imgState = StateEnum.GET_IMAGE;
//                    console.log("[Camera.js] Pref. red; image size: " + imgSize);
                }
//                else
//                {
//                    console.warn("[Camera.js] Expected string, got: " + msg.data);
//                }
                break;

            // get image data
            case StateEnum.GET_IMAGE:
                if (!(msg.data instanceof Blob))
                {
                    console.warn("[Camera.js] Expected blob, got: ", msg.data);
                    imgState = StateEnum.GET_PREFIX;
                    break;
                }

                imgBlob = new Blob([imgBlob, msg.data], {type: "image/jpeg"});

                imgToRead -= msg.data.size;

//                console.log("[Camera.js] Got blob of size: " + msg.data.size + ", left: " + imgToRead);

                if (imgToRead <= 0)
                {
                    var objectURL = (window.webkitURL || window.URL).createObjectURL(imgBlob);

                    var img = new Image;
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
        };

        var errorHandle = function(e)
        {
            console.log("Camera socket error; closing and reconnecting");

            if (socket.readyState == WebSocket.OPEN)
                socket.close();

            $('#camera-controls-container').empty();
            imgState = StateEnum.GET_CONTROLS;

            socket = WebSocketFactory.open(protocol);
        };

        var headControlHandle = function(e)
        {
            var cmd = {
                command: "controlHead",
                action: e.target.value
            };
            socket.send(JSON.stringify(cmd));
        };

        var createSocket = function()
        {
            socket = WebSocketFactory.open(protocol);
            socket.onerror = errorHandle;
            socket.onmessage = msgHandle;
        };

        createSocket();

        $('.camera-head-controls-button').click(headControlHandle);

        $('#camera-default-reset').click(function()
        {
            socket.send("-1 0");
        });
    }
);

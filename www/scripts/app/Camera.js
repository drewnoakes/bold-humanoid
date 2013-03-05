define(
    [
        'scripts/app/WebSocketFactory'
    ],
    function(WebSocketFactory)
    {
        var protocol = 'camera-protocol',
            socket = WebSocketFactory.open(protocol),
            container = $('#camera-container');

	var imgState = 0,
	    imgSize = 0,
	    imgToRead = 0;

	var imgBlob;

        var controls;

        var ControlTypeEnum = {
            INT: 1,
            BOOL: 2,
            MENU: 3,
            BUTTON: 4,
            INT64: 5,
            CTRL_CLASS: 6,
            STRING: 7,
            BITMASK: 8
        }

        var StateEnum = {
            GET_CONTROLS : 0,
            GET_IMG_TAGS : 1,
            GET_PREFIX : 2,
            GET_IMAGE : 3
        }

        var controlHandle = function(e)
        {
            var c = e.data;
            var val =
                c.type == ControlTypeEnum.INT ? e.target.value :
                c.type == ControlTypeEnum.BOOL ? (e.target.checked ? 1 : 0) :
                c.type == ControlTypeEnum.MENU ? e.target.value :
                0;
            socket.send(c.id + " " + val);
        }

        var genControls = function(controls)
        {

            for (var i = 0; i < controls.length; ++i)
            {
                var c = controls[i];
                c.iface = $('<div>').addClass('camera-control')
                    .append($('<span>').addClass('camera-control-name').text(c.name + " (" + c.minimum + " - " + c.maximum + " [" + c.defaultValue + "])"))
                    .append($('<br>'));

                switch (c.type)
                {
                case ControlTypeEnum.INT:
                    c.iface.append($('<input>').val(c.value).change(c,controlHandle));
                    break;

                case ControlTypeEnum.BOOL:
                    var check = $('<input type="checkbox">').change(c,controlHandle);
                    if (c.value == 1)
                        check.attr('checked','checked');
                    c.iface.append(check);
                    break;

                case ControlTypeEnum.MENU:
                    var menu = $('<select>').change(c,controlHandle);
                    for (var j = 0; j < c.menuItems.length; ++j)
                    {
                        var item = c.menuItems[j];
                        var option = $('<option>').val(item.index).text(item.name); 
                        if (c.value == item.index)
                            option.attr('selected', 'selected');
                        menu.append(option);
                    }
                    c.iface.append(menu);
                    break;
                }

                $('#camera-controls-container').append(c.iface);
            }
        }

	var msgHandle = function(msg)
	{
	    switch (imgState)
	    {
            // get control data
            case StateEnum.GET_CONTROLS:
                controls = JSON.parse(msg.data);
                console.log(controls);
                genControls(controls);
                imgState = StateEnum.GET_PREFIX;
                break;

            // get image tags
            case StateEnum.GET_IMG_TAGS:
                break;

            // get image prefix
            case StateEnum.GET_PREFIX:
		if (typeof(msg.data) === 'string')
		{
		    imgSize = imgToRead = parseInt(msg.data);
		    imgBlob = new Blob([], {type: "image/jpeg"});
		    imgState = StateEnum.GET_IMAGE;
		    console.log("[Camera.js] Pref. red; image size: " + imgSize);
		}
		else
		{
		    console.warn("[Camera.js] Expected string, got: " + msg.data);
		}
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

		console.log("[Camera.js] Got blob of size: " + msg.data.size + ", left: " + imgToRead);

		if (imgToRead <= 0)
		{
		    var objectURL = (window.webkitURL || window.URL).createObjectURL(imgBlob);
		    container.append($('<img>', {src: objectURL}));
		    
		    var images = container.find('img');
		    if (images.length > 5)
			images.first().remove();

		    imgState = StateEnum.GET_PREFIX;
		}
                break;
	    }
	}

	var errorHandle = function(e)
	{
	    console.log("Camera socket error; closing and reconnecting");

	    if (socket.readyState == WebSocket.OPEN)
		socket.close();

            $('#camera-controls-container').empty();
            imgState = StateEnum.GET_CONTROLS;

	    socket = WebSocketFactory.open(protocol);

	    socket.onerror = errorHandle;

            socket.onmessage = msgHandle;
	}
 
	socket.onerror = errorHandle;

        socket.onmessage = msgHandle;
        
        $('#camera-default-reset').click(function() {
            socket.send("-1 0");
        });
    }
);

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

        var genControls = function(controls)
        {
            for (var i = 0; i < controls.length; ++i)
            {
                var c = controls[i];
                c.iface = $('<div>').addClass('camera-control')
                    .append($('<span>').addClass('camera-control-name').text(c.name))
                    .append($('<br>'));

                if (c.menuItems)
                {
                }
                else
                {
                    c.iface.append($('<input>').val(c.value));
                }
                $('#camera-controls-container').append(c.iface);
            }
        }

	var msgHandle = function(msg)
	{
	    switch (imgState)
	    {
            // get control data
            case 0:
                controls = JSON.parse(msg.data);
                console.log(controls);
                genControls(controls);
                ++imgState;
                break;

            // get image tagse
            case 1:
                break;

	    case 2:
		if (typeof(msg.data) === 'string')
		{
		    imgSize = imgToRead = parseInt(msg.data);
		    imgState = 1;
		    imgBlob = new Blob([], {type: "image/jpeg"});
		}
		break;

	    case 3:
		if (!(msg.data instanceof Blob))
		{
		    imgState = 0;
		    break;
		}

		imgBlob = new Blob([imgBlob, msg.data], {type: "image/jpeg"});

		imgToRead -= msg.data.size;
		if (imgToRead <= 0)
		{
		    var objectURL = (window.webkitURL || window.URL).createObjectURL(imgBlob);
		    container.append($('<img>', {src: objectURL}));
		    
		    var images = container.find('img');
		    if (images.length > 5)
			images.first().remove();

		    imgState = 0;
		}
	    }

	    /*
            var imgBlob = new Blob([msg.data], {type: "image/jpeg"});
            var objectURL = (window.webkitURL || window.URL).createObjectURL(imgBlob);
            container.append($('<img>', {src: objectURL}));

            var images = container.find('img');
            if (images.length > 5)
                images.first().remove();
	    */
	}

	var errorHandle = function(e)
	{
	    console.log("Camera socket error; closing and reconnecting");

	    if (socket.readyState == WebSocket.OPEN)
		socket.close();

	    socket = WebSocketFactory.open(protocol);

	    socket.onerror = errorHandle;

            socket.onmessage = msgHandle;
	}
 
	socket.onerror = errorHandle;

        socket.onmessage = msgHandle;
    }
);

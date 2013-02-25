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

        socket.onmessage = function(msg)
        {
	    console.log(imgState);
	    switch (imgState)
	    {
	    case 0:
		if (typeof(msg.data) === 'string')
		{
		    imgSize = imgToRead = parseInt(msg.data);
		    console.log("image size: " + imgSize);
		    imgState = 1;
		    imgBlob = new Blob([], {type: "image/jpeg"});
		}
		break;

	    case 1:
		if (!(msg.data instanceof Blob))
		{
		    imgState = 0;
		    break;
		}

		console.log("to read: " + imgToRead);
		imgBlob = new Blob([imgBlob, msg.data], {type: "image/jpeg"});
		console.log("read: " + msg.data.size);

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
    }
);
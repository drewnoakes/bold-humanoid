define(
    [
        'scripts/app/WebSocketFactory'
    ],
    function(WebSocketFactory)
    {
        var protocol = 'camera-protocol',
            socket = WebSocketFactory.open(protocol),
		    container = $('#camera-container');

		socket.onmessage = function(msg)
        {
            var imgBlob = new Blob([msg.data], {type: "image/jpeg"});
            var objectURL = window.URL.createObjectURL(imgBlob);
            container.append($('<img>').attr('src', objectURL).css({position: 'absolute', left: '0px', top: '0px'}));

            var images = container.find('img');
            if (images.length > 5)
                images.first().remove();
        }
    }
);
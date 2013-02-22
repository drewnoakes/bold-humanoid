define(
    [
        'scripts/app/WebSocketFactory'
    ],
    function(WebSocketFactory)
    {
	var protocol = 'camera-protocol';

        var socket = WebSocketFactory.open(protocol);

	var nImg = 0;
        socket.onmessage = function (msg)
        {
	    var imgBlob = new Blob([msg.data], {type: "image/jpgeg"});
	    var objectURL = window.URL.createObjectURL(imgBlob);
	    $('#camera-container').append($('<img>').attr('src',objectURL).css({position: 'absolute', left: '0px', top: '0px'}));

	    var images = $('#camera-container img');
	    if (images.length > 5)
		images.first().remove();

        }

    }
);
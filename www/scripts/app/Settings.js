/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'Constants'
    ],
    function (Constants)
    {
        'use strict';

        var getQueryStringParameterByName = function(name)
        {
            name = name.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
            var regexS = "[\\?&]" + name + "=([^&#]*)";
            var regex = new RegExp(regexS);
            var results = regex.exec(window.location.search);
            if(results == null)
                return null;
            else
                return decodeURIComponent(results[1].replace(/\+/g, " "));
        };

        var getWebSocketUrl = function ()
        {
            var host = getQueryStringParameterByName("host");

            if (host == null) {
                // Use the current page's host
                var u = document.URL;
                if (u.substring(0, 4) === "http")
                    u = u.substr(7);
                if (u.indexOf(":") != -1)
                    u = u.substring(0, u.indexOf(":"));
                host = u.split('/')[0];
            }

            return "ws://" + host + ":" + Constants.webSocketPort;
        };

        //noinspection UnnecessaryLocalVariableJS
        var Settings = {
            webSocketUrl: getWebSocketUrl()
        };

        return Settings;
    }
);
/**
 * @author Drew Noakes https://drewnoakes.com
 */

// TODO consolidate this network code

export function load<T>(path: string, success: (item: T) => void, error?: (xhr: XMLHttpRequest) => void)
{
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = () =>
    {
        if (xhr.readyState == XMLHttpRequest.DONE) {
            if (xhr.status == 200) {
                if (success)
                    success(JSON.parse(xhr.responseText));
            } else {
                if (error)
                    error(xhr);
            }
        }
    };
    xhr.open("GET", path, true);
    xhr.send();
}

///////////////////////

export interface IServerResponse<T>
{
    success: boolean;
    error?: string;
    status?: number;
    data: T;
}

function requestJson<T>(url: string, method: string, callback: (response: IServerResponse<T>)=>void)
{
    var xhr = new XMLHttpRequest();

    var callbackInvoked = false;

    var fail = (message: string, status?: number, data?: T) =>
    {
        console.assert(!callbackInvoked);
        callback({success: false, error: message, status: status, data: data});
        callbackInvoked = true;
    };

    xhr.onerror = (ev :ErrorEvent) => { fail(ev.message); };

    xhr.onreadystatechange = () =>
    {
        if (xhr.readyState !== XMLHttpRequest.DONE)
            return;

        var response;
        try
        {
            response = JSON.parse(xhr.responseText);
        }
        catch (ex)
        {
            if (ex instanceof SyntaxError)
            {
                fail('Error parsing response JSON: ' + ex.message, xhr.status);
                return;
            }
        }

        console.assert(response !== undefined);

        var success = (response.hasOwnProperty('success') && response.success) || xhr.status === 200;

        if (success)
        {
            console.assert(!callbackInvoked);
            var data = (response.hasOwnProperty('success') && response.hasOwnProperty('data')) ? response.data : response;
            callback({success: true, status: xhr.status, data: data});
            callbackInvoked = true;
        }
        else
        {
            fail(response.message || 'Server indicated failure', xhr.status);
        }
    };

    xhr.open(method, url, /*async*/true);
    xhr.setRequestHeader('Accept', 'application/json');
    xhr.send();
}

export function getJson<T>(url: string, callback: (response: IServerResponse<T>)=>void)
{
    requestJson(url, "GET", callback);
}

export function postJson<T>(url: string, callback: (response: IServerResponse<T>)=>void)
{
    requestJson(url, "POST", callback);
}

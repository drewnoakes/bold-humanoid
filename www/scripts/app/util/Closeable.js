/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [],
    function()
    {
        var Closeable = function()
        {
            this.functions = [];
        };

        Closeable.prototype.add = function(obj)
        {
            if (obj instanceof Array)
            {
                _.each(obj, function (o) { this.add(o); }.bind(this));
            }
            else if (obj instanceof Function)
            {
                this.functions.push(obj);
            }
            else if (obj instanceof Object && obj.close instanceof Function)
            {
                this.functions.push(obj.close);
            }
            else
            {
                console.error('Unexpected closeable registered', obj);
            }
        };

        Closeable.prototype.closeAll = function()
        {
            _.each(this.functions, function(fun) { fun(); });

            this.functions = [];
        };

        return Closeable;
    }
);
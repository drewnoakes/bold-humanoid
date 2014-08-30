var allTestFiles = [],
    TEST_REGEXP = /(spec|test)\.js$/i;

var pathToModule = function (path)
{
    // Normalize paths to RequireJS module names
    return path.replace(/^\/base\//, '').replace(/\.js$/, '');
};

Object.keys(window.__karma__.files).forEach(function (file)
{
    //console.debug('FILE: ' + file);
    if (TEST_REGEXP.test(file))
    {
        var module = pathToModule(file);
        //console.debug('Found test: ' + module);
        allTestFiles.push(module);
    }
});

require.config({
    // Karma serves files under /base, which is the basePath from your config file
    baseUrl: '/base',

    // dynamically load all test files
    deps: allTestFiles,

    // we have to kickoff jasmine, as it is asynchronous
    callback: window.__karma__.start
});

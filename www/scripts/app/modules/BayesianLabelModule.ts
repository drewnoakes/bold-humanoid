import Module = require('Module');

class BayesianLabelModule extends Module
{
    constructor()
    {
        super('bayesian-label', 'bayesian label');
    }

    public load(width: number)
    {
        var imageDiv = document.createElement('div');

        var controlsDiv = document.createElement('div');
    }
}

export = BayesianLabelModule;

/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlClient'
    ],
    function (ControlClient)
    {
        'use strict';

        var nextControlId = 0;

        var ControlBuilder = {};

        ControlBuilder.action = function(id, target)
        {
            var button;
            if (target instanceof HTMLButtonElement)
            {
                button = target;
            }
            else if (target instanceof HTMLElement)
            {
                button = document.createElement('button');
                target.appendChild(button);
            }

            ControlClient.withAction(id, function(action)
            {
                if (!button.textContent)
                    button.innerHTML = action.label;

                button.addEventListener('click', function() { action.activate(); });
            });

            return button;
        };

        ControlBuilder.actions = function(idPrefix, target)
        {
            ControlClient.withActions(idPrefix, function(actions)
            {
                _.each(actions, function(action){ ControlBuilder.action(action.id, target)});
            });
        };

        function createSetting(setting, container, closeables)
        {
            var heading, input,
                wrapper = document.createElement('div');
            wrapper.dataset.path = setting.path;
            wrapper.className = 'setting control';

            if (setting.isAdvanced)
            {
                wrapper.classList.add('advanced');
            }

            switch (setting.type)
            {
                case "bool":
                {
                    var checkboxName = 'checkbox' + (nextControlId++);

                    var checkbox = document.createElement('input');
                    checkbox.type = 'checkbox';
                    checkbox.id = checkboxName;
                    checkbox.addEventListener('change', function ()
                    {
                        setting.setValue(checkbox.checked);
                    });
                    wrapper.appendChild(checkbox);
                    var label = document.createElement('label');
                    label.textContent = setting.getDescription();
                    label.htmlFor = checkboxName;
                    wrapper.appendChild(label);
                    closeables.push(setting.track(function (value)
                    {
                        checkbox.checked = value;
                    }));
                    break;
                }
                case "enum":
                {
                    heading = document.createElement('h3');
                    heading.textContent = setting.getDescription();
                    wrapper.appendChild(heading);

                    var select = document.createElement('select');
                    _.each(setting.enumValues, function(enumValue)
                    {
                        var option = document.createElement('option');
                        option.selected = setting.value === enumValue.value;
                        option.text = enumValue.text;
                        option.value = enumValue.value;
                        select.appendChild(option);
                    });
                    select.addEventListener('change', function()
                    {
                        setting.setValue(parseInt(select.options[select.selectedIndex].value));
                    });
                    closeables.push(setting.track(function(value)
                    {
                        var option = _.find(select.options, function(option) { return parseInt(option.value) === value; });
                        option.selected = true;
                    }));
                    wrapper.appendChild(select);
                    break;
                }
                case "int":
                {
                    heading = document.createElement('h3');
                    heading.textContent = setting.getDescription();
                    wrapper.appendChild(heading);

                    input = document.createElement('input');
                    input.type = 'number';
                    input.value = setting.value;
                    if (typeof(setting.min) !== 'undefined')
                        input.min = setting.min;
                    if (typeof(setting.max) !== 'undefined')
                        input.max = setting.max;
                    wrapper.appendChild(input);

                    input.addEventListener('change', function()
                    {
                        setting.setValue(parseInt(input.value));
                    });
                    closeables.push(setting.track(function(value)
                    {
                        input.value = value;
                    }));
                    break;
                }
                case "double":
                {
                    heading = document.createElement('h3');
                    heading.textContent = setting.getDescription();
                    wrapper.appendChild(heading);

                    input = document.createElement('input');
                    input.type = 'number';
                    input.value = setting.value;
                    if (typeof(setting.min) !== 'undefined')
                        input.min = setting.min;
                    if (typeof(setting.max) !== 'undefined')
                        input.max = setting.max;
                    wrapper.appendChild(input);

                    input.addEventListener('change', function()
                    {
                        setting.setValue(parseInt(input.value));
                    });
                    closeables.push(setting.track(function(value)
                    {
                        input.value = value;
                    }));
                    break;
                }
                default:
                {
                    console.error("Unsupported setting type", setting.type);
                }
            }

            container.appendChild(wrapper);
        }

        ControlBuilder.buildAll = function(idPrefix, container)
        {
            var closeables = [];
            ControlClient.withSettings(idPrefix, function(settings)
            {
                _.each(settings, function (setting)
                {
                    createSetting(setting, container, closeables);
                })
            });

            return closeables;
        };

        ControlBuilder.build = function(path, container)
        {
            var closeables = [];
            ControlClient.withSetting(path, function(setting)
            {
                createSetting(setting, container, closeables);
            });
            return closeables;
        };

        return ControlBuilder;
    }
);
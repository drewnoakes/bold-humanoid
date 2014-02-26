/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />

import ControlClient = require('ControlClient');
import HsvRangeEditor = require('HsvRangeEditor');
import color = require('color');
import Setting = require('Setting');

var nextControlId = 0;

class ControlBuilder
{
    constructor() { throw new Error("Cannot instantiate this class."); }

    public static action(id: string, target: HTMLElement)
    {
        console.assert(!!id && !!target);

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
        else
        {
            console.dir(target);
            console.assert(false && "Unexpected target type")
        }

        ControlClient.withAction(id, action =>
        {
            if (!button.textContent)
                button.innerHTML = action.label;

            button.addEventListener('click', () => action.activate());
        });

        return button;
    }

    public static actions(idPrefix: string, target: HTMLElement)
    {
        console.assert(!!idPrefix && !!target);

        ControlClient.withActions(idPrefix, actions =>
        {
            _.each(actions, action => ControlBuilder.action(action.id, target));
        });
    }

    private static createSetting(setting: Setting, container, closeables)
    {
        if (setting.isReadOnly)
            return;

        var heading, input,
            wrapper = document.createElement('div');
        wrapper.dataset['path'] = setting.path;
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
                checkbox.addEventListener('change', () => setting.setValue(checkbox.checked));
                wrapper.appendChild(checkbox);
                var label = document.createElement('label');
                label.textContent = setting.getDescription();
                label.htmlFor = checkboxName;
                wrapper.appendChild(label);
                closeables.push(setting.track(value => checkbox.checked = value));
                break;
            }
            case "enum":
            {
                heading = document.createElement('h3');
                heading.textContent = setting.getDescription();
                wrapper.appendChild(heading);

                var select = <HTMLSelectElement>document.createElement('select');
                _.each(setting.enumValues, enumValue =>
                {
                    var option = document.createElement('option');
                    option.selected = setting.value === enumValue.value;
                    option.text = enumValue.text;
                    option.value = enumValue.value;
                    select.appendChild(option);
                });
                select.addEventListener('change', () => setting.setValue(parseInt(select.options[select.selectedIndex].value)));
                closeables.push(setting.track(value =>
                {
                    var option = <HTMLOptionElement>_.find(select.options, option => parseInt(option.value) === value);
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

                input.addEventListener('change', () => setting.setValue(parseInt(input.value)));
                closeables.push(setting.track(value => input.value = value));
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

                input.addEventListener('change', () => setting.setValue(parseFloat(input.value)));
                closeables.push(setting.track(value => input.value = value));
                break;
            }
            case 'hsv-range':
            {
                var editor = new HsvRangeEditor(setting.getDescription());
                editor.onChange(value => setting.setValue(value));
                closeables.push(setting.track(value => editor.setValue(value)));
                wrapper.appendChild(editor.element);
                break;
            }
            case 'bgr-colour':
            {
                heading = document.createElement('h3');
                heading.textContent = setting.getDescription();
                wrapper.appendChild(heading);

                var colorInput = document.createElement('input');
                colorInput.type = 'color';
                colorInput.addEventListener('change', () =>
                {
                    var rgb = new color.Rgb(colorInput.value);
                    setting.setValue(rgb.toByteObject());
                });
                closeables.push(setting.track(value => colorInput.value = new color.Rgb(value.r/255, value.g/255, value.b/255).toString()));
                wrapper.appendChild(colorInput);
                break;
            }
            default:
            {
                console.error("Unsupported setting type", setting.type, "for", setting.path);
            }
        }

        container.appendChild(wrapper);
    }

    public static buildAll(idPrefix: string, container: HTMLElement)
    {
        console.assert(!!idPrefix && !!container);

        var closeables = [];
        ControlClient.withSettings(idPrefix, settings =>
        {
            var sortedSettings = settings.sort((a, b) => (a.type == "bool") != (b.type == "bool"));
            _.each(sortedSettings, setting => ControlBuilder.createSetting(setting, container, closeables))
        });
        return closeables;
    }

    public static build(path: string, container: HTMLElement)
    {
        console.assert(!!path && !!container);

        var closeables = [];
        ControlClient.withSetting(path, setting => ControlBuilder.createSetting(setting, container, closeables));
        return closeables;
    }
}

export = ControlBuilder;

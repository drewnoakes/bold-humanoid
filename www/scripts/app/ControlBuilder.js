/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/ControlTypeEnum'
    ],
    function (ControlTypeEnum)
    {
        'use strict';

        var controlId = 0;

        //noinspection UnnecessaryLocalVariableJS

        var ControlBuilder = {
            build: function(family, controls, container, sendCommand)
            {
                container.empty();

                _.each(controls, function(control)
                {
                    var element = $('<div></div>').addClass('control');

                    // Special handling for int types that only have a zero or one value
                    if (control.type === ControlTypeEnum.INT && control.minimum === 0 && control.maximum === 1)
                    {
                        control.type = ControlTypeEnum.BOOL;
                        delete control.minimum;
                        delete control.maximum;
                    }

                    switch (control.type)
                    {
                        case ControlTypeEnum.INT:
                        {
                            var headingHtml = control.name;
                            if (typeof(control.minimum) !== 'undefined' && typeof(control.maximum) !== 'undefined')
                                headingHtml +=  ' <span class="values">(' + control.minimum + ' - ' + control.maximum + ')</span>';
                            if (typeof(control.defaultValue) !== 'undefined')
                                headingHtml += ' <span class="values">[' + control.defaultValue + ']</span>';
                            var heading = $('<h3></h3>').html(headingHtml);
                            var input = $('<input>', {type: 'text'})
                                .val(control.value)
                                .change(function ()
                                {
                                    sendCommand(family, control.id, parseInt(this.value));
                                });

                            element.append(heading).append(input);
                            break;
                        }

                        case ControlTypeEnum.BOOL:
                        {
                            var id = family + (controlId++);
                            var checkbox = $('<input>', {id: id, type: 'checkbox', checked: control.value});

                            checkbox.change(function()
                            {
                                sendCommand(family, control.id, !!this.checked);
                            });

                            var labelHtml = control.name;
                            if (typeof(control.defaultValue) !== 'undefined')
                                labelHtml += ' <span class="values">[' + (control.defaultValue ? 'on' : 'off') + ']</span>';

                            var checkboxLabel = $('<label>', {for: id})
                                .html(labelHtml);

                            element.append(checkbox).append(checkboxLabel);
                            break;
                        }

                        case ControlTypeEnum.ENUM:
                        {
                            var headingText = control.name;
                            if (typeof(control.defaultValue) !== 'undefined')
                            {
                                var defaultEnumValue = _.find(control.enumValues, function(ev) { return ev.value === control.defaultValue; });
                                if (defaultEnumValue)
                                {
                                    headingText += ' <span class="values">[' + defaultEnumValue.name + ']</span>';
                                }
                            }
                            element.append($('<h3></h3>').html(headingText));
                            var menu = $('<select></select>').change(function()
                            {
                                sendCommand(family, control.id, parseInt(this.options[this.selectedIndex].value));
                            });

                            _.each(control.enumValues, function(enumValue)
                            {
                                var menuItem = $('<option></option>', {selected: control.value == enumValue.value})
                                    .val(enumValue.value)
                                    .text(enumValue.name);
                                menu.append(menuItem);
                            });
                            element.append(menu);
                            break;
                        }

                        case ControlTypeEnum.BUTTON:
                        {
                            var button = $('<button></button>').html(control.name).click(function()
                            {
                                sendCommand(family, control.id);
                                return false;
                            });
                            element.append(button);
                            break;
                        }
                    }

                    container.append(element);
                });
            }
        };

        return ControlBuilder;
    }
);
/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/ControlTypeEnum',
        'scripts/app/ControlClient'
    ],
    function (ControlTypeEnum, ControlClient)
    {
        'use strict';

        var nextControlId = 0;

        var ControlBuilder = {};

        ControlBuilder.build = function(family, container)
        {
            container.empty();

            ControlClient.withData(family, function(controls)
            {
                _.each(controls, function(control)
                {
                    var element = $('<div></div>').addClass('control');

                    if (control.advanced)
                    {
                        element.addClass('advanced');
                    }

                    switch (control.type)
                    {
                        case ControlTypeEnum.INT:
                        {
                            var headingHtml = control.name;
                            if (typeof(control.minimum) !== 'undefined' && typeof(control.maximum) !== 'undefined')
                                headingHtml +=  ' <span class="value-range">(' + control.minimum + ' - ' + control.maximum + ')</span>';
                            if (typeof(control.defaultValue) !== 'undefined')
                                headingHtml += ' <span class="default-value">[' + control.defaultValue + ']</span>';
                            var heading = $('<h3></h3>').html(headingHtml);
                            var input = $('<input>', {type: 'text'})
                                .val(control.value)
                                .change(function ()
                                {
                                    ControlClient.sendCommand(family, control.id, parseInt(this.value));
                                });

                            element.append(heading).append(input);
                            break;
                        }

                        case ControlTypeEnum.BOOL:
                        {
                            var id = family + (nextControlId++);
                            var checkbox = $('<input>', {id: id, type: 'checkbox', checked: control.value});

                            checkbox.change(function()
                            {
                                ControlClient.sendCommand(family, control.id, !!this.checked);
                            });

                            var labelHtml = control.name;
                            if (typeof(control.defaultValue) !== 'undefined')
                                labelHtml += ' <span class="default-value">[' + (control.defaultValue ? 'on' : 'off') + ']</span>';

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
                                    headingText += ' <span class="default-value">[' + defaultEnumValue.name + ']</span>';
                                }
                            }
                            element.append($('<h3></h3>').html(headingText));
                            var menu = $('<select></select>').change(function()
                            {
                                ControlClient.sendCommand(family, control.id, parseInt(this.options[this.selectedIndex].value));
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
                                ControlClient.sendCommand(family, control.id);
                                return false;
                            });
                            element.append(button);
                            break;
                        }
                    }

                    container.append(element);
                });
            });
        };

        return ControlBuilder;
    }
);
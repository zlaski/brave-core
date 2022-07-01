# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/. */

import contextlib
import inspect
import types


def override_function(scope, name=None):
    """Replaces existing function in the scope."""

    def decorator(new_function):
        is_dict_scope = isinstance(scope, dict)
        function_name = name or new_function.__name__
        if is_dict_scope:
            original_check = scope.get(function_name, None)
        else:
            original_check = getattr(scope, function_name, None)

        if not callable(original_check):
            raise NameError(f'Failed to override function: '
                            f'{function_name} not found or not callable')

        def wrapped_f(*args, **kwargs):
            return new_function(original_check, *args, **kwargs)

        if is_dict_scope:
            scope[function_name] = wrapped_f
        else:
            setattr(scope, function_name, wrapped_f)

        return wrapped_f

    return decorator


@contextlib.contextmanager
def override_scope_function(scope, new_function, name=None):
    """Scoped function override helper. Can override a scope function or a class
    method."""
    function_name = name or new_function.__name__
    original_function = getattr(scope, function_name, None)
    try:
        if not callable(original_function):
            raise NameError(f'Failed to override scope function: '
                            f'{function_name} not found or not callable')

        def wrapped_method(self, *args, **kwargs):
            return new_function(self, original_function, *args, **kwargs)

        def wrapped_function(*args, **kwargs):
            return new_function(original_function, *args, **kwargs)

        if inspect.ismethod(original_function):
            setattr(scope, function_name,
                    types.MethodType(wrapped_method, scope))
        else:
            setattr(scope, function_name, wrapped_function)

        yield
    finally:
        if original_function:
            setattr(scope, function_name, original_function)


@contextlib.contextmanager
def override_scope_variable(scope, name, value):
    """Scoped variable override helper."""
    if not hasattr(scope, name):
        raise NameError(f'Failed to override scope variable: {name} not found')
    original_value = getattr(scope, name)
    try:
        setattr(scope, name, value)
        yield
    finally:
        setattr(scope, name, original_value)

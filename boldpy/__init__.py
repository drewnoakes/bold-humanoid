def static_var(varname, value):
    def decorate(func):
        setattr(func, varname, value)
        return func
    return decorate

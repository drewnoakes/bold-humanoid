import bold
import os.path
import sys

class Param:
    """
    Empty class used to build parameter hierarchies
    """
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

    def __str__(self):
        return "<Param>"

    def add(self, kwds):
        self.__dict__.update(kwds)


class PyConf(bold.ConfImpl):
    def __init__(self, prefix="conf", reportMissing=False):
        bold.ConfImpl.__init__(self)

        self.prefix = prefix
        self.reportMissing = reportMissing
        self.reported = []
        self.reportFile = "missing.py"
        # Load entries that have already been listed as missing
        if reportMissing:
            if os.path.exists(self.reportFile):
                with open(self.reportFile, "r+") as f:
                    self.reported = list(map(lambda l: (l.split(" = "))[0], f.read().splitlines()))

    def _checkReportMissing(self, path, defVal):
        if self.reportMissing:
            if path not in self.reported:
                self.reported.append(path)
                try:
                    with open(self.reportFile, "a+") as f:
                        f.write(path + " = " + str(defVal) + "\n")
                except:
                    print("Failed opening missing param file")

    def paramExists(self, path):
        res = None
        try:
            res = eval("sys.modules['__main__']." + self.prefix + "." + path)
        except:
            pass

        return res != None

    def _getParam(self, path, defVal):
        els = path.split(".")

        par = sys.modules['__main__']
        res = None
        try:
            res = reduce(lambda p,c: None if p is None else p.getattr(c), els, par) 
        except:
            res = None

        #try:
        #    print("a")
        #    res = eval(toeval);
        #    print("b")
        #except:
        #    if (self.reportMissing):
        #        self._checkReportMissing(path, defVal)
        print("res: " + str(res))
        
        if (res == None):
            return defVal
        else:
            return res

    def getParamStr(self, path, defVal):
        return str(self._getParam(path, defVal))

    def getParamInt(self, path, defVal):
        return int(self._getParam(path, defVal))

    def getParamDbl(self, path, defVal):
        return float(self._getParam(path, defVal))

    def getParamBool(self, path, defVal):
        return bool(self._getParam(path, defVal))


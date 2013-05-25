import bold
import os.path
import sys

class Param:
    """
    Empty class used to build parameter hierarchies
    """
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

    def add(self, kwds):
        self.__dict__.update(kwds)


class PyConf(bold.ConfImpl):
    def __init__(self, reportMissing=False):
        bold.ConfImpl.__init__(self)

        self.reportMissing = reportMissing
        self.reported = []
        self.reportFile = "missing.py"
        # Load entries that have already been listed as missing
        if reportMissing:
            if os.path.exists(self.reportFile):
                with open(self.reportFile, "r+") as f:
                    self.reported = map(lambda l: (l.split(" = "))[0], f.read().splitlines())

    def _checkReportMissing(self, path, defVal):
        print(1)
        if self.reportMissing:
            print(1.5)
            if path not in self.reported:
                print(2)
                self.reported.append(path)
                print(3)
                with open(self.reportFile, "a+") as f:
                    print(4)
                    f.write(path + " = " + str(defVal) + "\n")
                    print(5)

    def paramExists(self, path):
        res = None
        try:
            res = eval("sys.modules['__main__']."+path)
        except:
            pass

        return res != None

    def _getParam(self, path, defVal):

        res = None
        try:
            res = eval("sys.modules['__main__']."+path)
        except:
            print("not found: " + path)
            self._checkReportMissing(path, defVal)

        if (res == None):
            return defVal
        else:
            return res

    def getParamStr(self, path, defVal):
        return self._getParam(path, defVal)

    def getParamInt(self, path, defVal):
        return self._getParam(path, defVal)

    def getParamDbl(self, path, defVal):
        return self._getParam(path, defVal)

    def getParamBool(self, path, defVal):
        return self._getParam(path, defVal)

